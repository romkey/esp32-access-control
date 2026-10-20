#include "access_syslog.h"

#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/components/network/util.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

#include <ArduinoJson.h>
#include <lwip/inet.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <mutex>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace esphome {
namespace access_syslog {

static const char *const TAG = "access_syslog";
static constexpr size_t UNLOCK_BODY_MAX = 512;
static constexpr uint32_t RESOLVE_RETRY_MS = 30000;
static constexpr size_t MAX_PENDING = 16;
static constexpr uint32_t BODY_DEADLINE_MS = 5000;

AccessSyslog *global_access_syslog = nullptr;  // NOLINT

float AccessSyslog::get_setup_priority() const { return setup_priority::WIFI - 1.0f; }

void AccessSyslog::setup() {
  global_access_syslog = this;
  this->ensure_socket_();

  if (this->unlock_path_.empty() || this->auth_ == nullptr) {
    return;
  }

  auto *base = web_server_base::global_web_server_base;
  if (base == nullptr) {
    ESP_LOGW(TAG, "WebServerBase not found; unlock endpoint disabled");
    return;
  }

  base->add_handler_without_auth(this);

  httpd_uri_t put_uri = {
      .uri = this->unlock_path_.c_str(),
      .method = HTTP_PUT,
      .handler = [](httpd_req_t *req) -> esp_err_t {
        if (global_access_syslog == nullptr) {
          httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, nullptr);
          return ESP_FAIL;
        }
        return global_access_syslog->handle_put_(req);
      },
      .user_ctx = nullptr,
  };

  auto *server = base->get_server();
  if (server != nullptr && server->get_server() != nullptr) {
    httpd_register_uri_handler(server->get_server(), &put_uri);
  }
}

void AccessSyslog::dump_config() {
  ESP_LOGCONFIG(TAG, "Access Syslog:");
  ESP_LOGCONFIG(TAG, "  Server: %s:%u", this->server_.c_str(), this->port_);
  ESP_LOGCONFIG(TAG, "  Hostname: %s", this->hostname_.c_str());
  ESP_LOGCONFIG(TAG, "  Facility: %u", this->facility_);
  if (this->site_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Site: %s", this->site_->get_name().c_str());
  }
  if (!this->unlock_path_.empty()) {
    ESP_LOGCONFIG(TAG, "  Unlock path: %s", this->unlock_path_.c_str());
  }
}

std::string AccessSyslog::site_label_() const {
  if (this->site_ != nullptr && !this->site_->get_name().empty()) {
    return this->site_->get_name();
  }
  return this->hostname_;
}

std::string AccessSyslog::resolve_person_name_(const access_db::AccessDbLookupResult *match) const {
  if (match != nullptr && match->found && !match->name.empty()) {
    return match->name;
  }
  if (this->access_db_ != nullptr) {
    const auto last = this->access_db_->get_last_match();
    if (last.found && !last.name.empty()) {
      return last.name;
    }
  }
  return "unknown";
}

bool AccessSyslog::ensure_socket_() {
  if (this->socket_ >= 0) {
    return true;
  }

  this->socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (this->socket_ < 0) {
    ESP_LOGE(TAG, "Failed to create syslog socket: errno %d", errno);
    return false;
  }

  struct timeval timeout {};
  timeout.tv_sec = 2;
  timeout.tv_usec = 0;
  setsockopt(this->socket_, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

  return true;
}

void AccessSyslog::close_socket_() {
  if (this->socket_ >= 0) {
    close(this->socket_);
    this->socket_ = -1;
  }
}

// Threads: the main loop, the HTTP server task (unlock endpoint) and a
// short-lived DNS task all reach this class. The rules:
//  - send_message() may be called from any thread; it only queues the message
//    under mutex_.
//  - Everything that touches the socket (ensure_socket_, transmit_,
//    close_socket_) and the resolve bookkeeping runs on the main loop only,
//    from loop(), so none of it needs a lock.
//  - mutex_ guards pending_, dest_, dest_valid_ and resolving_; the DNS task
//    publishes its result under it. It is never held across getaddrinfo,
//    sendto or logging. Components live for the whole run, so the task's
//    pointer to this never dangles.
//
// The server is an IPv4 literal or a hostname. A literal is parsed here (no
// network access). A hostname is resolved by a short-lived FreeRTOS task,
// never on the main loop: getaddrinfo() blocks for the resolver timeout. A
// task rather than lwIP's dns_gethostbyname(), which needs the tcpip core lock
// and calls back on the tcpip thread; getaddrinfo() is the call ESP-IDF
// supports from any task. After a failure the next task starts at most every
// RESOLVE_RETRY_MS.
bool AccessSyslog::resolve_server_(struct sockaddr_in *out) {
  {
    std::lock_guard<std::mutex> lock(this->mutex_);
    if (this->dest_valid_) {
      *out = this->dest_;
      return true;
    }
    if (this->resolving_) {
      return false;
    }
  }

  struct in_addr literal {};
  if (inet_pton(AF_INET, this->server_.c_str(), &literal) == 1) {
    struct sockaddr_in dest {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(this->port_);
    dest.sin_addr = literal;
    std::lock_guard<std::mutex> lock(this->mutex_);
    this->dest_ = dest;
    this->dest_valid_ = true;
    *out = dest;
    return true;
  }

  const uint32_t now = millis();
  if (this->resolve_attempted_ && now - this->last_resolve_ms_ < RESOLVE_RETRY_MS) {
    return false;
  }
  if (!network::is_connected()) {
    return false;
  }
  this->resolve_attempted_ = true;
  this->last_resolve_ms_ = now;
  {
    std::lock_guard<std::mutex> lock(this->mutex_);
    this->resolving_ = true;
  }
  if (xTaskCreate(&AccessSyslog::resolve_task_, "syslog_dns", 4096, this, 1, nullptr) != pdPASS) {
    ESP_LOGW(TAG, "Cannot start the syslog DNS task");
    std::lock_guard<std::mutex> lock(this->mutex_);
    this->resolving_ = false;
  }
  return false;
}

void AccessSyslog::resolve_task_(void *arg) {
  auto *self = static_cast<AccessSyslog *>(arg);
  struct addrinfo hints {};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  struct addrinfo *res = nullptr;
  const int err = getaddrinfo(self->server_.c_str(), nullptr, &hints, &res);

  struct sockaddr_in dest {};
  const bool ok = err == 0 && res != nullptr;
  if (ok) {
    dest.sin_family = AF_INET;
    dest.sin_port = htons(self->port_);
    dest.sin_addr = reinterpret_cast<struct sockaddr_in *>(res->ai_addr)->sin_addr;
  } else {
    ESP_LOGW(TAG, "Cannot resolve syslog server %s (error %d); retrying in %us", self->server_.c_str(), err,
             static_cast<unsigned>(RESOLVE_RETRY_MS / 1000));
  }
  if (res != nullptr) {
    freeaddrinfo(res);
  }
  {
    std::lock_guard<std::mutex> lock(self->mutex_);
    if (ok) {
      self->dest_ = dest;
      self->dest_valid_ = true;
    }
    self->resolving_ = false;
  }
  vTaskDelete(nullptr);
}

// Forget a resolved hostname so the next loop looks it up again; the address
// may have changed. A literal needs no lookup. Not while a lookup is running.
// Main loop only.
void AccessSyslog::invalidate_server_() {
  struct in_addr literal {};
  if (inet_pton(AF_INET, this->server_.c_str(), &literal) == 1) {
    return;
  }
  std::lock_guard<std::mutex> lock(this->mutex_);
  if (this->resolving_) {
    return;
  }
  this->dest_valid_ = false;
  this->resolve_attempted_ = false;
}

void AccessSyslog::loop() {
  const bool connected = network::is_connected();
  if (connected && !this->was_connected_) {
    this->invalidate_server_();
  }
  this->was_connected_ = connected;

  struct sockaddr_in dest {};
  if (!this->resolve_server_(&dest)) {
    return;
  }

  std::deque<std::string> pending;
  {
    std::lock_guard<std::mutex> lock(this->mutex_);
    if (this->pending_.empty()) {
      return;
    }
    pending.swap(this->pending_);
  }
  for (const auto &message : pending) {
    this->transmit_(dest, message);
  }
}

// Any thread. The message is sent by loop(), at once if the server address is
// known and otherwise once it resolves; only the latest MAX_PENDING are kept.
void AccessSyslog::send_message(const std::string &message) {
  if (message.empty()) {
    return;
  }
  bool dropped = false;
  {
    std::lock_guard<std::mutex> lock(this->mutex_);
    if (this->pending_.size() >= MAX_PENDING) {
      this->pending_.pop_front();
      dropped = true;
    }
    this->pending_.push_back(message);
  }
  if (dropped) {
    ESP_LOGW(TAG, "Syslog queue full; dropped the oldest message");
  }
}

void AccessSyslog::transmit_(const struct sockaddr_in &dest, const std::string &message) {
  if (!this->ensure_socket_()) {
    return;
  }

  char timestamp[20];
  struct timeval tv {};
  gettimeofday(&tv, nullptr);
  struct tm timeinfo {};
  localtime_r(&tv.tv_sec, &timeinfo);
  strftime(timestamp, sizeof(timestamp), "%b %e %H:%M:%S", &timeinfo);

  const int pri = this->facility_ * 8 + 6;
  char packet[512];
  const int len = snprintf(packet, sizeof(packet), "<%d>%s %s access-control: %s", pri, timestamp,
                           this->hostname_.c_str(), message.c_str());
  if (len <= 0 || len >= static_cast<int>(sizeof(packet))) {
    ESP_LOGW(TAG, "Syslog message too long");
    return;
  }

  const ssize_t sent =
      sendto(this->socket_, packet, len, 0, reinterpret_cast<const struct sockaddr *>(&dest), sizeof(dest));
  if (sent < 0) {
    ESP_LOGW(TAG, "Syslog send failed: errno %d", errno);
    this->close_socket_();
    this->invalidate_server_();
    return;
  }

  ESP_LOGD(TAG, "Syslog: %s", message.c_str());
}

void AccessSyslog::send_door_opened_for_person_(const std::string &person_name) {
  const std::string site = this->site_label_();
  this->send_message(person_name + " has opened " + site + " door");
}

void AccessSyslog::send_unlocked_by_person_(const std::string &person_name) {
  const std::string site = this->site_label_();
  this->send_message(site + " door unlocked by " + person_name);
}

void AccessSyslog::send_online() { this->send_message(this->site_label_() + " access control is online"); }

void AccessSyslog::send_door_opened() { this->send_door_opened_for_person_(this->resolve_person_name_()); }

void AccessSyslog::send_unlocked_by() { this->send_unlocked_by_person_(this->resolve_person_name_()); }

void AccessSyslog::send_unlocked_by_uid(const std::string &uid) {
  if (this->access_db_ == nullptr) {
    this->send_unlocked_by_person_("unknown");
    return;
  }
  const auto match = this->access_db_->lookup_by_uid(uid);
  this->send_unlocked_by_person_(this->resolve_person_name_(&match));
}

void AccessSyslog::send_access_denied(const std::string &detail) {
  const std::string site = this->site_label_();
  if (detail.empty()) {
    this->send_message("access denied at " + site);
  } else {
    this->send_message("access denied at " + site + " (" + detail + ")");
  }
}

bool AccessSyslog::canHandle(web_server_idf::AsyncWebServerRequest *request) const {
  if (this->unlock_path_.empty() || this->auth_ == nullptr) {
    return false;
  }
  if (request->method() != HTTP_POST) {
    return false;
  }
  char url_buf[web_server_idf::AsyncWebServerRequest::URL_BUF_SIZE];
  return request->url_to(url_buf) == this->unlock_path_;
}

// ESPHome's web server reads a POST body that isn't a form before it calls
// handleRequest() and hands it over in chunks here (see AccessDb::handleBody).
// Clients must send a non-form Content-Type such as application/json.
void AccessSyslog::handleBody(web_server_idf::AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index,
                              size_t total) {
  this->on_body_(data, len, index, total);
}

// The PUT route is registered with httpd directly, so ESPHome has not read its
// body; read it here and feed it through the same path. body_ is only touched
// on the HTTP server task, which handles one request at a time. A receive
// timeout is retried until BODY_DEADLINE_MS; any other failure means the
// socket is gone and ESP_FAIL makes httpd close it.
esp_err_t AccessSyslog::handle_put_(httpd_req_t *req) {
  const size_t total = req->content_len;
  if (total > UNLOCK_BODY_MAX) {
    // Too large to read; answer and let httpd close the connection.
    this->on_body_(nullptr, 0, 0, total);
    this->handle_raw_request_(req);
    return ESP_FAIL;
  }
  const uint32_t start = millis();
  char buffer[128];
  size_t index = 0;
  if (total == 0) {
    this->on_body_(nullptr, 0, 0, 0);
  }
  while (index < total) {
    const int n = httpd_req_recv(req, buffer, std::min(total - index, sizeof(buffer)));
    if (n == HTTPD_SOCK_ERR_TIMEOUT) {
      if (millis() - start > BODY_DEADLINE_MS) {
        this->body_complete_ = false;
        httpd_resp_set_status(req, "408 Request Timeout");
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"ok\":false,\"error\":\"body_timeout\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
      }
      continue;
    }
    if (n <= 0) {
      this->body_complete_ = false;
      return ESP_FAIL;
    }
    this->on_body_(reinterpret_cast<uint8_t *>(buffer), n, index, total);
    index += n;
  }
  this->handle_raw_request_(req);
  return ESP_OK;
}

void AccessSyslog::on_body_(uint8_t *data, size_t len, size_t index, size_t total) {
  if (index == 0) {
    this->body_.clear();
    this->body_complete_ = false;
    this->body_ok_ = total > 0 && total <= UNLOCK_BODY_MAX;
    if (this->body_ok_) {
      this->body_.reserve(total);
    }
  }
  if (this->body_ok_) {
    this->body_.append(reinterpret_cast<const char *>(data), len);
    this->body_complete_ = index + len >= total;
  }
}

void AccessSyslog::handleRequest(web_server_idf::AsyncWebServerRequest *request) {
  this->handle_raw_request_(*request);
}

void AccessSyslog::handle_raw_request_(httpd_req_t *req) {
  if (this->auth_ == nullptr || !this->auth_->validate_raw(req)) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"invalid_token\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  if (this->access_db_ == nullptr) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"access_db_unavailable\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  std::string body;
  body.swap(this->body_);
  const bool body_ok = this->body_complete_;  // not a partial body from an aborted request
  this->body_ok_ = false;
  this->body_complete_ = false;
  if (!body_ok || body.empty()) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"invalid_body\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, body) != DeserializationError::Ok) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"parse_failed\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  const char *uid = doc["uid"].as<const char *>();
  if (uid == nullptr || uid[0] == '\0') {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"missing_uid\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  const auto match = this->access_db_->lookup_by_uid(uid);
  if (!match.found) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":false,\"error\":\"unknown_uid\"}", HTTPD_RESP_USE_STRLEN);
    return;
  }

  this->send_unlocked_by_person_(match.name);

  char response[160];
  snprintf(response, sizeof(response), "{\"ok\":true,\"uid\":\"%s\",\"name\":\"%s\"}", match.uid.c_str(),
           match.name.c_str());
  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

}  // namespace access_syslog
}  // namespace esphome
