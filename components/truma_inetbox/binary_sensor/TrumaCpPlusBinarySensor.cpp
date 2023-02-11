#include "TrumaCpPlusBinarySensor.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.binary_sensor";

void TrumaCpPlusBinarySensor::update() {
  if (this->parent_->get_lin_bus_fault() || (this->parent_->get_last_cp_plus_request() == 0)) {
    this->publish_state(false);
    return;
  }
  auto timeout = this->parent_->get_last_cp_plus_request() + 30 * 1000 * 1000 /* 30 seconds*/;
  this->publish_state(esp_timer_get_time() < timeout);
}

void TrumaCpPlusBinarySensor::dump_config() { ESP_LOGCONFIG("", "Truma CP Plus Binary Sensor"); }
}  // namespace truma_inetbox
}  // namespace esphome