#include "auto_nilan_button.h"

#include "esphome/core/log.h"

namespace esphome {
namespace auto_nilan {

static const char *const TAG = "auto_nilan.button";

void AutoNilanButton::dump_config() {
  LOG_BUTTON("  ", "Nilan button", this);
  ESP_LOGCONFIG(TAG, "  Action: %u", (unsigned) this->action_);
}

void AutoNilanButton::press_action() {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "Button pressed but hub is missing");
    return;
  }
  std::string error;
  switch (this->action_) {
    case ACTION_COMMISSION:
      this->parent_->commission();
      break;
    case ACTION_RESET_ALARM:
      this->parent_->reset_alarm();
      break;
    case ACTION_RESET_FILTER:
      if (!this->parent_->press_point("reset_filter", &error))
        ESP_LOGW(TAG, "Reset filter failed: %s", error.c_str());
      break;
    default:
      break;
  }
}

}  // namespace auto_nilan
}  // namespace esphome
