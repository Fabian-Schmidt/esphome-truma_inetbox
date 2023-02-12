#include "TrumaStatusFrame.h"
#include "esphome/core/helpers.h"
#include "TrumaiNetBoxApp.h"
#include "helpers.h"

namespace esphome {
namespace truma_inetbox {
void status_frame_create_empty(StatusFrame *response, u_int8_t message_type, u_int8_t message_length,
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

void status_frame_calculate_checksum(StatusFrame *response) {
  response->inner.genericHeader.checksum = 0x0;
  response->inner.genericHeader.checksum = data_checksum(&response->raw[10], sizeof(StatusFrame) - 10, 0);
}

void status_frame_create_init(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter) {
  status_frame_create_empty(response, STATUS_FRAME_RESPONSE_INIT_REQUEST, 0, command_counter);

  // Init frame is empty.

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + 0;
}

void status_frame_create_update_clock(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter,
                                      u_int8_t hour, u_int8_t minute, u_int8_t second, ClockMode clockMode) {
  status_frame_create_empty(response, STATUS_FRAME_CLOCK_RESPONSE, sizeof(StatusFrameClock), command_counter);

  response->inner.clock.clock_hour = hour;
  response->inner.clock.clock_minute = minute;
  response->inner.clock.clock_second = second;
  response->inner.clock.display_1 = 0x1;
  response->inner.clock.display_2 = 0x1;
  response->inner.clock.clock_mode = clockMode;

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameClock);
}

void status_frame_create_update_timer(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter,
                                      TimerActive active, u_int8_t start_hour, u_int8_t start_minute,
                                      u_int8_t stop_hour, u_int8_t stop_minute, TargetTemp room, TargetTemp water,
                                      HeatingMode mode, EnergyMix energy, ElectricPowerLevel elPower) {
  status_frame_create_empty(response, STATUS_FRAME_TIMER_RESPONSE, sizeof(StatusFrameTimerResponse), command_counter);

  response->inner.timerResponse.timer_target_temp_room = room;
  response->inner.timerResponse.timer_heating_mode = mode;
  response->inner.timerResponse.timer_target_temp_water = water;
  response->inner.timerResponse.timer_energy_mix_a = energy;
  response->inner.timerResponse.timer_energy_mix_b = energy;
  response->inner.timerResponse.timer_el_power_level_a = elPower;
  response->inner.timerResponse.timer_el_power_level_b = elPower;
  response->inner.timerResponse.timer_resp_active = active;
  response->inner.timerResponse.timer_resp_start_hours = start_hour;
  response->inner.timerResponse.timer_resp_start_minutes = start_minute;
  response->inner.timerResponse.timer_resp_stop_hours = stop_hour;
  response->inner.timerResponse.timer_resp_stop_minutes = stop_minute;

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameTimerResponse);
}

void status_frame_create_update_heater(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter,
                                       TargetTemp room, TargetTemp water, HeatingMode mode, EnergyMix energy,
                                       ElectricPowerLevel elPower) {
  status_frame_create_empty(response, STATUS_FRAME_HEATER_RESPONSE, sizeof(StatusFrameHeaterResponse), command_counter);

  response->inner.heaterResponse.target_temp_room = room;
  response->inner.heaterResponse.heating_mode = mode;
  response->inner.heaterResponse.target_temp_water = water;
  response->inner.heaterResponse.energy_mix_a = energy;
  response->inner.heaterResponse.energy_mix_b = energy;
  response->inner.heaterResponse.el_power_level_a = elPower;
  response->inner.heaterResponse.el_power_level_b = elPower;

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameHeaterResponse);
}

}  // namespace truma_inetbox
}  // namespace esphome