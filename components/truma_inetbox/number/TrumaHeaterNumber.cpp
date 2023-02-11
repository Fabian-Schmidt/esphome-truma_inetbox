#include "TrumaHeaterNumber.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.sensor";

void TrumaHeaterNumber::setup() {
  this->parent_->register_listener([this](const StatusFrameHeater *status_heater) {
    switch (this->type_) {
      case TRUMA_NUMBER_TYPE::TARGET_ROOM_TEMPERATURE:
        this->publish_state(temp_code_to_decimal(status_heater->target_temp_room, 0));
        break;
      case TRUMA_NUMBER_TYPE::TARGET_WATER_TEMPERATURE:
        this->publish_state(temp_code_to_decimal(status_heater->target_temp_water, 0));
        break;
      case TRUMA_NUMBER_TYPE::ELECTRIC_POWER_LEVEL:
        this->publish_state(static_cast<float>(status_heater->el_power_level_a));
        break;
    }
  });
}

void TrumaHeaterNumber::control(float value) {
  switch (this->type_) {
    case TRUMA_NUMBER_TYPE::TARGET_ROOM_TEMPERATURE:
      this->parent_->action_heater_room(static_cast<u_int8_t>(value));
      break;
    case TRUMA_NUMBER_TYPE::TARGET_WATER_TEMPERATURE:
      this->parent_->action_heater_water(static_cast<u_int8_t>(value));
      break;
    case TRUMA_NUMBER_TYPE::ELECTRIC_POWER_LEVEL:
      this->parent_->action_heater_water(static_cast<u_int16_t>(value));
      break;
  }
}

void TrumaHeaterNumber::dump_config() {
  ESP_LOGCONFIG("", "Truma Heater Number");
  ESP_LOGCONFIG(TAG, "Type %u", this->type_);
}
}  // namespace truma_inetbox
}  // namespace esphome