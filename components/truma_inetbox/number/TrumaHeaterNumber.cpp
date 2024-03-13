#include "TrumaHeaterNumber.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.heater_number";

void TrumaHeaterNumber::setup() {
  this->parent_->get_heater()->add_on_message_callback([this](const StatusFrameHeater *status_heater) {
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
      default:
        break;
    }
  });
}

void TrumaHeaterNumber::control(float value) {
  switch (this->type_) {
    case TRUMA_NUMBER_TYPE::TARGET_ROOM_TEMPERATURE:
      this->parent_->get_heater()->action_heater_room(static_cast<u_int8_t>(value));
      break;
    case TRUMA_NUMBER_TYPE::TARGET_WATER_TEMPERATURE:
      this->parent_->get_heater()->action_heater_water(static_cast<u_int8_t>(value));
      break;
    case TRUMA_NUMBER_TYPE::ELECTRIC_POWER_LEVEL:
      this->parent_->get_heater()->action_heater_electric_power_level(static_cast<u_int16_t>(value));
      break;
    default:
      break;
  }
}

void TrumaHeaterNumber::dump_config() {
  LOG_NUMBER("", "Truma Heater Number", this);
  ESP_LOGCONFIG(TAG, "  Type '%s'", enum_to_c_str(this->type_));
}
}  // namespace truma_inetbox
}  // namespace esphome