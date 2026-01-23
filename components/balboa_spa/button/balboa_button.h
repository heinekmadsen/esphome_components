#pragma once

#include "esphome/core/component.h"
#include "esphome/components/button/button.h"
#include "esphome/core/log.h"
#include "esphome/components/balboa_spa/balboa_spa.h"

namespace esphome
{
    namespace balboa_spa
    {
        class BalboaButton : public Component, public button::Button
        {
        public:
            void press_action() override;
            void set_button_id(unsigned int id) { this->button_id = id; }
            void set_balboa_parent(BalboaSpa *parent)
            {
                this->parent_ = parent;
            }

        protected:
            BalboaSpa *parent_{nullptr};
            unsigned int button_id;
        };
    }
}