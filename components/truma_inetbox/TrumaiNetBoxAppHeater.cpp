#include "TrumaiNetBoxAppHeater.h"
#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "helpers.h"
#include "TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxAppHeater";

StatusFrameHeaterResponse *TrumaiNetBoxAppHeater::update_prepare() {
  // An update is currently going on.
  if (this->update_status_prepared_ || this->update_status_stale_) {
    return &this->update_status_;
  }

  // prepare status heater response
  this->update_status_ = {};
  this->update_status_.target_temp_room = this->data_.target_temp_room;
  this->update_status_.heating_mode = this->data_.heating_mode;
  this->update_status_.el_power_level_a = this->data_.el_power_level_a;
  this->update_status_.target_temp_water = this->data_.target_temp_water;
  this->update_status_.el_power_level_b = this->data_.el_power_level_b;
  this->update_status_.energy_mix_a = this->data_.energy_mix_a;
  this->update_status_.energy_mix_b = this->data_.energy_mix_b;

  this->update_status_prepared_ = true;
  return &this->update_status_;
}

void TrumaiNetBoxAppHeater::create_update_data(StatusFrame *response, u_int8_t *response_len,
                                               u_int8_t command_counter) {
  status_frame_create_empty(response, STATUS_FRAME_HEATER_RESPONSE, sizeof(StatusFrameHeaterResponse), command_counter);

  response->heaterResponse.target_temp_room = this->update_status_.target_temp_room;
  response->heaterResponse.heating_mode = this->update_status_.heating_mode;
  response->heaterResponse.target_temp_water = this->update_status_.target_temp_water;
  response->heaterResponse.energy_mix_a = this->update_status_.energy_mix_a;
  response->heaterResponse.energy_mix_b = this->update_status_.energy_mix_a;
  response->heaterResponse.el_power_level_a = this->update_status_.el_power_level_a;
  response->heaterResponse.el_power_level_b = this->update_status_.el_power_level_a;

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameHeaterResponse);

  TrumaStausFrameResponseStorage<StatusFrameHeater, StatusFrameHeaterResponse>::update_submitted();
}

void TrumaiNetBoxAppHeater::dump_data() const {}

bool TrumaiNetBoxAppHeater::can_update() {
  return TrumaStausFrameResponseStorage<StatusFrameHeater, StatusFrameHeaterResponse>::can_update() &&
         this->parent_->get_heater_device() != TRUMA_DEVICE::UNKNOWN;
}

bool TrumaiNetBoxAppHeater::action_heater_room(u_int8_t temperature, HeatingMode mode) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto heater = this->update_prepare();

  heater->target_temp_room = decimal_to_room_temp(temperature);

  // Ensure `heating_mode` and `energy_mix_a` is set.
  if (heater->target_temp_room == TargetTemp::TARGET_TEMP_OFF) {
    heater->heating_mode = HeatingMode::HEATING_MODE_OFF;
  } else {
    if (this->parent_->get_heater_device() == TRUMA_DEVICE::HEATER_VARIO) {
      // If parameter `mode` contains a valid Heating mode use it or else use `AUTO`.
      if (mode == HeatingMode::HEATING_MODE_VARIO_HEAT_NIGHT || mode == HeatingMode::HEATING_MODE_VARIO_HEAT_AUTO ||
          mode == HeatingMode::HEATING_MODE_BOOST) {
        heater->heating_mode = mode;
      } else if (heater->heating_mode == HeatingMode::HEATING_MODE_OFF) {
        heater->heating_mode = HeatingMode::HEATING_MODE_VARIO_HEAT_AUTO;
      }
    } else {
      // HEATER_COMBI
      // If parameter `mode` contains a valid Heating mode use it or else use `ECO`.
      if (mode == HeatingMode::HEATING_MODE_ECO || mode == HeatingMode::HEATING_MODE_HIGH ||
          mode == HeatingMode::HEATING_MODE_BOOST) {
        heater->heating_mode = mode;
      } else if (heater->heating_mode == HeatingMode::HEATING_MODE_OFF) {
        heater->heating_mode = HeatingMode::HEATING_MODE_ECO;
      }
    }
  }
  if (heater->energy_mix_a == EnergyMix::ENERGY_MIX_NONE) {
    heater->energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
  }

  this->update_submit();
  return true;
}

bool TrumaiNetBoxAppHeater::action_heater_water(u_int8_t temperature) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto heater = this->update_prepare();

  heater->target_temp_water = decimal_to_water_temp(temperature);

  // Ensure `energy_mix_a` is set.
  if (heater->target_temp_water != TargetTemp::TARGET_TEMP_OFF && heater->energy_mix_a == EnergyMix::ENERGY_MIX_NONE) {
    heater->energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
  }

  this->update_submit();
  return true;
}

bool TrumaiNetBoxAppHeater::action_heater_water(TargetTemp temperature) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto heater = this->update_prepare();

  // If parameter `temperature` contains a valid mode use it or else use `OFF`.
  if (temperature == TargetTemp::TARGET_TEMP_WATER_ECO || temperature == TargetTemp::TARGET_TEMP_WATER_HIGH ||
      temperature == TargetTemp::TARGET_TEMP_WATER_BOOST) {
    heater->target_temp_water = temperature;
  } else {
    heater->target_temp_water = TargetTemp::TARGET_TEMP_OFF;
  }

  // Ensure `energy_mix_a` is set.
  if (heater->target_temp_water != TargetTemp::TARGET_TEMP_OFF && heater->energy_mix_a == EnergyMix::ENERGY_MIX_NONE) {
    heater->energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
  }

  this->update_submit();
  return true;
}

bool TrumaiNetBoxAppHeater::action_heater_electric_power_level(u_int16_t value) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto heater = this->update_prepare();

  heater->el_power_level_a = decimal_to_el_power_level(value);
  if (heater->el_power_level_a != ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0) {
    if (heater->energy_mix_a != EnergyMix::ENERGY_MIX_MIX &&
        heater->energy_mix_a != EnergyMix::ENERGY_MIX_ELECTRICITY) {
      heater->energy_mix_a = EnergyMix::ENERGY_MIX_MIX;
    }
  } else {
    heater->energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
  }

  this->update_submit();
  return true;
}

bool TrumaiNetBoxAppHeater::action_heater_energy_mix(EnergyMix energy_mix, ElectricPowerLevel el_power_level) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto heater = this->update_prepare();

  // If parameter `el_power_level` contains a valid mode use it.
  if (el_power_level == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0 ||
      el_power_level == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_900 ||
      el_power_level == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_1800) {
    heater->el_power_level_a = el_power_level;
  }

  if (energy_mix == EnergyMix::ENERGY_MIX_GAS) {
    heater->energy_mix_a = energy_mix;
    heater->el_power_level_a = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0;
  } else if (energy_mix == EnergyMix::ENERGY_MIX_MIX || energy_mix == EnergyMix::ENERGY_MIX_ELECTRICITY) {
    heater->energy_mix_a = energy_mix;
    // Electric energy is requested by user without a power level. Set it to minimum.
    if (heater->el_power_level_a == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0) {
      heater->el_power_level_a = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_900;
    }
  }

  // This last check is reached if invalid `energy_mix` parameter was submitted.
  if (heater->el_power_level_a != ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0) {
    if (heater->energy_mix_a != EnergyMix::ENERGY_MIX_MIX &&
        heater->energy_mix_a != EnergyMix::ENERGY_MIX_ELECTRICITY) {
      heater->energy_mix_a = EnergyMix::ENERGY_MIX_MIX;
    }
  } else {
    heater->energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
  }

  this->update_submit();
  return true;
}

}  // namespace truma_inetbox
}  // namespace esphome