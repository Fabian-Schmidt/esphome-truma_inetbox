#pragma once

#ifdef USE_ESP_IDF

#include "uart_component_esp_idf.h"

namespace esphome {
namespace uart {

class truma_IDFUARTComponent : public IDFUARTComponent {
 public:
  bool is_hw_serial() { return true; }
  uint8_t get_hw_serial_number() { return this->uart_num_; }
};

}  // namespace uart
}  // namespace esphome

#endif  // USE_ESP_IDF
