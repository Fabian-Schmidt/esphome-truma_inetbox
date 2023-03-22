#include "TrumaAirconManualNumber.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.aircon_manual_number";

void TrumaAirconManualNumber::setup() {
  this->parent_->get_aircon_manual()->add_on_message_callback([this](const StatusFrameAirconManual *status) {
    switch (this->type_) {
      case TRUMA_NUMBER_TYPE::AIRCON_MANUAL_TEMPERATURE:
        this->publish_state(temp_code_to_decimal(status->target_temp_aircon, 0));
        break;
      default:
        break;
    }
  });
}

void TrumaAirconManualNumber::control(float value) {
  switch (this->type_) {
    case TRUMA_NUMBER_TYPE::AIRCON_MANUAL_TEMPERATURE:
      this->parent_->get_aircon_manual()->action_set_temp(static_cast<u_int8_t>(value));
      break;
    default:
      break;
  }
}

void TrumaAirconManualNumber::dump_config() {
  LOG_NUMBER("", "Truma Aircon Manual Number", this);
  ESP_LOGCONFIG(TAG, "  Type '%s'", enum_to_c_str(this->type_));
}
}  // namespace truma_inetbox
}  // namespace esphome