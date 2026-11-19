#pragma once

#include "esphome/components/button/button.h"
#include "esphome/core/component.h"

#include "../auto_nilan.h"

namespace esphome {
namespace auto_nilan {

class AutoNilanButton : public button::Button, public Component {
 public:
  enum Action : uint8_t {
    ACTION_COMMISSION = 0,
    ACTION_RESET_ALARM = 1,
    ACTION_RESET_FILTER = 2,
  };

  void set_parent(AutoNilan *parent) { this->parent_ = parent; }
  void set_action(uint8_t action) { this->action_ = action; }
  void dump_config() override;

 protected:
  void press_action() override;

  AutoNilan *parent_{nullptr};
  uint8_t action_{ACTION_COMMISSION};
};

}  // namespace auto_nilan
}  // namespace esphome
