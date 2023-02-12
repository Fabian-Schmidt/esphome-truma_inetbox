#pragma once

#include "TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

void status_frame_create_empty(StatusFrame *response, u_int8_t message_type, u_int8_t message_length,
                               u_int8_t command_counter);

void status_frame_calculate_checksum(StatusFrame *response);

void status_frame_create_init(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter);

void status_frame_create_update_clock(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter,
                                      u_int8_t hour, u_int8_t minute, u_int8_t second, ClockMode clockMode);

void status_frame_create_update_timer(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter,
                                      TimerActive active, u_int8_t start_hour, u_int8_t start_minute,
                                      u_int8_t stop_hour, u_int8_t stop_minute, TargetTemp room, TargetTemp water,
                                      HeatingMode mode, EnergyMix energy, ElectricPowerLevel elPower);

void status_frame_create_update_heater(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter,
                                       TargetTemp room, TargetTemp water, HeatingMode mode, EnergyMix energy,
                                       ElectricPowerLevel elPower);

}  // namespace truma_inetbox
}  // namespace esphome