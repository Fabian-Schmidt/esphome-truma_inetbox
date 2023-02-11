#include "LinBusListener.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "helpers.h"

#ifdef USE_ESP32
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "soc/uart_reg.h"
#endif

#ifdef USE_ESP8266
#include "esphome/components/uart/uart_component_esp8266.h"
#endif
#ifdef USE_ESP32_FRAMEWORK_ARDUINO
#include "esphome/components/uart/truma_uart_component_esp32_arduino.h"
#include "esphome/components/uart/uart_component_esp32_arduino.h"
#endif
#ifdef USE_ESP32_FRAMEWORK_ESP_IDF
#include "esphome/components/uart/truma_uart_component_esp_idf.h"
#include "esphome/components/uart/uart_component_esp_idf.h"
#endif
#ifdef USE_RP2040
#include "esphome/components/uart/truma_uart_component_rp2040.h"
#include "esphome/components/uart/uart_component_rp2040.h"
#endif

#ifdef USE_ESP32_FRAMEWORK_ARDUINO
// For method `xTaskCreateUniversal`
#include <esp32-hal.h>
#endif

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.LinBusListener";

#define LIN_BREAK 0x00
#define LIN_SYNC 0x55
#define DIAGNOSTIC_FRAME_MASTER 0x3c
#define DIAGNOSTIC_FRAME_SLAVE 0x3d

void LinBusListener::dump_config() {
  ESP_LOGCONFIG(TAG, "TODO");

  this->check_uart_settings(9600, 2, esphome::uart::UART_CONFIG_PARITY_NONE, 8);
}

void LinBusListener::setup() {
  ESP_LOGCONFIG(TAG, "Setting up LIN BUS...");
  this->time_per_baud_ = (1000.0f * 1000.0f / this->parent_->get_baud_rate());
  this->time_per_lin_break_ = this->time_per_baud_ * this->lin_break_length * 1.1;
  this->time_per_pid_ = this->time_per_baud_ * this->frame_length_ * 1.1;
  this->time_per_first_byte_ = this->time_per_baud_ * this->frame_length_ * 3.0;
  this->time_per_byte_ = this->time_per_baud_ * this->frame_length_ * 1.1;

#ifdef USE_ESP32_FRAMEWORK_ARDUINO
  auto uartComp = static_cast<esphome::uart::truma_ESP32ArduinoUARTComponent *>(this->parent_);
  auto uart_num = uartComp->get_hw_serial_number();

  // Tweak the fifo settings so data is available as soon as the first byte is recieved.
  // If not it will wait either until fifo is filled or a certain time has passed.
  uart_intr_config_t uart_intr;
  uart_intr.intr_enable_mask =
      UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M;  // only these IRQs - no BREAK, PARITY or OVERFLOW
  // UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M | UART_FRM_ERR_INT_ENA_M |
  // UART_RXFIFO_OVF_INT_ENA_M | UART_BRK_DET_INT_ENA_M | UART_PARITY_ERR_INT_ENA_M;
  uart_intr.rxfifo_full_thresh =
      1;  // UART_FULL_THRESH_DEFAULT,  //120 default!! aghh! need receive 120 chars before we see them
  uart_intr.rx_timeout_thresh =
      10;  // UART_TOUT_THRESH_DEFAULT,  //10 works well for my short messages I need send/receive
  uart_intr.txfifo_empty_intr_thresh = 10;  // UART_EMPTY_THRESH_DEFAULT
  uart_intr_config(uart_num, &uart_intr);
#elif USE_ESP32_FRAMEWORK_ESP_IDF

  // uartSetFastReading
  auto uartComp = ((*uart::truma_IDFUARTComponent) this->parent_);

  // Tweak the fifo settings so data is available as soon as the first byte is recieved.
  // If not it will wait either until fifo is filled or a certain time has passed.
  uart_intr_config_t uart_intr;
  uart_intr.intr_enable_mask = 0;  // UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M | UART_FRM_ERR_INT_ENA_M |
                                   // UART_RXFIFO_OVF_INT_ENA_M | UART_BRK_DET_INT_ENA_M | UART_PARITY_ERR_INT_ENA_M;
  uart_intr.rxfifo_full_thresh =
      1;  // UART_FULL_THRESH_DEFAULT,  //120 default!! aghh! need receive 120 chars before we see them
  uart_intr.rx_timeout_thresh =
      1;  // UART_TOUT_THRESH_DEFAULT,  //10 works well for my short messages I need send/receive
  uart_intr.txfifo_empty_intr_thresh = 10;  // UART_EMPTY_THRESH_DEFAULT
  uart_intr_config(uartComp->get_hw_serial_number(), &uart_intr);
#else
// truma_RP2040UartComponent
#error Only ESP32 Arduino is supported.
#endif

  xTaskCreateUniversal(LinBusListener::read_task,
                       "read_task",              // name
                       4096,                     // stack size (in words)
                       this,                     // input params
                       1,                        // priority
                       &this->read_task_handle,  // handle
                       0                         // core
  );

  if (this->cs_pin_ != nullptr) {
    this->cs_pin_->digital_write(true);
  }
}

void LinBusListener::write_lin_answer_(const u_int8_t *data, size_t len) {
  if (!this->can_write_lin_answer_) {
    ESP_LOGE(TAG, "Cannot answer LIN because there is no open order from master.");
    return;
  }
  this->can_write_lin_answer_ = false;
  if (len > 8) {
    ESP_LOGE(TAG, "LIN answer cannot be longer than 8 bytes.");
    return;
  }

  int64_t wait_time = 0;
  // Was data read from FIFO and the master is awaiting an answer.
  if (this->total_wait_ > 1000) {
    // I am up to date and should not answer too quickly.
    auto current = esp_timer_get_time();
    auto wait_time_in_us = (int64_t) this->time_per_baud_ - (current - this->last_data_recieved_);
    wait_time = wait_time_in_us;
    if (wait_time_in_us > 1000 || wait_time_in_us < 0) {
      wait_time_in_us = 0;
    }
    delayMicroseconds(wait_time_in_us);
  }

  u_int8_t data_CRC = 0;
  if (this->lin_checksum_ == LIN_CHECKSUM::LIN_CHECKSUM_VERSION_1 || this->current_PID_ == DIAGNOSTIC_FRAME_SLAVE) {
    // LIN checksum V1
    data_CRC = data_checksum(data, len, 0);
  } else {
    // LIN checksum V2
    data_CRC = data_checksum(data, len, this->current_PID_with_parity_);
  }

  if (!this->observer_mode_) {
    this->write_array(data, len);
    this->write(data_CRC);
    this->flush();
  }
  ESP_LOGV(TAG, "RESPONSE %02x %s %02x T %lli", this->current_PID_, format_hex_pretty(data, len).c_str(), data_CRC,
           wait_time);
}

void LinBusListener::read_task(void *params) {
  LinBusListener *instance = (LinBusListener *) params;

  while (true) {
    // Check if Lin Bus is faulty.
    if (instance->fault_pin_ != nullptr) {
      if (!instance->fault_pin_->digital_read()) {
        if (!instance->fault_on_lin_bus_reported_) {
          instance->fault_on_lin_bus_reported_ = true;
          ESP_LOGE(TAG, "Fault on LIN BUS detected.");
        }
        // Ignore any data present in buffer
        instance->clear_uart_buffer_();
      } else if (instance->fault_on_lin_bus_reported_) {
        instance->fault_on_lin_bus_reported_ = false;
        ESP_LOGI(TAG, "Fault on LIN BUS fixed.");
      }
    }

    if (!instance->fault_on_lin_bus_reported_) {
      while (instance->available()) {
        instance->read_lin_frame_();
      }
    }

    // Check if CP Plus is inactive mode. In inactive mode it checks the bus every ~15 seconds for ~5 seconds. At the
    // start it send a Break to notify devices to wake up.
    auto time_since_last_activity = esp_timer_get_time() - instance->last_data_recieved_;
    if (time_since_last_activity > 100 * 1000 /* 100 ms*/) {
      // CP Plus is inactive.
      delay(500);  // NOLINT
    } else {
      // CP Plus is active.
      // 1'000'000 ns / 9600 baud = 104 ns/baud * (8 bit + start bit + 2 stop bit) = 1144 ns/byte * 3 (BREAK,SYNC,PID) =
      // ~3.5ms per preamble till I should answer. It is still working with 50ms. But thats the upper limit. CP Plus
      // waits 50ms when ordering for an answer. With higher polling the number of CRC errors increases and I cannot
      // answer lin orders.
      delay(10);  // NOLINT
    }
  }
}

void LinBusListener::read_lin_frame_() {
  u_int8_t buf;
  bool dataRecieved;
  u_int8_t data_length, data_CRC, data_CRC_master, data_CRC_slave;
  bool message_source_know, message_from_master;

  // Reset current state
  {
    this->current_PID_with_parity_ = 0x00;
    this->current_PID_ = 0x00;
    this->current_data_valid = true;
    this->current_data_count_ = 0;
    memset(this->current_data_, 0, sizeof(this->current_data_));
    this->total_wait_ = 0;
  }

  // First is Break expected
  if (!this->read_byte(&buf) || buf != LIN_BREAK) {
    // Update I recieved garbage
    this->last_data_recieved_ = esp_timer_get_time();
    ESP_LOGVV(TAG, "Expected BREAK not received.");
    return;
  }

  // Update I recieved a break
  this->last_data_recieved_ = esp_timer_get_time();

  if (!this->wait_for_data_available_with_timeout_(this->time_per_lin_break_)) {
    ESP_LOGV(TAG, "Timeout waiting for Sync");
    return;
  }

  // Second is Sync expected
  if (!this->read_byte(&buf) || buf != LIN_SYNC) {
    // No data present on UART
    ESP_LOGVV(TAG, "Expected SYNC not found.");
    return;
  }

  if (!this->wait_for_data_available_with_timeout_(this->time_per_pid_)) {
    ESP_LOGVV(TAG, "Timeout waiting for PID.");
    return;
  }

  this->read_byte(&(this->current_PID_with_parity_));
  this->current_PID_ = this->current_PID_with_parity_ & 0x3F;
  if (this->lin_checksum_ == LIN_CHECKSUM::LIN_CHECKSUM_VERSION_2) {
    if (this->current_PID_with_parity_ != (this->current_PID_ | (addr_parity(this->current_PID_) << 6))) {
      ESP_LOGW(TAG, "LIN CRC error");
      this->current_data_valid = false;
    }
  }

  this->can_write_lin_answer_ = true;
  // Should I response to this PID order? Ask the handling class.
  this->answer_lin_order_(this->current_PID_);
  this->can_write_lin_answer_ = false;

  dataRecieved = wait_for_data_available_with_timeout_(this->time_per_first_byte_);
  while (dataRecieved) {
    this->read_byte(&buf);
    if (this->current_data_count_ < sizeof(this->current_data_)) {
      this->current_data_[this->current_data_count_] = buf;
      this->current_data_count_++;
      dataRecieved = wait_for_data_available_with_timeout_(this->time_per_byte_);
    } else {
      // end of data reached. There cannot be more than 9 bytes in a LIN frame.
      dataRecieved = false;
    }
  }

  if (this->current_data_count_ > 1) {
    data_length = this->current_data_count_ - 1;
    data_CRC = this->current_data_[this->current_data_count_ - 1];
    message_source_know = false;
    message_from_master = true;

    if (this->lin_checksum_ == LIN_CHECKSUM::LIN_CHECKSUM_VERSION_1 ||
        (this->current_PID_ == DIAGNOSTIC_FRAME_MASTER || this->current_PID_ == DIAGNOSTIC_FRAME_SLAVE)) {
      if (data_CRC != data_checksum(this->current_data_, data_length, 0)) {
        ESP_LOGW(TAG, "LIN v1 CRC error");
        this->current_data_valid = false;
      }
      if (this->current_PID_ == DIAGNOSTIC_FRAME_MASTER) {
        message_source_know = true;
        message_from_master = true;
      } else if (this->current_PID_ == DIAGNOSTIC_FRAME_SLAVE) {
        message_source_know = true;
        message_from_master = false;
      }
    } else {
      data_CRC_master = data_checksum(this->current_data_, data_length, this->current_PID_);
      data_CRC_slave = data_checksum(this->current_data_, data_length, this->current_PID_with_parity_);
      if (data_CRC != data_CRC_master && data_CRC != data_CRC_slave) {
        ESP_LOGW(TAG, "LIN v2 CRC error");
        this->current_data_valid = false;
      }
      message_source_know = true;
      if (data_CRC == data_CRC_slave) {
        message_from_master = false;
      }
    }

    // Mark the PID of the TRUMA Combi heater as very verbose message.
    if (this->current_PID_ == 0x20 || this->current_PID_ == 0x21 || this->current_PID_ == 0x22) {
      ESP_LOGVV(TAG, "PID %02x (%02x) %s %s %s", this->current_PID_, this->current_PID_with_parity_,
                format_hex_pretty(this->current_data_, this->current_data_count_).c_str(),
                message_source_know ? (message_from_master ? " - MASTER" : " - SLAVE") : "",
                this->current_data_valid ? "" : "INVALID");
    } else {
      ESP_LOGV(TAG, "PID %02x (%02x) %s %s %S", this->current_PID_, this->current_PID_with_parity_,
               format_hex_pretty(this->current_data_, this->current_data_count_).c_str(),
               message_source_know ? (message_from_master ? " - MASTER" : " - SLAVE") : "",
               this->current_data_valid ? "" : "INVALID");
    }

    if (this->current_data_valid && message_from_master) {
      this->lin_message_recieved_(this->current_PID_, this->current_data_, data_length);
    }
  } else {
    ESP_LOGV(TAG, "PID %02x (%02x) order no answer", this->current_PID_, this->current_PID_with_parity_);
  }
}

void LinBusListener::clear_uart_buffer_() {
  u_int8_t buffer;
  while (this->available() && this->read_byte(&buffer)) {
  }
}

bool LinBusListener::wait_for_data_available_with_timeout_(u_int32_t timeout) {
  int64_t start = esp_timer_get_time();
  int64_t current = esp_timer_get_time();
  int64_t latest_end = start + timeout;
  while (current < latest_end) {
    current = esp_timer_get_time();
    if (this->available()) {
      this->total_wait_ += current - start;
      this->last_data_recieved_ = current;
      return true;
    }
    NOP();
  }
  return false;
}

}  // namespace truma_inetbox
}  // namespace esphome
