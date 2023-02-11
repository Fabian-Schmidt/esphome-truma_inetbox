#pragma once

#ifdef USE_RP2040

#include "uart_component_rp2040.h"

namespace esphome {
namespace uart {

class truma_RP2040UartComponent : public RP2040UartComponent {
 public:
  bool is_hw_serial() { return this->hw_serial_; }
  HardwareSerial *get_hw_serial() { return this->serial_; }
};

}  // namespace uart
}  // namespace esphome

#endif  // USE_RP2040
