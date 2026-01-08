#include "esphome/components/balboa_spa/button/balboa_button.h"

namespace esphome
{
    namespace balboa_spa
    {
        static const char *TAG = "balboa_spa.button";

        void BalboaButton::press_action()
        {
            switch (this->button_id)
            {
            case 0:
                this->parent_->on_toggle_light();
                break;
            case 1:
                this->parent_->on_toggle_jet1();
                break;
            case 2:
                this->parent_->on_toggle_jet2();
                break;
            }
        }
    }
}