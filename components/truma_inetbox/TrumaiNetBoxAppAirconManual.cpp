#include "TrumaiNetBoxAppAirconManual.h"
#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "helpers.h"
#include "TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxAppAirconManual";

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

void TrumaiNetBoxAppAirconManual::create_update_data(StatusFrame *response, u_int8_t *response_len,
                                                     u_int8_t command_counter) {
  status_frame_create_empty(response, STATUS_FRAME_AIRCON_MANUAL_RESPONSE, sizeof(StatusFrameAirconManualResponse),
                            command_counter);

  response->airconManualResponse.mode = this->update_status_.mode;
  response->airconManualResponse.unknown_02 = this->update_status_.unknown_02;
  response->airconManualResponse.operation = this->update_status_.operation;
  response->airconManualResponse.energy_mix = this->update_status_.energy_mix;
  response->airconManualResponse.target_temp_aircon = this->update_status_.target_temp_aircon;

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameAirconManualResponse);

  TrumaStausFrameResponseStorage<StatusFrameAirconManual, StatusFrameAirconManualResponse>::update_submitted();
}

void TrumaiNetBoxAppAirconManual::dump_data() const {}

bool TrumaiNetBoxAppAirconManual::can_update() {
  return TrumaStausFrameResponseStorage<StatusFrameAirconManual, StatusFrameAirconManualResponse>::can_update() &&
         this->parent_->get_aircon_device() != TRUMA_DEVICE::UNKNOWN;
}

bool TrumaiNetBoxAppAirconManual::action_set_temp(u_int8_t temperature) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }

  auto update_data = this->update_prepare();

  update_data->target_temp_aircon = decimal_to_aircon_manual_temp(temperature);

  this->update_submit();
  return true;
}

}  // namespace truma_inetbox
}  // namespace esphome