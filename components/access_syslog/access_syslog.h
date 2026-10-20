#pragma once

#include "esphome/core/component.h"
#include "esphome/components/access_db/access_db.h"
#include "esphome/components/access_site/access_site.h"
#include "esphome/components/push_auth/push_auth.h"
#include "esphome/components/web_server_idf/web_server_idf.h"

#include <esp_http_server.h>
#include <string>

namespace esphome {
namespace access_syslog {

class AccessSyslog : public web_server_idf::AsyncWebHandler, public Component {
 public:
  void set_server(const std::string &server) { this->server_ = server; }
  void set_port(uint16_t port) { this->port_ = port; }
  void set_hostname(const std::string &hostname) { this->hostname_ = hostname; }
  void set_facility(uint8_t facility) { this->facility_ = facility; }
  void set_site(access_site::AccessSite *site) { this->site_ = site; }
  void set_access_db(access_db::AccessDb *access_db) { this->access_db_ = access_db; }
  void set_auth(push_auth::PushAuth *auth) { this->auth_ = auth; }
  void set_unlock_path(const std::string &path) { this->unlock_path_ = path; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void send_message(const std::string &message);
  void send_online();
  void send_door_opened();
  void send_unlocked_by();
  void send_unlocked_by_uid(const std::string &uid);
  void send_access_denied(const std::string &detail = "");

  bool canHandle(web_server_idf::AsyncWebServerRequest *request) const override;
  void handleRequest(web_server_idf::AsyncWebServerRequest *request) override;
  void handleBody(web_server_idf::AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index,
                  size_t total) override;
  void handle_raw_request_(httpd_req_t *req);
  bool isRequestHandlerTrivial() const override { return false; }

 protected:
  std::string site_label_() const;
  std::string resolve_person_name_(const access_db::AccessDbLookupResult *match = nullptr) const;
  void send_door_opened_for_person_(const std::string &person_name);
  void send_unlocked_by_person_(const std::string &person_name);
  bool ensure_socket_();
  void close_socket_();

  std::string server_;
  uint16_t port_{514};
  std::string hostname_{"access-control"};
  uint8_t facility_{16};
  std::string unlock_path_{"/access-unlock"};
  access_site::AccessSite *site_{nullptr};
  access_db::AccessDb *access_db_{nullptr};
  push_auth::PushAuth *auth_{nullptr};
  int socket_{-1};
  std::string body_;
  bool body_ok_{false};
};

extern AccessSyslog *global_access_syslog;  // NOLINT

}  // namespace access_syslog
}  // namespace esphome
