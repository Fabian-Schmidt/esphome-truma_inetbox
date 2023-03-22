#include "TrumaTime.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.time";

void TrumaTime::setup() {
  this->parent_->get_clock()->add_on_message_callback([this](const StatusFrameClock *status_clock) {
    if (this->auto_disable_count_ > 0) {
      if (this->read_time() && this->auto_disable_) {
        this->auto_disable_count_--;
      }
    }
  });
}

void TrumaTime::update() {}

void TrumaTime::dump_config() { ESP_LOGCONFIG(TAG, "Truma Time", this); }

bool TrumaTime::read_time() {
  if (!this->parent_->get_clock()->get_status_valid()) {
    return false;
  }
  auto status_clock = this->parent_->get_clock()->get_status();

  time::ESPTime rtc_time{.second = status_clock->clock_second,
                         .minute = status_clock->clock_minute,
                         .hour = status_clock->clock_hour,
                         .day_of_week = 1,
                         .day_of_month = 1,
                         .day_of_year = 1,  // ignored by recalc_timestamp_utc(false)
                         .month = 1,
                         .year = 2020};
  if (!rtc_time.is_valid()) {
    ESP_LOGE(TAG, "Invalid RTC time, not syncing to system clock.");
    return false;
  }
  time::RealTimeClock::synchronize_epoch_(rtc_time.timestamp);
  return true;
}

}  // namespace truma_inetbox
}  // namespace esphome