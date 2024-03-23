#pragma once

#include "TrumaEnums.h"
#include "TrumaEnumsAlde.h"

namespace esphome {
namespace truma_inetbox {

// Length 28 (0x1C) - MSG x51
struct StatusFameAldeStatus {  // NOLINT(altera-struct-pack-align)
  u_int8_t unknown_00;         // x01
  u_int8_t unknown_01;         // x00
  TargetTemp target_temp_room;
  u_int16_t unknown_04;  // xFFFE
  u_int8_t unknown_06;   // x00
  ElectricPowerLevelAlde el_mode;
  GasModeAlde gas_mode;
  u_int8_t unknown_09;  // x00, x01 - ?neutral, day or night mod
  u_int8_t unknown_10;  // x00
  u_int8_t unknown_11;  // x64
  u_int8_t unknown_12;  // xFF
  u_int8_t unknown_13;  // xFF
  u_int8_t unknown_14;  // xFF
  u_int8_t message_counter;
  TargetTemp current_temp_inside;
  u_int16_t unknown_18;  // xFFFE
  TargetTemp current_temp_outside;
  u_int8_t unknown_22;   // x00
  u_int8_t unknown_23;   // x1E
  u_int16_t unknown_24;  // xFFFF
  u_int16_t unknown_26;  // xFFFF
} __attribute__((packed));

// Length 16 (0x10) - MSG x50
struct StatusFameAldeStatusResponse {  // NOLINT(altera-struct-pack-align)
  u_int8_t unknown_00;                 // x01
  u_int8_t unknown_01;                 // x00
  TargetTemp target_temp_room;
  u_int16_t unknown_04;  // xFFFF
  u_int8_t unknown_06;   // x00
  ElectricPowerLevelAlde el_mode;
  GasModeAlde gas_mode;
  u_int8_t unknown_09;  // x00, x01 - ?neutral, day or night mod
  u_int8_t unknown_10;  // x00
  u_int8_t unknown_11;  // x64
  u_int8_t unknown_12;  // xFF
  u_int8_t unknown_13;  // xFF
  u_int8_t unknown_14;  // xFF
  u_int8_t message_counter;
} __attribute__((packed));

// Length 36 (0x24) - MSG x53
struct StatusFameAldeAddon {  // NOLINT(altera-struct-pack-align)
  u_int8_t unknown_00[15];
  TempSensorUsageAlde temp_sensor;
  u_int8_t unknown_16[3];  // xFF
  u_int8_t message_counter;
  u_int8_t unknown_20[17];
} __attribute__((packed));

// Length 12 (0x0C) - MSG x55
struct StatusFameAldeHeaterNight {  // NOLINT(altera-struct-pack-align)
  TargetTemp target_temp_room;
  u_int16_t mode_start_time;  // 0x03DE - 990/60 -> 16.5 -> 16:30
  u_int16_t mode_end_time;    // 0x01C2 - 450/60 -> 07.5 -> 07:30
  WeekDayAlde weekday_start;
  WeekDayAlde weekday_end;
  u_int8_t bits;         // 17 sensor bett, 7 sensor paneel / auto with night mode, 6 Night mode off, 3 hot water, 1
  u_int16_t unknown_09;  // x03, x02, x01
  u_int8_t unknown_10;   // 0xFF
  u_int8_t message_counter;
} __attribute__((packed));

// Length 12 (0x0C) - MSG x57
struct StatusFameAldeHeaterDay {  // NOLINT(altera-struct-pack-align)
  TargetTemp target_temp_room;
  u_int16_t mode_start_time;  // 0x03DE - 990/60 -> 16.5 -> 16:30
  u_int16_t mode_end_time;    // 0x04CE - 1230/60 -> 20.5 -> 20:30
  WeekDayAlde weekday_start;
  WeekDayAlde weekday_end;
  u_int8_t bits;
  u_int16_t unknown_09;  // 0x00
  u_int8_t unknown_10;   // 0xFF
  u_int8_t message_counter;
} __attribute__((packed));

}  // namespace truma_inetbox
}  // namespace esphome