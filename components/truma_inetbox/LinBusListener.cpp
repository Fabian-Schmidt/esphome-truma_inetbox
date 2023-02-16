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
  this->time_per_lin_break_ = this->time_per_baud_ * this->lin_break_length * 1.1f;
  this->time_per_pid_ = this->time_per_baud_ * this->frame_length_ * 1.1f;
  this->time_per_first_byte_ = this->time_per_baud_ * this->frame_length_ * 5.0f;
  this->time_per_byte_ = this->time_per_baud_ * this->frame_length_ * 1.1f;

  if (this->cs_pin_ != nullptr) {
    this->cs_pin_->setup();
  }

  if (this->fault_pin_ != nullptr) {
    this->fault_pin_->setup();
  }

  // call device specific function
  this->setup_framework();

  if (this->cs_pin_ != nullptr) {
    // Enable LIN driver if not in oberserver mode.
    this->cs_pin_->digital_write(!this->observer_mode_);
  }
}

void LinBusListener::update() { this->check_for_lin_fault_(); }

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

  // I am answering too quick ~50-60us after stop bit. Normal communication has a ~100us pause (second stop bits).
  // The heater is answering after ~500-600us.
  // If there is any issue I might have to add a delay here.
  // Check when last byte was read from buffer and wait at least one baud time.
  // It is working when I answer quicker.

  if (!this->observer_mode_) {
    this->current_PID_order_answered_ = true;
    this->write_array(data, len);
    this->write(data_CRC);

    ESP_LOGV(TAG, "RESPONSE %02X %s %02X", this->current_PID_, format_hex_pretty(data, len).c_str(), data_CRC);
  } else {
    ESP_LOGV(TAG, "RESPONSE %02X %s %02X - NOT SEND (OBSERVER MODE)", this->current_PID_,
             format_hex_pretty(data, len).c_str(), data_CRC);
  }
}

bool LinBusListener::check_for_lin_fault_() {
  // Check if Lin Bus is faulty.
  if (this->fault_pin_ != nullptr) {
    // Fault pin is inverted (HIGH = no fault)
    if (!this->fault_pin_->digital_read()) {
      if (this->fault_on_lin_bus_reported_ < 0xFF) {
        this->fault_on_lin_bus_reported_++;
      } else {
        this->fault_on_lin_bus_reported_ = 0x0F;
      }
      if (this->fault_on_lin_bus_reported_ % 3 == 0) {
        ESP_LOGE(TAG, "Fault on LIN BUS detected.");
      }
    } else if (this->get_lin_bus_fault()) {
      this->fault_on_lin_bus_reported_ = 0;
      ESP_LOGI(TAG, "Fault on LIN BUS fixed.");
    } else {
      this->fault_on_lin_bus_reported_ = 0;
    }
  }

  if (this->get_lin_bus_fault()) {
    this->current_state_reset_();
    // Ignore any data present in buffer
    this->clear_uart_buffer_();
    return true;
  } else {
    return false;
  }
}

void LinBusListener::onReceive_() {
  if (!this->check_for_lin_fault_()) {
    while (this->available()) {
      this->read_lin_frame_();
      this->last_data_recieved_ = micros();
    }
  }
}

void LinBusListener::read_lin_frame_() {
  u_int8_t buf;
  switch (this->current_state_) {
    case READ_STATE_BREAK:
      // Check if there was an unanswered message before break.
      if (this->current_PID_with_parity_ != 0x00 && this->current_PID_ != 0x00 && this->current_data_valid) {
        if (this->current_PID_order_answered_ && this->current_data_count_ < 8) {
          // Expectation is that I can see an echo of my data from the lin driver chip.
          ESP_LOGE(TAG, "PID %02X (%02X) order - unable to send response", this->current_PID_, this->current_PID_with_parity_);
        } else if (this->current_data_count_ == 0) {
          ESP_LOGV(TAG, "PID %02X (%02X) order no answer", this->current_PID_, this->current_PID_with_parity_);
        } else if (this->current_data_count_ < 8) {
          ESP_LOGW(TAG, "PID %02X (%02X) %s partial data received", this->current_PID_, this->current_PID_with_parity_,
                   format_hex_pretty(this->current_data_, this->current_data_count_).c_str());
        }
      }

      // Reset current state
      this->current_state_reset_();

      // First is Break expected
      if (!this->read_byte(&buf) || buf != LIN_BREAK) {
        ESP_LOGVV(TAG, "0x%02X Expected BREAK not received.", buf);
      } else {
        // ESP_LOGVV(TAG, "%02X BREAK received.", buf);
        this->current_state_ = READ_STATE_SYNC;
      }
      break;
    case READ_STATE_SYNC:
      // Second is Sync expected
      if (!this->read_byte(&buf) || buf != LIN_SYNC) {
        ESP_LOGVV(TAG, "0x%02X Expected SYNC not found.", buf);
        this->current_state_ = buf == LIN_BREAK ? READ_STATE_SYNC : READ_STATE_BREAK;
      } else {
        // ESP_LOGVV(TAG, "%02X SYNC found.", buf);
        this->current_state_ = READ_STATE_SID;
      }
      break;
    case READ_STATE_SID:
      this->read_byte(&(this->current_PID_with_parity_));
      this->current_PID_ = this->current_PID_with_parity_ & 0x3F;
      if (this->lin_checksum_ == LIN_CHECKSUM::LIN_CHECKSUM_VERSION_2) {
        if (this->current_PID_with_parity_ != (this->current_PID_ | (addr_parity(this->current_PID_) << 6))) {
          ESP_LOGW(TAG, "0x%02X LIN CRC error on SID.", this->current_PID_with_parity_);
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
    case READ_STATE_DATA: {
      auto current = micros();
      if (current > (this->last_data_recieved_ + this->time_per_first_byte_)) {
        // timeout occured.
        this->current_state_ = READ_STATE_BREAK;
        return;
      }
    }
      this->read_byte(&buf);
      this->current_data_[this->current_data_count_] = buf;
      this->current_data_count_++;

      if (this->current_data_count_ >= sizeof(this->current_data_)) {
        // End of data reached. There cannot be more than 9 bytes in a LIN frame.
        this->current_state_ = READ_STATE_ACT;
      }
      break;
    default:
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
    if (this->current_PID_ == 0x20 || this->current_PID_ == 0x21 || this->current_PID_ == 0x22 ||
        ((this->current_PID_ == DIAGNOSTIC_FRAME_MASTER || this->current_PID_ == DIAGNOSTIC_FRAME_SLAVE) &&
         this->current_data_[0] == 0x01 /* ID of heater */)) {
      ESP_LOGVV(TAG, "PID %02X (%02X) %s %s %s", this->current_PID_, this->current_PID_with_parity_,
                format_hex_pretty(this->current_data_, this->current_data_count_).c_str(),
                message_source_know ? (message_from_master ? " - MASTER" : " - SLAVE") : "",
                this->current_data_valid ? "" : "INVALID");
    } else {
      ESP_LOGV(TAG, "PID %02X (%02X) %s %s %S", this->current_PID_, this->current_PID_with_parity_,
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
