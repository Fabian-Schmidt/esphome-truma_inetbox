#pragma once

#include <vector>
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace truma_inetbox {

enum class LIN_CHECKSUM { LIN_CHECKSUM_VERSION_1, LIN_CHECKSUM_VERSION_2 };

class LinBusListener : public PollingComponent, public uart::UARTDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }

  void dump_config() override;
  void setup() override;

  void set_lin_checksum(LIN_CHECKSUM val) { this->lin_checksum_ = val; }
  void set_cs_pin(GPIOPin *pin) { this->cs_pin_ = pin; }
  void set_fault_pin(GPIOPin *pin) { this->fault_pin_ = pin; }
  void set_observer_mode(bool val) { this->observer_mode_ = val; }
  bool get_lin_bus_fault() { return fault_on_lin_bus_reported_; }

 protected:
  LIN_CHECKSUM lin_checksum_;
  GPIOPin *cs_pin_;
  GPIOPin *fault_pin_;
  bool observer_mode_ = false;

  void write_lin_answer_(const u_int8_t *data, size_t len);

  virtual bool answer_lin_order_(const u_int8_t pid) = 0;
  virtual void lin_message_recieved_(const u_int8_t pid, const u_int8_t *message, u_int8_t length) = 0;

 private:
  // Microseconds per UART Baud
  u_int32_t time_per_baud_;
  // 9.. 15
  u_int8_t lin_break_length = 13;
  // Microseconds per LIN Break
  u_int32_t time_per_lin_break_;
  u_int8_t frame_length_ = (8 /* bits */ + 1 /* Start bit */ + 2 /* Stop bits */);
  // Microseconds per UART Byte (UART Frame)
  u_int32_t time_per_pid_;
  // Microseconds per UART Byte (UART Frame)
  u_int32_t time_per_first_byte_;
  // Microseconds per UART Byte (UART Frame)
  u_int32_t time_per_byte_;

  bool fault_on_lin_bus_reported_ = false;
  bool can_write_lin_answer_ = false;

  enum read_state {
    READ_STATE_BREAK,
    READ_STATE_SYNC,
    READ_STATE_SID,
    READ_STATE_DATA,
    READ_STATE_ACT,
  };
  read_state current_state_ = READ_STATE_BREAK;
  u_int8_t current_PID_with_parity_ = 0x00;
  u_int8_t current_PID_ = 0x00;
  bool current_data_valid = true;
  u_int8_t current_data_count_ = 0;
  // up to 8 byte data frame + CRC
  u_int8_t current_data_[9] = {};
  // // Time when the last LIN data was available.
  // int64_t last_data_recieved_;

  void read_lin_frame_();
  void clear_uart_buffer_();
  void setup_framework();
};

}  // namespace truma_inetbox
}  // namespace esphome
