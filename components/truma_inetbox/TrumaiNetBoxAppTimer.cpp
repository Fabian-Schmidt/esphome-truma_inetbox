#include "TrumaiNetBoxAppTimer.h"
#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "helpers.h"
#include "TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxAppTimer";

StatusFrameTimerResponse *TrumaiNetBoxAppTimer::update_prepare() {
  // An update is currently going on.
  if (this->update_status_prepared_ || this->update_status_stale_) {
    return &this->update_status_;
  }

  // prepare status heater response
  this->update_status_ = {};
  this->update_status_.timer_target_temp_room = this->data_.timer_target_temp_room;
  this->update_status_.timer_heating_mode = this->data_.timer_heating_mode;
  this->update_status_.timer_el_power_level_a = this->data_.timer_el_power_level_a;
  this->update_status_.timer_target_temp_water = this->data_.timer_target_temp_water;
  this->update_status_.timer_el_power_level_b = this->data_.timer_el_power_level_b;
  this->update_status_.timer_energy_mix_a = this->data_.timer_energy_mix_a;
  this->update_status_.timer_energy_mix_b = this->data_.timer_energy_mix_b;
  this->update_status_.timer_resp_active = this->data_.timer_active;
  this->update_status_.timer_resp_start_minutes = this->data_.timer_start_minutes;
  this->update_status_.timer_resp_start_hours = this->data_.timer_start_hours;
  this->update_status_.timer_resp_stop_minutes = this->data_.timer_stop_minutes;
  this->update_status_.timer_resp_stop_hours = this->data_.timer_stop_hours;

  this->update_status_prepared_ = true;
  return &this->update_status_;
}

void TrumaiNetBoxAppTimer::create_update_data(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter) {
  status_frame_create_empty(response, STATUS_FRAME_TIMER_RESPONSE, sizeof(StatusFrameTimerResponse), command_counter);

  response->timerResponse.timer_target_temp_room = this->update_status_.timer_target_temp_room;
  response->timerResponse.timer_heating_mode = this->update_status_.timer_heating_mode;
  response->timerResponse.timer_target_temp_water = this->update_status_.timer_target_temp_water;
  response->timerResponse.timer_energy_mix_a = this->update_status_.timer_energy_mix_a;
  response->timerResponse.timer_energy_mix_b = this->update_status_.timer_energy_mix_a;
  response->timerResponse.timer_el_power_level_a = this->update_status_.timer_el_power_level_a;
  response->timerResponse.timer_el_power_level_b = this->update_status_.timer_el_power_level_a;
  response->timerResponse.timer_resp_active = this->update_status_.timer_resp_active;
  response->timerResponse.timer_resp_start_hours = this->update_status_.timer_resp_start_hours;
  response->timerResponse.timer_resp_start_minutes = this->update_status_.timer_resp_start_minutes;
  response->timerResponse.timer_resp_stop_hours = this->update_status_.timer_resp_stop_hours;
  response->timerResponse.timer_resp_stop_minutes = this->update_status_.timer_resp_stop_minutes;

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameTimerResponse);

  TrumaStausFrameResponseStorage<StatusFrameTimer, StatusFrameTimerResponse>::update_submitted();
}

void TrumaiNetBoxAppTimer::dump_data() const {
  ESP_LOGD(TAG, "StatusFrameTimer target_temp_room: %f target_temp_water: %f %02u:%02u -> %02u:%02u %s",
           temp_code_to_decimal(this->data_.timer_target_temp_room),
           temp_code_to_decimal(this->data_.timer_target_temp_water), this->data_.timer_start_hours,
           this->data_.timer_start_minutes, this->data_.timer_stop_hours, this->data_.timer_stop_minutes,
           ((u_int8_t) this->data_.timer_active ? " ON" : " OFF"));
}

bool TrumaiNetBoxAppTimer::action_timer_disable() {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  auto timer = this->update_prepare();

  timer->timer_resp_active = TimerActive::TIMER_ACTIVE_OFF;

  this->update_submit();
  return true;
}

bool TrumaiNetBoxAppTimer::action_timer_activate(u_int16_t start, u_int16_t stop, u_int8_t room_temperature,
                                                 HeatingMode mode, u_int8_t water_temperature, EnergyMix energy_mix,
                                                 ElectricPowerLevel el_power_level) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }
  if (start > 1440 || stop > 1440) {
    ESP_LOGW(TAG, "Invalid values start/stop submitted.");
    return false;
  }

  auto timer = this->update_prepare();

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
    if (this->parent_->get_heater_device() == TRUMA_DEVICE::HEATER_VARIO) {
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

  timer->timer_target_temp_water = decimal_to_water_temp(water_temperature);

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

  this->update_submit();
  return true;
}

}  // namespace truma_inetbox
}  // namespace esphome