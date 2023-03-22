#include "TrumaiNetBoxAppAirconAuto.h"
#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "helpers.h"

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
  //   this->update_status_.mode = this->data_.mode;
  //   this->update_status_.operation = this->data_.operation;
  //   this->update_status_.energy_mix = this->data_.energy_mix;
  //   this->update_status_.target_temp_aircon = this->data_.target_temp_aircon;

  this->update_status_prepared_ = true;
  return &this->update_status_;
}

void TrumaiNetBoxAppAirconAuto::create_update_data(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter){
  
  status_frame_create_empty(response, STATUS_FRAME_AIRCON_AUTO_RESPONSE, sizeof(StatusFrameAirconAutoResponse),
                            command_counter);

  // response->inner.airconAutoResponse.mode = this->update_status_.mode;
  // response->inner.airconAutoResponse.unknown_02 = this->update_status_.unknown_02;
  // response->inner.airconAutoResponse.operation = this->update_status_.operation;
  // response->inner.airconAutoResponse.energy_mix = this->update_status_.energy_mix;
  // response->inner.airconAutoResponse.target_temp_aircon = this->update_status_.target_temp_aircon;

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameAirconAutoResponse);
   TrumaStausFrameResponseStorage<StatusFrameAirconAuto, StatusFrameAirconAutoResponse>::update_submitted();
}
  
void TrumaiNetBoxAppAirconAuto::dump_data() const {}

}  // namespace truma_inetbox
}  // namespace esphome