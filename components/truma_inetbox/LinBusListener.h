#pragma once

#include "LinBusLog.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#ifdef USE_ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif  // USE_ESP32
#ifdef USE_RP2040
#include <hardware/uart.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <queue.h>
#endif  // USE_RP2040

#ifndef  TRUMA_MSG_QUEUE_LENGTH
#define TRUMA_MSG_QUEUE_LENGTH 6
#endif
#ifndef  TRUMA_LOG_QUEUE_LENGTH
#define TRUMA_LOG_QUEUE_LENGTH 6
#endif

namespace esphome {
namespace truma_inetbox {

enum class LIN_CHECKSUM { LIN_CHECKSUM_VERSION_1, LIN_CHECKSUM_VERSION_2 };

struct QUEUE_LIN_MSG {
  u_int8_t current_PID;
  u_int8_t data[8];
  u_int8_t len;
};

class LinBusListener : public PollingComponent, public uart::UARTDevice {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }

  void dump_config() override;
  void setup() override;
  void update() override;

  void set_lin_checksum(LIN_CHECKSUM val) { this->lin_checksum_ = val; }
  void set_cs_pin(GPIOPin *pin) { this->cs_pin_ = pin; }
  void set_fault_pin(GPIOPin *pin) { this->fault_pin_ = pin; }
  void set_observer_mode(bool val) { this->observer_mode_ = val; }
  bool get_lin_bus_fault() { return fault_on_lin_bus_reported_ > 3; }

  void process_lin_msg_queue(TickType_t xTicksToWait);
  void process_log_queue(TickType_t xTicksToWait);

#ifdef USE_RP2040
  // Return is the expected wait time till next data check is recommended.
  u_int32_t onSerialEvent();
#endif  // USE_RP2040

 protected:
  LIN_CHECKSUM lin_checksum_ = LIN_CHECKSUM::LIN_CHECKSUM_VERSION_2;
  GPIOPin *cs_pin_ = nullptr;
  GPIOPin *fault_pin_ = nullptr;
  bool observer_mode_ = false;

  void write_lin_answer_(const u_int8_t *data, u_int8_t len);
  bool check_for_lin_fault_();
  virtual bool answer_lin_order_(const u_int8_t pid) = 0;
  virtual void lin_message_recieved_(const u_int8_t pid, const u_int8_t *message, u_int8_t length) = 0;

 private:
  // Microseconds per UART Baud
  u_int32_t time_per_baud_;
  // 9.. 15
  const u_int8_t lin_break_length = 13;
  // Microseconds per LIN Break
  u_int32_t time_per_lin_break_;
  const u_int8_t frame_length_ = (8 /* bits */ + 1 /* Start bit */ + 2 /* Stop bits */);
  // Microseconds per UART Byte (UART Frame)
  u_int32_t time_per_pid_;
  // Microseconds per UART Byte (UART Frame)
  u_int32_t time_per_first_byte_;
  // Microseconds per UART Byte (UART Frame)
  u_int32_t time_per_byte_;

  u_int8_t fault_on_lin_bus_reported_ = 0;
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
  bool current_PID_order_answered_ = false;
  bool current_data_valid = true;
  u_int8_t current_data_count_ = 0;
  // up to 8 byte data frame + CRC
  u_int8_t current_data_[9] = {};
  // // Time when the last LIN data was available.
  uint32_t last_data_recieved_ = 0;

  void current_state_reset_() {
    this->current_state_ = READ_STATE_BREAK;
    this->current_PID_with_parity_ = 0x00;
    this->current_PID_ = 0x00;
    this->current_PID_order_answered_ = false;
    this->current_data_valid = true;
    this->current_data_count_ = 0;
    memset(this->current_data_, 0, sizeof(this->current_data_));
  };
  void onReceive_();
  void read_lin_frame_();
  void clear_uart_buffer_();
  void setup_framework();

  uint8_t lin_msg_static_queue_storage[TRUMA_MSG_QUEUE_LENGTH * sizeof(QUEUE_LIN_MSG)];
  StaticQueue_t lin_msg_static_queue_;
  QueueHandle_t lin_msg_queue_ =
      xQueueCreateStatic(/* uxQueueLength */ TRUMA_MSG_QUEUE_LENGTH,
                         /* uxItemSize */ sizeof(QUEUE_LIN_MSG),
                         /* pucQueueStorageBuffer */ lin_msg_static_queue_storage, &lin_msg_static_queue_);

#if ESPHOME_LOG_LEVEL > ESPHOME_LOG_LEVEL_NONE
  uint8_t log_static_queue_storage[TRUMA_LOG_QUEUE_LENGTH * sizeof(QUEUE_LOG_MSG)];
  StaticQueue_t log_static_queue_;
  QueueHandle_t log_queue_ =
      xQueueCreateStatic(/* uxQueueLength */ TRUMA_LOG_QUEUE_LENGTH,
                         /* uxItemSize */ sizeof(QUEUE_LOG_MSG),
                         /* pucQueueStorageBuffer */ log_static_queue_storage, &log_static_queue_);
#endif

#ifdef USE_ESP32
  TaskHandle_t eventTaskHandle_;
  static void eventTask_(void *args);
#endif  // USE_ESP32
#ifdef USE_ESP32_FRAMEWORK_ESP_IDF
  TaskHandle_t uartEventTaskHandle_;
  static void uartEventTask_(void *args);
#endif  // USE_ESP32_FRAMEWORK_ESP_IDF
#ifdef USE_RP2040
  u_int8_t uart_number_ = 0;
  uart_inst_t *uart_ = nullptr;
#endif  // USE_RP2040
};

}  // namespace truma_inetbox
}  // namespace esphome