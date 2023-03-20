#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#ifdef USE_ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif  // USE_ESP32
#ifdef USE_ESP32_FRAMEWORK_ESP_IDF
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif  // USE_ESP32_FRAMEWORK_ESP_IDF
#ifdef USE_RP2040
#include <hardware/uart.h>
#endif  // USE_RP2040

namespace esphome {
namespace truma_inetbox {

enum class LIN_CHECKSUM { LIN_CHECKSUM_VERSION_1, LIN_CHECKSUM_VERSION_2 };

struct QUEUE_LIN_MSG {
  u_int8_t current_PID;
  u_int8_t data[8];
  u_int8_t len;
};

enum class QUEUE_LOG_MSG_TYPE {
  UNKNOWN,
  ERROR_LIN_ANSWER_CAN_WRITE_LIN_ANSWER,
  ERROR_LIN_ANSWER_TOO_LONG,
#ifdef ESPHOME_LOG_HAS_VERBOSE
  VERBOSE_LIN_ANSWER_RESPONSE,
#endif  // ESPHOME_LOG_HAS_VERBOSE

  ERROR_CHECK_FOR_LIN_FAULT_DETECTED,
  INFO_CHECK_FOR_LIN_FAULT_FIXED,

  ERROR_READ_LIN_FRAME_UNABLE_TO_ANSWER,
  ERROR_READ_LIN_FRAME_LOST_MSG,
#ifdef ESPHOME_LOG_HAS_VERY_VERBOSE
  VV_READ_LIN_FRAME_BREAK_EXPECTED,
  VV_READ_LIN_FRAME_SYNC_EXPECTED,
#endif  // ESPHOME_LOG_HAS_VERY_VERBOSE
  WARN_READ_LIN_FRAME_SID_CRC,
  WARN_READ_LIN_FRAME_LINv1_CRC,
  WARN_READ_LIN_FRAME_LINv2_CRC,
#ifdef ESPHOME_LOG_HAS_VERBOSE
  VERBOSE_READ_LIN_FRAME_MSG,
#endif  // ESPHOME_LOG_HAS_VERBOSE
};

// Log messages generated during interrupt are pushed to log queue.
struct QUEUE_LOG_MSG {
  QUEUE_LOG_MSG_TYPE type;
  u_int8_t current_PID;
  u_int8_t data[9];
  u_int8_t len;
#ifdef ESPHOME_LOG_HAS_VERBOSE
  bool current_data_valid;
  bool message_source_know;
  bool message_from_master;
#endif  // ESPHOME_LOG_HAS_VERBOSE
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
  void process_lin_msg_queue_(TickType_t xTicksToWait);
  void process_log_queue_(TickType_t xTicksToWait);

  uint8_t lin_msg_static_queue_storage[6 * sizeof(QUEUE_LIN_MSG)];
  StaticQueue_t lin_msg_static_queue_;
  QueueHandle_t lin_msg_queue_ =
      xQueueCreateStatic(/* uxQueueLength */ 6,
                         /* uxItemSize */ sizeof(QUEUE_LIN_MSG),
                         /* pucQueueStorageBuffer */ lin_msg_static_queue_storage, &lin_msg_static_queue_);

  uint8_t log_static_queue_storage[6 * sizeof(QUEUE_LOG_MSG)];
  StaticQueue_t log_static_queue_;
  QueueHandle_t log_queue_ =
      xQueueCreateStatic(/* uxQueueLength */ 6,
                         /* uxItemSize */ sizeof(QUEUE_LOG_MSG),
                         /* pucQueueStorageBuffer */ log_static_queue_storage, &log_static_queue_);

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