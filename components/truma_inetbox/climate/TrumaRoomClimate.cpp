#include "TrumaRoomClimate.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.truma_room_climate";
void TrumaRoomClimate::setup() {
  this->parent_->register_listener([this](const StatusFrameHeater *status_heater) {
    // Publish updated state
    this->target_temperature = temp_code_to_decimal(status_heater->target_temp_room);
    this->current_temperature = temp_code_to_decimal(status_heater->current_temp_room);
    this->mode = (status_heater->operating_status >= OperatingStatus::OPERATING_STATUS_START_OR_COOL_DOWN)
                     ? climate::CLIMATE_MODE_HEAT
                     : climate::CLIMATE_MODE_OFF;

    switch (status_heater->heating_mode) {
      case HeatingMode::HEATING_MODE_ECO:
        this->preset = climate::CLIMATE_PRESET_ECO;
        break;
      case HeatingMode::HEATING_MODE_HIGH:
        this->preset = climate::CLIMATE_PRESET_COMFORT;
        break;
      case HeatingMode::HEATING_MODE_BOOST:
        this->preset = climate::CLIMATE_PRESET_BOOST;
        break;
      default:
        this->preset = climate::CLIMATE_PRESET_NONE;
        break;
    }
    this->publish_state();
  });
}

void TrumaRoomClimate::dump_config() { ESP_LOGCONFIG(TAG, "Truma Room Climate"); }

void TrumaRoomClimate::control(const climate::ClimateCall &call) {
  if (call.get_target_temperature().has_value()) {
    float temp = *call.get_target_temperature();
    this->parent_->action_heater_room(static_cast<u_int8_t>(temp));
  }

  if (call.get_mode().has_value()) {
    // User requested mode change
    climate::ClimateMode mode = *call.get_mode();
    auto status_heater = this->parent_->get_status_heater();
    switch (mode) {
      case climate::CLIMATE_MODE_HEAT:
        if (status_heater->target_temp_room == TargetTemp::TARGET_TEMP_OFF) {
          this->parent_->action_heater_room(5);
        }
        break;
      default:
        this->parent_->action_heater_room(0);
        break;
    }
  }

  if (call.get_preset().has_value()) {
    climate::ClimatePreset pres = *call.get_preset();
    auto status_heater = this->parent_->get_status_heater();
    auto current_target_temp = temp_code_to_decimal(status_heater->target_temp_room);
    if (call.get_target_temperature().has_value()) {
      current_target_temp = *call.get_target_temperature();
    }
    switch (pres) {
      case climate::CLIMATE_PRESET_ECO:
        this->parent_->action_heater_room(current_target_temp, HeatingMode::HEATING_MODE_ECO);
        break;
      case climate::CLIMATE_PRESET_COMFORT:
        this->parent_->action_heater_room(current_target_temp, HeatingMode::HEATING_MODE_HIGH);
        break;
      case climate::CLIMATE_PRESET_BOOST:
        this->parent_->action_heater_room(current_target_temp, HeatingMode::HEATING_MODE_BOOST);
        break;
      default:
        this->parent_->action_heater_room(0);
        break;
    }
  }
}

climate::ClimateTraits TrumaRoomClimate::traits() {
  // The capabilities of the climate device
  auto traits = climate::ClimateTraits();
  traits.set_supports_current_temperature(true);
  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT});
  //  traits.set_supported_fan_modes({{
  //      climate::CLIMATE_FAN_LOW,
  //      climate::CLIMATE_FAN_MEDIUM,
  //      climate::CLIMATE_FAN_HIGH,
  //  }});
  traits.set_supported_presets({{
      climate::CLIMATE_PRESET_NONE,
      climate::CLIMATE_PRESET_ECO,
      climate::CLIMATE_PRESET_COMFORT,
      climate::CLIMATE_PRESET_BOOST,
  }});
  traits.set_visual_min_temperature(5);
  traits.set_visual_max_temperature(30);
  traits.set_visual_temperature_step(1);
  return traits;
}
}  // namespace truma_inetbox
}  // namespace esphome