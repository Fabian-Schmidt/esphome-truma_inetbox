#include "TrumaiNetBoxAppHeater.h"

namespace esphome {
namespace truma_inetbox {

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

}  // namespace truma_inetbox
}  // namespace esphome