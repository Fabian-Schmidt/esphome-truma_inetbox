#include "TrumaiNetBoxAppAirconAuto.h"
#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "helpers.h"
#include "TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxAppAirconAuto";

StatusFrameAirconAutoResponse *TrumaiNetBoxAppAirconAuto::update_prepare() {
  // An update is currently going on.
  if (this->update_status_prepared_ || this->update_status_stale_) {
    return &this->update_status_;
  }

  // prepare status response
  this->update_status_ = {};
  this->update_status_.energy_mix_a = this->data_.energy_mix_a;
  this->update_status_.unknown_02 = this->data_.unknown_02;
  this->update_status_.energy_mix_b = this->data_.energy_mix_b;
  this->update_status_.unknown_04 = this->data_.unknown_04;
  this->update_status_.unknown_05 = this->data_.unknown_05;
  this->update_status_.unknown_06 = this->data_.unknown_06;
  this->update_status_.target_temp_aircon_auto = this->data_.target_temp_aircon_auto;
  this->update_status_.el_power_level_a = this->data_.el_power_level_a;
  this->update_status_.unknown_11 = this->data_.unknown_11;
  this->update_status_.unknown_12 = this->data_.unknown_12;
  this->update_status_.el_power_level_b = this->data_.el_power_level_b;

  this->update_status_prepared_ = true;
  return &this->update_status_;
}

void TrumaiNetBoxAppAirconAuto::create_update_data(StatusFrame *response, u_int8_t *response_len,
                                                   u_int8_t command_counter) {
  status_frame_create_empty(response, STATUS_FRAME_AIRCON_AUTO_RESPONSE, sizeof(StatusFrameAirconAutoResponse),
                            command_counter);

  response->airconAutoResponse.energy_mix_a = this->update_status_.energy_mix_a;
  response->airconAutoResponse.unknown_02 = this->update_status_.unknown_02;
  response->airconAutoResponse.energy_mix_b = this->update_status_.energy_mix_b;
  response->airconAutoResponse.unknown_04 = this->update_status_.unknown_04;
  response->airconAutoResponse.unknown_05 = this->update_status_.unknown_05;
  response->airconAutoResponse.unknown_06 = this->update_status_.unknown_06;
  response->airconAutoResponse.target_temp_aircon_auto = this->update_status_.target_temp_aircon_auto;
  response->airconAutoResponse.el_power_level_a = this->update_status_.el_power_level_a;
  response->airconAutoResponse.unknown_11 = this->update_status_.unknown_11;
  response->airconAutoResponse.unknown_12 = this->update_status_.unknown_12;
  response->airconAutoResponse.el_power_level_b = this->update_status_.el_power_level_b;

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameAirconAutoResponse);
  TrumaStausFrameResponseStorage<StatusFrameAirconAuto, StatusFrameAirconAutoResponse>::update_submitted();
}

void TrumaiNetBoxAppAirconAuto::dump_data() const {}

bool TrumaiNetBoxAppAirconAuto::can_update() {
  return TrumaStausFrameResponseStorage<StatusFrameAirconAuto, StatusFrameAirconAutoResponse>::can_update() &&
         this->parent_->get_aircon_device() != TRUMA_DEVICE::UNKNOWN;
}

}  // namespace truma_inetbox
}  // namespace esphome