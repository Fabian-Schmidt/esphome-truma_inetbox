#include "TrumaiNetBoxApp.h"
#include "TrumaStatusFrame.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxApp";

bool TrumaiNetBoxApp::action_heater_room(u_int8_t temperature, HeatingMode mode) {
  if (!this->truma_heater_can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto heater = this->update_heater_prepare();

  heater->target_temp_room = decimal_to_room_temp(temperature);

  // Ensure `heating_mode` and `energy_mix_a` is set.
  if (heater->target_temp_room == TargetTemp::TARGET_TEMP_OFF) {
    heater->heating_mode = HeatingMode::HEATING_MODE_OFF;
  } else {
    if (this->heater_device_ == TRUMA_DEVICE::HEATER_VARIO) {
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

  this->update_heater_submit();
  return true;
}

bool TrumaiNetBoxApp::action_heater_water(u_int8_t temperature) {
  if (!this->truma_heater_can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto heater = this->update_heater_prepare();

  heater->target_temp_water = deciaml_to_water_temp(temperature);

  // Ensure `energy_mix_a` is set.
  if (heater->target_temp_water != TargetTemp::TARGET_TEMP_OFF && heater->energy_mix_a == EnergyMix::ENERGY_MIX_NONE) {
    heater->energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
  }

  this->update_heater_submit();
  return true;
}

bool TrumaiNetBoxApp::action_heater_water(TargetTemp temperature) {
  if (!this->truma_heater_can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto heater = this->update_heater_prepare();

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

  this->update_heater_submit();
  return true;
}

bool TrumaiNetBoxApp::action_heater_electric_power_level(u_int16_t value) {
  if (!this->truma_heater_can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto heater = this->update_heater_prepare();

  heater->el_power_level_a = decimal_to_el_power_level(value);
  if (heater->el_power_level_a != ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0) {
    if (heater->energy_mix_a != EnergyMix::ENERGY_MIX_MIX &&
        heater->energy_mix_a != EnergyMix::ENERGY_MIX_ELECTRICITY) {
      heater->energy_mix_a = EnergyMix::ENERGY_MIX_MIX;
    }
  } else {
    heater->energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
  }

  this->update_heater_submit();
  return true;
}

bool TrumaiNetBoxApp::action_heater_energy_mix(EnergyMix energy_mix, ElectricPowerLevel el_power_level) {
  if (!this->truma_heater_can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto heater = this->update_heater_prepare();

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

  this->update_heater_submit();
  return true;
}

bool TrumaiNetBoxApp::action_timer_disable() {
  if (!this->truma_timer_can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto timer = this->update_timer_prepare();

  timer->timer_resp_active = TimerActive::TIMER_ACTIVE_OFF;

  this->update_timer_submit();
  return true;
}

bool TrumaiNetBoxApp::action_timer_activate(u_int16_t start, u_int16_t stop, u_int8_t room_temperature,
                                            HeatingMode mode, u_int8_t water_temperature, EnergyMix energy_mix,
                                            ElectricPowerLevel el_power_level) {
  if (!this->truma_timer_can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  if (start > 1440 || stop > 1440) {
    ESP_LOGW(TAG, "Invalid values start/stop submitted.");
    return false;
  }

  auto timer = this->update_timer_prepare();

  timer->timer_resp_active = TimerActive::TIMER_ACTIVE_ON;
  timer->timer_resp_start_hours = start / 60;
  timer->timer_resp_start_minutes = start % 60;
  timer->timer_resp_stop_hours = stop / 60;
  timer->timer_resp_stop_minutes = stop % 60;
  timer->timer_target_temp_room = decimal_to_room_temp(room_temperature);

  // Ensure `timer_heating_mode` and `timer_energy_mix_a` is set.
  if (timer->timer_target_temp_room == TargetTemp::TARGET_TEMP_OFF) {
    timer->timer_heating_mode = HeatingMode::HEATING_MODE_OFF;
  } else {
    if (this->heater_device_ == TRUMA_DEVICE::HEATER_VARIO) {
      // If parameter `mode` contains a valid Heating mode use it or else use `AUTO`.
      if (mode == HeatingMode::HEATING_MODE_VARIO_HEAT_NIGHT || mode == HeatingMode::HEATING_MODE_VARIO_HEAT_AUTO ||
          mode == HeatingMode::HEATING_MODE_BOOST) {
        timer->timer_heating_mode = mode;
      } else if (timer->timer_heating_mode == HeatingMode::HEATING_MODE_OFF) {
        timer->timer_heating_mode = HeatingMode::HEATING_MODE_VARIO_HEAT_AUTO;
      }
    } else {
      // HEATER_COMBI
      // If parameter `mode` contains a valid Heating mode use it or else use `ECO`.
      if (mode == HeatingMode::HEATING_MODE_ECO || mode == HeatingMode::HEATING_MODE_HIGH ||
          mode == HeatingMode::HEATING_MODE_BOOST) {
        timer->timer_heating_mode = mode;
      } else if (timer->timer_heating_mode == HeatingMode::HEATING_MODE_OFF) {
        timer->timer_heating_mode = HeatingMode::HEATING_MODE_ECO;
      }
    }
  }

  timer->timer_target_temp_water = deciaml_to_water_temp(water_temperature);

  // If parameter `el_power_level` contains a valid mode use it.
  if (el_power_level == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0 ||
      el_power_level == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_900 ||
      el_power_level == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_1800) {
    timer->timer_el_power_level_a = el_power_level;
  }

  // Ensure `timer_energy_mix_a` is set
  if (timer->timer_energy_mix_a == EnergyMix::ENERGY_MIX_NONE) {
    timer->timer_energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
  }

  // User has supplied a `energy_mix`
  if (energy_mix == EnergyMix::ENERGY_MIX_GAS) {
    timer->timer_energy_mix_a = energy_mix;
    timer->timer_el_power_level_a = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0;
  } else if (energy_mix == EnergyMix::ENERGY_MIX_MIX || energy_mix == EnergyMix::ENERGY_MIX_ELECTRICITY) {
    timer->timer_energy_mix_a = energy_mix;
    // Electric energy is requested by user without a power level. Set it to minimum.
    if (timer->timer_el_power_level_a == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0) {
      timer->timer_el_power_level_a = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_900;
    }
  }

  this->update_timer_submit();
  return true;
}

#ifdef USE_TIME
bool TrumaiNetBoxApp::action_write_time() {
  if (!this->truma_clock_can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }

  if (this->time_ == nullptr) {
    ESP_LOGW(TAG, "Missing system time component.");
    return false;
  }

  auto now = this->time_->now();
  if (!now.is_valid()) {
    ESP_LOGW(TAG, "Invalid system time, not syncing to CP Plus.");
    return false;
  }

  // The behaviour of this method is special.
  // Just an update is marked. The actual package is prepared when CP Plus asks for the data in the
  // `lin_multiframe_recieved` method.

  this->update_clock_submit();
  return true;
}
#endif  // USE_TIME

}  // namespace truma_inetbox
}  // namespace esphome