#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/core/log.h"
#include "esphome/components/balboa_spa/balboa_spa.h"

namespace esphome
{
    namespace balboa_spa
    {
    class BalboaBinarySensor : public Component, public binary_sensor::BinarySensor
        {
        public:
            void setup() override;
            void set_binary_sensor_id(unsigned int id) { this->binary_sensor_id_ = id; }
            void set_balboa_parent(BalboaSpa *parent)
            {
                this->parent_ = parent;
            }

        protected:
            BalboaSpa *parent_{nullptr};
            unsigned int binary_sensor_id_;
        };
    }
}