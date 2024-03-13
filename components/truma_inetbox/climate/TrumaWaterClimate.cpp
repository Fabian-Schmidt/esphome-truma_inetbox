#include "TrumaWaterClimate.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.water_climate";

void TrumaWaterClimate::setup() {
  this->parent_->get_heater()->add_on_message_callback([this](const StatusFrameHeater *status_heater) {
    // Publish updated state
    this->target_temperature = water_temp_200_fix(temp_code_to_decimal(status_heater->target_temp_water));
    this->current_temperature = temp_code_to_decimal(status_heater->current_temp_water);
    this->mode = (status_heater->target_temp_water == TargetTemp::TARGET_TEMP_OFF) ? climate::CLIMATE_MODE_OFF
                                                                                   : climate::CLIMATE_MODE_HEAT;
    this->publish_state();
  });
}

void TrumaWaterClimate::dump_config() { LOG_CLIMATE(TAG, "Truma Climate", this); }

void TrumaWaterClimate::control(const climate::ClimateCall &call) {
  if (call.get_target_temperature().has_value()) {
    float temp = *call.get_target_temperature();
    this->parent_->get_heater()->action_heater_water(static_cast<u_int8_t>(temp));
  }

  if (call.get_mode().has_value()) {
    climate::ClimateMode mode = *call.get_mode();
    auto status_heater = this->parent_->get_heater()->get_status();
    switch (mode) {
      case climate::CLIMATE_MODE_HEAT:
        if (status_heater->target_temp_water == TargetTemp::TARGET_TEMP_OFF) {
          this->parent_->get_heater()->action_heater_water(40);
        }
        break;
      default:
        this->parent_->get_heater()->action_heater_water(0);
        break;
    }
  }
}

climate::ClimateTraits TrumaWaterClimate::traits() {
  // The capabilities of the climate device
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
  traits.set_visual_min_temperature(40);
  traits.set_visual_max_temperature(80);
  traits.set_visual_temperature_step(20);
  return traits;
}
}  // namespace truma_inetbox
}  // namespace esphome