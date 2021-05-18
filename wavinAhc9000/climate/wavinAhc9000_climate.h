#pragma once

#include "../wavinAhc9000.h"
#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"

namespace esphome {
namespace wavinAhc9000 {

class WavinAhc9000Climate : public climate::Climate, public Component {
 public:
  WavinAhc9000Climate(WavinAhc9000 *wavin) : wavin_(wavin) {}
  void setup() override;
  void dump_config() override;

  void set_channel(int channel) { channel_ = channel; }

 protected:
  WavinAhc9000 *wavin_;
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

  int channel_;
};

}  // namespace wavinAhc9000
}  // namespace esphome
