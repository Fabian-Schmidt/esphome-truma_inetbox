#include "LinBusListener.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "helpers.h"

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

  // call device specific function
  this->setup_framework();

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
  ESP_LOGV(TAG, "RESPONSE %02x %s %02x", this->current_PID_, format_hex_pretty(data, len).c_str(), data_CRC);
}

void LinBusListener::read_lin_frame_() {
  u_int8_t buf;
  switch (this->current_state_) {
    case READ_STATE_BREAK:
      // Check if there was an unanswered message before break.
      if (this->current_PID_with_parity_ != 0x00 && this->current_PID_ != 0x00 && this->current_data_valid &&
          this->current_data_count_ == 0) {
        ESP_LOGV(TAG, "PID %02x (%02x) order no answer", this->current_PID_, this->current_PID_with_parity_);
      }

      // Reset current state
      {
        this->current_PID_with_parity_ = 0x00;
        this->current_PID_ = 0x00;
        this->current_data_valid = true;
        this->current_data_count_ = 0;
        memset(this->current_data_, 0, sizeof(this->current_data_));
      }

      // First is Break expected
      if (!this->read_byte(&buf) || buf != LIN_BREAK) {
        ESP_LOGVV(TAG, "Expected BREAK not received.");
      } else {
        this->current_state_ = READ_STATE_SYNC;
      }
      break;
    case READ_STATE_SYNC:
      // Second is Sync expected
      if (!this->read_byte(&buf) || buf != LIN_SYNC) {
        ESP_LOGVV(TAG, "Expected SYNC not found.");
        this->current_state_ = READ_STATE_BREAK;
      } else {
        this->current_state_ = READ_STATE_SID;
      }
      break;
    case READ_STATE_SID:
      this->read_byte(&(this->current_PID_with_parity_));
      this->current_PID_ = this->current_PID_with_parity_ & 0x3F;
      if (this->lin_checksum_ == LIN_CHECKSUM::LIN_CHECKSUM_VERSION_2) {
        if (this->current_PID_with_parity_ != (this->current_PID_ | (addr_parity(this->current_PID_) << 6))) {
          ESP_LOGW(TAG, "LIN CRC error on SID.");
          this->current_data_valid = false;
        }
      }

      if (this->current_data_valid) {
        this->can_write_lin_answer_ = true;
        // Should I response to this PID order? Ask the handling class.
        this->answer_lin_order_(this->current_PID_);
        this->can_write_lin_answer_ = false;
      }

      // Even on error read data.
      this->current_state_ = READ_STATE_DATA;
      break;
    case READ_STATE_DATA:
      this->read_byte(&buf);
      this->current_data_[this->current_data_count_] = buf;
      this->current_data_count_++;

      if (this->current_data_count_ >= sizeof(this->current_data_)) {
        // End of data reached. There cannot be more than 9 bytes in a LIN frame.
        this->current_state_ = READ_STATE_ACT;
      }
      break;
  }

  if (this->current_state_ == READ_STATE_ACT && this->current_data_count_ > 1) {
    u_int8_t data_length = this->current_data_count_ - 1;
    u_int8_t data_CRC = this->current_data_[this->current_data_count_ - 1];
    bool message_source_know = false;
    bool message_from_master = true;

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
      u_int8_t data_CRC_master = data_checksum(this->current_data_, data_length, this->current_PID_);
      u_int8_t data_CRC_slave = data_checksum(this->current_data_, data_length, this->current_PID_with_parity_);
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
    this->current_state_ = READ_STATE_BREAK;
  }
}

void LinBusListener::clear_uart_buffer_() {
  u_int8_t buffer;
  while (this->available() && this->read_byte(&buffer)) {
  }
}

}  // namespace truma_inetbox
}  // namespace esphome
