#include "esphome/components/balboa_spa/climate/balboa_climate.h"
namespace esphome
{
  namespace balboa_spa
  {
    static const char *TAG = "balboa_spa.climate";

    void BalboaClimate::setup()
    {
      this->rest_mode_ = false;
      this->heater_active_ = false;

      this->parent_->register_sensor_callback(40, [this](const float temp) {
        this->target_temperature = temp;
        this->publish_state();
      });
      this->parent_->register_sensor_callback(41, [this](const float temp) {
        this->current_temperature = temp;
        this->publish_state();
      });

      this->parent_->register_binary_sensor_callback(23, [this](const bool rest_state) {
        this->rest_mode_ = rest_state;
        if (rest_state)
        {
          this->heater_active_ = false;
        }
        this->update_operating_state_();
      });

      this->parent_->register_binary_sensor_callback(21, [this](const bool heater_state) {
        this->heater_active_ = heater_state;
        if (this->rest_mode_)
        {
          this->heater_active_ = false;
        }
        this->update_operating_state_();
      });

      this->update_operating_state_();
    }

    void BalboaClimate::control(const climate::ClimateCall &call)
    {
      if (call.get_target_temperature().has_value())
      {
        float temp = *call.get_target_temperature();
        this->parent_->on_set_temp(temp);
      }

      if (call.get_mode().has_value())
      {
        auto mode = *call.get_mode();
        if (mode == climate::CLIMATE_MODE_OFF)
        {
          this->parent_->set_heater_enabled(false);
        }
        else if (mode == climate::CLIMATE_MODE_HEAT)
        {
          this->parent_->set_heater_enabled(true);
        }
      }
    }

    void BalboaClimate::update_operating_state_()
    {
      if (this->rest_mode_)
      {
        this->mode = climate::CLIMATE_MODE_OFF;
        this->action = climate::CLIMATE_ACTION_OFF;
      }
      else
      {
        this->mode = climate::CLIMATE_MODE_HEAT;
        this->action = this->heater_active_ ? climate::CLIMATE_ACTION_HEATING : climate::CLIMATE_ACTION_IDLE;
      }
      this->publish_state();
    }
  }
}
