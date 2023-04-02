#include "TrumaiNetBoxAppClock.h"
#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "helpers.h"
#include "TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxAppClock";

void TrumaiNetBoxAppClock::dump_data() const {
  ESP_LOGD(TAG, "StatusFrameClock %02d:%02d:%02d", this->data_.clock_hour, this->data_.clock_minute,
           this->data_.clock_second);
}

#ifdef USE_TIME
bool TrumaiNetBoxAppClock::action_write_time() {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Truma.");
    return false;
  }

  if (this->parent_->get_time() == nullptr) {
    ESP_LOGW(TAG, "Missing system time component.");
    return false;
  }

  auto now = this->parent_->get_time()->now();
  if (!now.is_valid()) {
    ESP_LOGW(TAG, "Invalid system time, not syncing to CP Plus.");
    return false;
  }

  // The behaviour of this method is special.
  // Just an update is marked. The actual package is prepared when CP Plus asks for the data in the
  // `lin_multiframe_recieved` method.
  this->update_submit();
  return true;
}

void TrumaiNetBoxAppClock::create_update_data(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter) {
  if (this->parent_->get_time() != nullptr) {
    ESP_LOGD(TAG, "Requested read: Sending clock update");
    // read time live
    auto now = this->parent_->get_time()->now();

    status_frame_create_empty(response, STATUS_FRAME_CLOCK_RESPONSE, sizeof(StatusFrameClock), command_counter);

    response->clock.clock_hour = now.hour;
    response->clock.clock_minute = now.minute;
    response->clock.clock_second = now.second;
    response->clock.display_1 = 0x1;
    response->clock.display_2 = 0x1;
    response->clock.clock_mode = this->data_.clock_mode;

    status_frame_calculate_checksum(response);
    (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameClock);
  }
  this->update_status_unsubmitted_ = false;
}

#endif  // USE_TIME

}  // namespace truma_inetbox
}  // namespace esphome