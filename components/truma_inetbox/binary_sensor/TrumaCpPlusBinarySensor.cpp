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
  const auto timeout = this->parent_->get_last_cp_plus_request() + 90 * 1000 * 1000 /* 90 seconds*/;
  this->publish_state(micros() < timeout);
}

void TrumaCpPlusBinarySensor::dump_config() { ESP_LOGCONFIG("", "Truma CP Plus Binary Sensor"); }
}  // namespace truma_inetbox
}  // namespace esphome