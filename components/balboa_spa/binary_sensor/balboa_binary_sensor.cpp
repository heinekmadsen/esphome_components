#include "esphome/components/balboa_spa/binary_sensor/balboa_binary_sensor.h"

namespace esphome
{
    namespace balboa_spa
    {
        static const char *TAG = "balboa_spa.binary_sensor";

        void BalboaBinarySensor::setup()
        {
            
            this->parent_->register_binary_sensor_callback(this->binary_sensor_id_, [this](const bool state)
                                                    { this->publish_state(state); });
        }
    }
}