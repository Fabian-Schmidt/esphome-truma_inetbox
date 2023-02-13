#pragma once

#ifdef USE_ESP_IDF

#include "uart_component_esp_idf.h"

namespace esphome {
namespace uart {

class truma_IDFUARTComponent : public IDFUARTComponent {
 public:
  uint8_t get_hw_serial_number() { return this->uart_num_; }
  // `QueueHandle_t uart_event_queue_;` is also added to base class.
  QueueHandle_t *get_uart_event_queue() { return &this->uart_event_queue_; }
};

}  // namespace uart
}  // namespace esphome

#endif  // USE_ESP_IDF
