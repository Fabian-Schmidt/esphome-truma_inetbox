#include "TrumaiNetBoxAppAirconManual.h"

namespace esphome {
namespace truma_inetbox {

StatusFrameAirconManualResponse *TrumaiNetBoxAppAirconManual::update_prepare() {
  // An update is currently going on.
  if (this->update_status_prepared_ || this->update_status_stale_) {
    return &this->update_status_;
  }

  // prepare status response
  this->update_status_ = {};
  this->update_status_.mode = this->data_.mode;
  this->update_status_.operation = this->data_.operation;
  this->update_status_.energy_mix = this->data_.energy_mix;
  this->update_status_.target_temp_aircon = this->data_.target_temp_aircon;

  this->update_status_prepared_ = true;
  return &this->update_status_;
}

}  // namespace truma_inetbox
}  // namespace esphome