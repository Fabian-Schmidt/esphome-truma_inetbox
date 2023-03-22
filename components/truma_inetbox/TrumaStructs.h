#pragma once

#include "TrumaEnums.h"

namespace esphome {
namespace truma_inetbox {

struct StatusFrameHeader {  // NOLINT(altera-struct-pack-align)
  // sid
  u_int8_t service_identifier;
  u_int8_t header[10];
  u_int8_t header_2;
  u_int8_t header_3;
  // after checksum
  u_int8_t message_length;
  u_int8_t message_type;
  u_int8_t command_counter;
  u_int8_t checksum;
} __attribute__((packed));

// Length 20 (0x14)
struct StatusFrameHeater {  // NOLINT(altera-struct-pack-align)
  TargetTemp target_temp_room;
  // Room
  HeatingMode heating_mode;
  ElectricPowerLevel el_power_level_a;
  TargetTemp target_temp_water;
  ElectricPowerLevel el_power_level_b;
  EnergyMix energy_mix_a;
  // Ignored by response
  EnergyMix energy_mix_b;
  u_int16_t current_temp_water;
  u_int16_t current_temp_room;
  OperatingStatus operating_status;
  u_int8_t error_code_low;
  u_int8_t error_code_high;
  u_int8_t heater_unknown_2;
} __attribute__((packed));

// Length 12 (0x0C)
struct StatusFrameHeaterResponse {  // NOLINT(altera-struct-pack-align)
  TargetTemp target_temp_room;
  // Room
  HeatingMode heating_mode;
  ElectricPowerLevel el_power_level_a;
  TargetTemp target_temp_water;
  ElectricPowerLevel el_power_level_b;
  EnergyMix energy_mix_a;
  // Ignored?
  EnergyMix energy_mix_b;
} __attribute__((packed));

// Length 24 (0x18)
struct StatusFrameTimer {  // NOLINT(altera-struct-pack-align)
  TargetTemp timer_target_temp_room;
  HeatingMode timer_heating_mode;
  ElectricPowerLevel timer_el_power_level_a;
  TargetTemp timer_target_temp_water;
  ElectricPowerLevel timer_el_power_level_b;
  EnergyMix timer_energy_mix_a;
  EnergyMix timer_energy_mix_b;
  // used by timer response message
  u_int8_t unused[5];
  u_int8_t timer_unknown_3;
  u_int8_t timer_unknown_4;
  TimerActive timer_active;
  u_int8_t timer_start_minutes;
  u_int8_t timer_start_hours;
  u_int8_t timer_stop_minutes;
  u_int8_t timer_stop_hours;
} __attribute__((packed));

// Length 13 (0x0D)
struct StatusFrameTimerResponse {  // NOLINT(altera-struct-pack-align)
  TargetTemp timer_target_temp_room;
  HeatingMode timer_heating_mode;
  ElectricPowerLevel timer_el_power_level_a;
  TargetTemp timer_target_temp_water;
  ElectricPowerLevel timer_el_power_level_b;
  EnergyMix timer_energy_mix_a;
  EnergyMix timer_energy_mix_b;
  // set by response message to active timer
  TimerActive timer_resp_active;
  // set by response message to active timer
  u_int8_t timer_resp_start_minutes;
  // set by response message to active timer
  u_int8_t timer_resp_start_hours;
  // set by response message to active timer
  u_int8_t timer_resp_stop_minutes;
  // set by response message to active timer
  u_int8_t timer_resp_stop_hours;
} __attribute__((packed));

// Length 2 (0x02)
struct StatusFrameResponseAck {  // NOLINT(altera-struct-pack-align)
  ResponseAckResult error_code;
  u_int8_t unknown;
} __attribute__((packed));

// Length 10 (0x0A)
struct StatusFrameClock {  // NOLINT(altera-struct-pack-align)
  u_int8_t clock_hour;
  u_int8_t clock_minute;
  u_int8_t clock_second;
  // MUST be 0x1, 0x2, 0x3..? (lower than 0x9)
  u_int8_t display_1;
  // MUST be 0x1
  u_int8_t display_2;
  u_int8_t display_3;
  ClockMode clock_mode;
  ClockSource clock_source;
  u_int8_t display_4;
  u_int8_t display_5;
} __attribute__((packed));

// Length 10 (0x0A)
struct StatusFrameConfig {  // NOLINT(altera-struct-pack-align)
  // 0x01 .. 0x0A
  u_int8_t display_brightness;
  Language language;
  // Mit „AC SET“ wird ein Offset zwischen Kühlen und Heizen eingestellt.
  // Die Einstellung ist in Schritten von 0,5 °C im Bereich von 0 °C bis +5 °C möglich.
  TargetTemp ac_offset;
  TargetTemp temp_offset;
  OperatingUnits temp_units;
  u_int8_t unknown_6;
  u_int8_t unknown_7;
  u_int8_t unknown_8;
} __attribute__((packed));

// Length 12 (0x0C)
struct StatusFrameDevice {  // NOLINT(altera-struct-pack-align)
  u_int8_t device_count;
  u_int8_t device_id;
  TRUMA_DEVICE_STATE state;
  // 0x00
  u_int8_t unknown_1;
  u_int16_t hardware_revision_major;
  u_int8_t hardware_revision_minor;
  // `software_revision[0].software_revision[1].software_revision[2]`
  // software_revision[0] -> TRUMA_DEVICE
  u_int8_t software_revision[3];
  // 0xAD, 0x0B on CPplus with Combi4 or 0x66 on CPplus with Vario Heat Comfort ohne E
  // 0x00 on Combi4, Vario Heat
  u_int8_t unknown_2;
  // 0x10, 0x12 on CPplus
  // 0x00 on Combi4, Vario Heat
  u_int8_t unknown_3;
} __attribute__((packed));

// Length 18 (0x12)
// TODO
struct StatusFrameAirconManual {  // NOLINT(altera-struct-pack-align)
  AirconMode mode;
  // 0x00
  u_int8_t unknown_02;
  AirconOperation operation;
  EnergyMix energy_mix;
  TargetTemp target_temp_aircon;
  // 0x00
  u_int8_t unknown_07;
  // 0x00
  u_int8_t unknown_08;
  // No idea why two current_temp
  TargetTemp current_temp_aircon;
  // 0x00
  u_int8_t unknown_11;
  // 0x00
  u_int8_t unknown_12;
  ElectricPowerLevel el_power_level;
  // 0x00
  u_int8_t unknown_15;
  // 0x00
  u_int8_t unknown_16;
  TargetTemp current_temp_room;
} __attribute__((packed));

struct StatusFrameAirconManualResponse {  // NOLINT(altera-struct-pack-align)
  AirconMode mode;
  // 0x00
  u_int8_t unknown_02;
  AirconOperation operation;
  EnergyMix energy_mix;
  TargetTemp target_temp_aircon;
} __attribute__((packed));

// Length 22 (0x16)
// TODO
struct StatusFrameAirconManualInit {  // NOLINT(altera-struct-pack-align)
  u_int8_t unknown_01;                // 0x00
  u_int8_t unknown_02;                // 0x00
  AirconOperation operation;
  EnergyMix energy_mix;
  u_int8_t unknown_05;  // 0x00
  u_int8_t unknown_06;  // 0x00
  u_int8_t unknown_07;  // 0x00
  u_int8_t unknown_08;  // 0x00
  u_int8_t unknown_09;  // 0x00
  u_int8_t unknown_10;  // 0x00
  u_int8_t unknown_11;  // 0x00
  u_int8_t unknown_12;  // 0x00
  u_int8_t unknown_13;  // 0x00
  u_int8_t unknown_14;  // 0x00
  u_int8_t unknown_15;  // 0x00
  u_int8_t unknown_16;  // 0x00
  u_int8_t unknown_17;  // 0x00
  u_int8_t unknown_18;  // 0x00
  u_int8_t unknown_19;  // 0x00
  u_int8_t unknown_20;  // 0x00
  u_int8_t unknown_21;  // 0x00
  u_int8_t unknown_22;  // 0x00
} __attribute__((packed));

// Length 18 (0x12)
// TODO
struct StatusFrameAirconAuto {  // NOLINT(altera-struct-pack-align)
  EnergyMix energy_mix_a;
  u_int8_t unknown_02;  // 0x00
  EnergyMix energy_mix_b;
  u_int8_t unknown_04;  // 0x00
  u_int8_t unknown_05;  // 0x00
  u_int8_t unknown_06;  // 0x00
  TargetTemp target_temp_aircon_auto;
  ElectricPowerLevel el_power_level_a;
  u_int8_t unknown_11;  // 0x00
  u_int8_t unknown_12;  // 0x00
  ElectricPowerLevel el_power_level_b;
  TargetTemp current_temp;
  TargetTemp target_temp;
} __attribute__((packed));

// Length 20 (0x14)
// TODO
struct StatusFrameAirconAutoInit {  // NOLINT(altera-struct-pack-align)
  EnergyMix energy_mix_a;
  u_int8_t unknown_02;  // 0x00
  EnergyMix energy_mix_b;
  u_int8_t unknown_04;  // 0x00
  u_int8_t unknown_05;  // 0x00
  u_int8_t unknown_06;  // 0x00
  u_int8_t unknown_07;  // 0x00
  u_int8_t unknown_08;  // 0x00
  u_int8_t unknown_09;  // 0x00
  u_int8_t unknown_10;  // 0x00
  u_int8_t unknown_11;  // 0x00
  u_int8_t unknown_12;  // 0x00
  u_int8_t unknown_13;  // 0x00
  u_int8_t unknown_14;  // 0x00
  u_int8_t unknown_15;  // 0x00
  u_int8_t unknown_16;  // 0x00
  u_int8_t unknown_17;  // 0x00
  u_int8_t unknown_18;  // 0x00
  u_int8_t unknown_19;  // 0x00
  u_int8_t unknown_20;  // 0x00
} __attribute__((packed));

}  // namespace truma_inetbox
}  // namespace esphome