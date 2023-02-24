#include "TrumaTimerBinarySensor.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.binary_sensor";

void TrumaTimerBinarySensor::setup() {
  this->parent_->register_listener([this](const StatusFrameTimer *status_timer) {
    switch (this->type_) {
      case TRUMA_BINARY_SENSOR_TYPE::TIMER_ACTIVE:
        this->publish_state(status_timer->timer_active == TimerActive::TIMER_ACTIVE_ON);
        break;
      case TRUMA_BINARY_SENSOR_TYPE::TIMER_ROOM:
        this->publish_state(status_timer->timer_target_temp_room != TargetTemp::TARGET_TEMP_OFF);
        break;
      case TRUMA_BINARY_SENSOR_TYPE::TIMER_WATER:
        this->publish_state(status_timer->timer_target_temp_water != TargetTemp::TARGET_TEMP_OFF);
        break;
      default:
        break;
    }
  });
}

void TrumaTimerBinarySensor::dump_config() {
  ESP_LOGCONFIG("", "Truma Timer Binary Sensor");
  ESP_LOGCONFIG(TAG, "Type %u", (size_t) this->type_);
}
}  // namespace truma_inetbox
}  // namespace esphome