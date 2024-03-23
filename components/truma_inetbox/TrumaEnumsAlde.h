#pragma once

#include "esphome/core/helpers.h"

namespace esphome {
namespace truma_inetbox {

enum class ElectricPowerLevelAlde : u_int8_t {
  ELECTRIC_POWER_LEVEL_ALDE_0 = 0,
  ELECTRIC_POWER_LEVEL_ALDE_1000 = 10,
  ELECTRIC_POWER_LEVEL_ALDE_2000 = 20,
  ELECTRIC_POWER_LEVEL_ALDE_300 = 30,
};

enum class GasModeAlde : u_int8_t {
  GAS_MODE_ALDE_OFF = 0x00,
  GAS_MODE_ALDE_ON = 0x37,
};

enum class TempSensorUsageAlde : u_int8_t {
  TEMP_SENSOR_USAGE_ALDE_AUTO = 0x00,
  TEMP_SENSOR_USAGE_ALDE_BED = 0x02,
  TEMP_SENSOR_USAGE_ALDE_PANEL = 0x03,
};

enum class WeekDayAlde : u_int8_t {
  WEEK_DAY_ALDE_MONDAY = 0x00,
  WEEK_DAY_ALDE_TUESDAY = 0x01,
  WEEK_DAY_ALDE_WEDNESDAY = 0x02,
  WEEK_DAY_ALDE_THURSDAY = 0x03,
  WEEK_DAY_ALDE_FRIDAY = 0x04,
  WEEK_DAY_ALDE_SATURDAY = 0x05,
  WEEK_DAY_ALDE_SUNDAY = 0x06,
  WEEK_DAY_ALDE_EVERY_DAY = 0x07,
};

}  // namespace truma_inetbox
}  // namespace esphome