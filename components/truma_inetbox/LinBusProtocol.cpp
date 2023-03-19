#include "LinBusProtocol.h"
#include <array>
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.LinBusProtocol";

#define DIAGNOSTIC_FRAME_MASTER 0x3c
#define DIAGNOSTIC_FRAME_SLAVE 0x3d
#define LIN_NAD_BROADCAST 0x7F
#define LIN_SID_RESPONSE 0x40
#define LIN_SID_ASSIGN_NAD 0xB0
#define LIN_SID_ASSIGN_NAD_RESPONSE (LIN_SID_ASSIGN_NAD | LIN_SID_RESPONSE)
#define LIN_SID_READ_BY_IDENTIFIER 0xB2
#define LIN_SID_READ_BY_IDENTIFIER_RESPONSE (LIN_SID_READ_BY_IDENTIFIER | LIN_SID_RESPONSE)
#define LIN_SID_HEARTBEAT 0xB9
#define LIN_SID_HEARTBEAT_RESPONSE (LIN_SID_HEARTBEAT | LIN_SID_RESPONSE)

void LinBusProtocol::lin_reset_device(){
    // clear any messages in send queue of LinBus Protocol handler.
  while (!this->updates_to_send_.empty()) {
    this->updates_to_send_.pop();
  }
}

bool LinBusProtocol::answer_lin_order_(const u_int8_t pid) {
  // Send requested answer
  if (pid == DIAGNOSTIC_FRAME_SLAVE) {
    if (!this->updates_to_send_.empty()) {
      auto update_to_send_ = this->updates_to_send_.front();
      this->updates_to_send_.pop();
      this->write_lin_answer_(update_to_send_.data(), (u_int8_t) update_to_send_.size());
      return true;
    }
  }
  return false;
}

void LinBusProtocol::lin_message_recieved_(const u_int8_t pid, const u_int8_t *message, u_int8_t length) {
  if (pid == DIAGNOSTIC_FRAME_MASTER) {
    // The original Inet Box is answering this message. Works fine without.
    // std::array<u_int8_t, 8> message_array = {};
    // std::copy(message, message + length, message_array.begin());
    // if (message_array == this->lin_empty_response_) {
    //   std::array<u_int8_t, 8> response = this->lin_empty_response_;
    //   response[0] = 0x00;
    //   response[1] = 0x55;
    //   response[2] = 0x03;  // this->lin_node_address_;
    //   response[3] = 0x66;
    //   response[4] = 0x5B;
    //   response[5] = 0xA7;
    //   response[6] = 0x0E;
    //   response[7] = 0x49;
    //   this->prepare_update_msg_(response);
    // }

    {
      // auto node_address = message[0];
      bool my_node_address = message[0] == this->lin_node_address_;
      bool broadcast_address = message[0] == LIN_NAD_BROADCAST;
      if (!my_node_address && !broadcast_address) {
        return;
      }
    }
    u_int8_t protocol_control_information = message[1];
    if ((protocol_control_information & 0xF0) == 0x00) {
      // Single Frame mode
      {
        // End any open Multi frame mode message
        this->multi_pdu_message_expected_size_ = 0;
        this->multi_pdu_message_len_ = 0;
        this->multi_pdu_message_frame_counter_ = 0;
      }
      this->lin_msg_diag_single_(message, length);
    } else if ((protocol_control_information & 0xF0) == 0x10) {
      // First Frame of multi PDU message
      this->lin_msg_diag_first_(message, length);
    } else if ((protocol_control_information & 0xF0) == 0x20) {
      // Consecutive Frames
      if (this->lin_msg_diag_consecutive_(message, length)) {
        this->lin_msg_diag_multi_();
      }
    }

  } else if (pid == this->lin_node_address_) {
    ESP_LOGW(TAG, "Unhandled message for me.");
  }
}

bool LinBusProtocol::is_matching_identifier_(const u_int8_t *message) {
  auto lin_identifier = this->lin_identifier();
  return message[0] == lin_identifier[0] && message[1] == lin_identifier[1] && message[2] == lin_identifier[2] &&
         message[3] == lin_identifier[3];
}

void LinBusProtocol::lin_msg_diag_single_(const u_int8_t *message, u_int8_t length) {
  // auto node_address = message[0];
  bool my_node_address = message[0] == this->lin_node_address_;
  bool broadcast_address = message[0] == LIN_NAD_BROADCAST;

  u_int8_t message_length = message[1];
  u_int8_t service_identifier = message[2];
  if (message_length > 6) {
    ESP_LOGE(TAG, "LIN Protocol issue: Single frame message too long.");
    // ignore invalid message
    return;
  }

  if (service_identifier == LIN_SID_READ_BY_IDENTIFIER && message_length == 6) {
    if (this->is_matching_identifier_(&message[4])) {
      // I have observed the following identifiers:
      // broadcast_address:
      // - 0x00 - response lin_identifier[0:4] + 0x00 /* Hardware revision*/
      // my_node_address:
      // - 0x20 - displayed version
      // - 0x22 - unknown
      auto identifier = message[3];
      std::array<u_int8_t, 8> response = this->lin_empty_response_;
      response[0] = this->lin_node_address_;

      std::array<u_int8_t, 5> identifier_response = {};
      if (this->lin_read_field_by_identifier_(identifier, &identifier_response)) {
        response[1] = 6; /* bytes length - ignored by CP Plus?*/
        response[2] = LIN_SID_READ_BY_IDENTIFIER_RESPONSE;
        auto iterator = response.begin();
        std::advance(iterator, 3);
        std::copy(identifier_response.data(), identifier_response.data() + identifier_response.size(), iterator);
      } else {
        // Not supported - Negative response (see 4.2.6.1 Read by identifier)
        response[1] = 3; /* bytes length*/
        response[2] = 0x7F;
        response[3] = LIN_SID_READ_BY_IDENTIFIER;
        response[4] = 0x12;
      }
      this->prepare_update_msg_(response);
    }
  } else if (my_node_address && service_identifier == LIN_SID_HEARTBEAT && message_length >= 5) {
    // if (message[3] == 0x00 && message[4] == 0x1F && message[5] == 0x00 && message[6] == 0x00) {
    std::array<u_int8_t, 8> response = this->lin_empty_response_;
    response[0] = this->lin_node_address_;
    response[1] = 2; /* bytes length*/
    response[2] = LIN_SID_HEARTBEAT_RESPONSE;
    response[3] = 0x00;
    this->prepare_update_msg_(response);

    this->lin_heartbeat();
    //}
  } else if (broadcast_address && service_identifier == LIN_SID_ASSIGN_NAD && message_length == 6) {
    if (this->is_matching_identifier_(&message[3])) {
      ESP_LOGI(TAG, "Assigned new SID %02X", message[7]);

      // send response with old node address.
      std::array<u_int8_t, 8> response = this->lin_empty_response_;
      response[0] = this->lin_node_address_;
      response[1] = 1; /* bytes length*/
      response[2] = LIN_SID_ASSIGN_NAD_RESPONSE;

      this->prepare_update_msg_(response);
      this->lin_node_address_ = message[7];
    }
  } else {
    if (my_node_address) {
      ESP_LOGD(TAG, "SID %02X  MY  - %s - Unhandled", service_identifier, format_hex_pretty(message, length).c_str());
    } else if (broadcast_address) {
      ESP_LOGD(TAG, "SID %02X  BC  - %s - Unhandled", service_identifier, format_hex_pretty(message, length).c_str());
    }
  }
}

void LinBusProtocol::lin_msg_diag_first_(const u_int8_t *message, u_int8_t length) {
  u_int8_t protocol_control_information = message[1];
  u_int16_t message_length = (protocol_control_information & 0x0F << 8) + message[2];
  if (message_length < 7) {
    ESP_LOGE(TAG, "LIN Protocol issue: Multi frame message too short.");
    // ignore invalid message
    return;
  }
  if (message_length > sizeof(this->multi_pdu_message_)) {
    ESP_LOGE(TAG, "LIN Protocol issue: Multi frame message too long.");
    // ignore invalid message
    return;
  }
  this->multi_pdu_message_expected_size_ = message_length;
  this->multi_pdu_message_len_ = 0;
  this->multi_pdu_message_frame_counter_ = 1;

  // Copy recieved message over to `multi_pdu_message_` buffer.
  for (size_t i = 3; i < 8; i++) {
    this->multi_pdu_message_[this->multi_pdu_message_len_++] = message[i];
  }
}

bool LinBusProtocol::lin_msg_diag_consecutive_(const u_int8_t *message, u_int8_t length) {
  if (this->multi_pdu_message_len_ >= this->multi_pdu_message_expected_size_) {
    // ignore, because i don't await a consecutive frame
    return false;
  }
  u_int8_t protocol_control_information = message[1];
  u_int8_t frame_counter = protocol_control_information & 0x0F;
  if (frame_counter != this->multi_pdu_message_frame_counter_) {
    // ignore, because i don't await this consecutive frame
    return false;
  }
  this->multi_pdu_message_frame_counter_++;
  if (this->multi_pdu_message_frame_counter_ > 0x0F) {
    // Frame counter has only 4 bit and wraps around.
    this->multi_pdu_message_frame_counter_ = 0x00;
  }

  // Copy recieved message over to `multi_pdu_message_` buffer.
  for (u_int8_t i = 2; i < 8; i++) {
    if (this->multi_pdu_message_len_ < this->multi_pdu_message_expected_size_) {
      this->multi_pdu_message_[this->multi_pdu_message_len_++] = message[i];
    }
  }

  // Check if this was the last consecutive message.
  return this->multi_pdu_message_len_ == this->multi_pdu_message_expected_size_;
}

void LinBusProtocol::lin_msg_diag_multi_() {
  ESP_LOGD(TAG, "Multi package request  %s",
           format_hex_pretty(this->multi_pdu_message_, this->multi_pdu_message_len_).c_str());

  u_int8_t answer_len = 0;
  // Ask handling class what to answer to this request.
  auto answer = this->lin_multiframe_recieved(this->multi_pdu_message_, this->multi_pdu_message_len_, &answer_len);
  if (answer_len > 0) {
    ESP_LOGD(TAG, "Multi package response %s", format_hex_pretty(answer, answer_len).c_str());

    std::array<u_int8_t, 8> response = this->lin_empty_response_;
    if (answer_len <= 6) {
      // Single Frame response - first frame
      response[0] = this->lin_node_address_;
      response[1] = answer_len; /* bytes length */
      response[2] = answer[0] | LIN_SID_RESPONSE;
      for (u_int8_t i = 1; i < answer_len; i++) {
        response[i + 2] = answer[i];
      }
      this->prepare_update_msg_(response);
    } else {
      // Multi Frame response
      response[0] = this->lin_node_address_;
      response[1] = 0x10 | ((answer_len >> 8) & 0x0F);
      response[2] = answer_len & 0xFF;
      response[3] = answer[0] | LIN_SID_RESPONSE;
      for (u_int8_t i = 1; i < 5; i++) {
        response[i + 3] = answer[i];
      }
      this->prepare_update_msg_(response);

      // Multi Frame response - consecutive frame
      u_int16_t answer_position = 5;      // The first 5 bytes are sent in First frame of multi frame response.
      u_int8_t answer_frame_counter = 0;  // Each answer frame can contain 6 bytes
      while (answer_position < answer_len) {
        response = this->lin_empty_response_;
        response[0] = this->lin_node_address_;
        response[1] = ((answer_frame_counter + 1) & 0x0F) | 0x20;
        for (u_int8_t i = 0; i < 6; i++) {
          if (answer_position < answer_len) {
            response[i + 2] = answer[answer_position++];
          }
        }
        this->prepare_update_msg_(response);
        answer_frame_counter++;
      }
    }
  }
}

}  // namespace truma_inetbox
}  // namespace esphome