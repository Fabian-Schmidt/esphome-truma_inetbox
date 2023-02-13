#pragma once

#ifdef USE_ESP32_FRAMEWORK_ARDUINO

#include "uart_component_esp32_arduino.h"

namespace esphome {
namespace uart {

class truma_ESP32ArduinoUARTComponent : public ESP32ArduinoUARTComponent {
 public:
  HardwareSerial *get_hw_serial() { return this->hw_serial_; }
  uint8_t get_hw_serial_number() { return this->number_; }
};

}  // namespace uart
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ARDUINO
