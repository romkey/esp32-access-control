#pragma once

#include "access_syslog.h"
#include "esphome/core/automation.h"

namespace esphome {
namespace access_syslog {

template<typename... Ts> class SendOnlineAction : public Action<Ts...> {
 public:
  SendOnlineAction(AccessSyslog *parent) : parent_(parent) {}
  void play(Ts... x) override { this->parent_->send_online(); }

 protected:
  AccessSyslog *parent_;
};

template<typename... Ts> class SendDoorOpenedAction : public Action<Ts...> {
 public:
  SendDoorOpenedAction(AccessSyslog *parent) : parent_(parent) {}
  void play(Ts... x) override { this->parent_->send_door_opened(); }

 protected:
  AccessSyslog *parent_;
};

template<typename... Ts> class SendUnlockedByAction : public Action<Ts...> {
 public:
  SendUnlockedByAction(AccessSyslog *parent) : parent_(parent) {}
  void play(Ts... x) override { this->parent_->send_unlocked_by(); }

 protected:
  AccessSyslog *parent_;
};

template<typename... Ts> class SendUnlockedByUidAction : public Action<Ts...> {
 public:
  SendUnlockedByUidAction(AccessSyslog *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, uid)

  void play(Ts... x) override { this->parent_->send_unlocked_by_uid(this->uid_.value(x...)); }

 protected:
  AccessSyslog *parent_;
};

template<typename... Ts> class SendMessageAction : public Action<Ts...> {
 public:
  SendMessageAction(AccessSyslog *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, message)

  void play(Ts... x) override { this->parent_->send_message(this->message_.value(x...)); }

 protected:
  AccessSyslog *parent_;
};

}  // namespace access_syslog
}  // namespace esphome
