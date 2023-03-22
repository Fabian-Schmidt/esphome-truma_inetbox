#pragma once

#include "TrumaStructs.h"
#include "helpers.h"

namespace esphome {
namespace truma_inetbox {

inline void status_frame_create_empty(StatusFrame *response, u_int8_t message_type, u_int8_t message_length,
                                      u_int8_t command_counter) {
  response->inner.genericHeader.service_identifier = LIN_SID_READ_STATE_BUFFER | LIN_SID_RESPONSE;
  // Copy header over for this message.
  for (size_t i = 1; i < truma_message_header.size(); i++) {
    response->raw[i] = truma_message_header[i];
  }
  response->inner.genericHeader.header_2 = 'T';
  response->inner.genericHeader.header_3 = 0x01;
  response->inner.genericHeader.message_type = message_type;
  response->inner.genericHeader.message_length = message_length;
  response->inner.genericHeader.command_counter = command_counter;
}

inline void status_frame_calculate_checksum(StatusFrame *response) {
  response->inner.genericHeader.checksum = 0x0;
  response->inner.genericHeader.checksum = data_checksum(&response->raw[10], sizeof(StatusFrame) - 10, 0);
}

inline void status_frame_create_init(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter) {
  status_frame_create_empty(response, STATUS_FRAME_RESPONSE_INIT_REQUEST, 0, command_counter);

  // Init frame is empty.

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + 0;
}

}  // namespace truma_inetbox
}  // namespace esphome