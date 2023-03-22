#include "TrumaiNetBoxAppTimer.h"

namespace esphome {
namespace truma_inetbox {

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

}  // namespace truma_inetbox
}  // namespace esphome