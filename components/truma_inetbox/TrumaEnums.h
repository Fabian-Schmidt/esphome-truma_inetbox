#pragma once

#include "esphome/core/helpers.h"

namespace esphome {
namespace truma_inetbox {

enum class HeatingMode : u_int16_t {
  HEATING_MODE_OFF = 0x0,
  // COMBI
  HEATING_MODE_ECO = 0x1,
  // Vario Heat
  HEATING_MODE_VARIO_HEAT_NIGHT = 0x2,
  // Vario Heat
  HEATING_MODE_VARIO_HEAT_AUTO = 0x3,
  // COMBI
  HEATING_MODE_HIGH = 0xA,
  // COMBI, Vario Heat
  HEATING_MODE_BOOST = 0xB,

  // Feedback Invalid message only with following `heating_mode`. Others are ignored no feedback.
  // 00FF
  // 01FE
  // 02FD
  // ....
  // FD02
  // FE01
  // FF00
};

enum class ElectricPowerLevel : u_int16_t {
  ELECTRIC_POWER_LEVEL_0 = 0,
  ELECTRIC_POWER_LEVEL_900 = 900,
  ELECTRIC_POWER_LEVEL_1800 = 1800,
};

enum class TargetTemp : u_int16_t {
  TARGET_TEMP_OFF = 0x0,

  // 40C
  TARGET_TEMP_WATER_ECO = (40 + 273) * 10,
  // 60C
  TARGET_TEMP_WATER_HIGH = (60 + 273) * 10,
  // 200C
  TARGET_TEMP_WATER_BOOST = (200 + 273) * 10,

  TARGET_TEMP_05C = (5 + 273) * 10,
  TARGET_TEMP_06C = (6 + 273) * 10,
  TARGET_TEMP_07C = (7 + 273) * 10,
  TARGET_TEMP_08C = (8 + 273) * 10,
  TARGET_TEMP_09C = (9 + 273) * 10,
  TARGET_TEMP_10C = (10 + 273) * 10,
  TARGET_TEMP_11C = (11 + 273) * 10,
  TARGET_TEMP_12C = (12 + 273) * 10,
  TARGET_TEMP_13C = (13 + 273) * 10,
  TARGET_TEMP_14C = (14 + 273) * 10,
  TARGET_TEMP_15C = (15 + 273) * 10,
  TARGET_TEMP_16C = (16 + 273) * 10,
  TARGET_TEMP_17C = (17 + 273) * 10,
  TARGET_TEMP_18C = (18 + 273) * 10,
  TARGET_TEMP_19C = (19 + 273) * 10,
  TARGET_TEMP_20C = (20 + 273) * 10,
  TARGET_TEMP_21C = (21 + 273) * 10,
  TARGET_TEMP_22C = (22 + 273) * 10,
  TARGET_TEMP_23C = (23 + 273) * 10,
  TARGET_TEMP_24C = (24 + 273) * 10,
  TARGET_TEMP_25C = (25 + 273) * 10,
  TARGET_TEMP_26C = (26 + 273) * 10,
  TARGET_TEMP_27C = (27 + 273) * 10,
  TARGET_TEMP_28C = (28 + 273) * 10,
  TARGET_TEMP_29C = (29 + 273) * 10,
  TARGET_TEMP_30C = (30 + 273) * 10,
  TARGET_TEMP_31C = (31 + 273) * 10,

  TARGET_TEMP_ROOM_MIN = (5 + 273) * 10,
  TARGET_TEMP_ROOM_MAX = (30 + 273) * 10,

  TARGET_TEMP_AIRCON_MIN = (16 + 273) * 10,
  TARGET_TEMP_AIRCON_MAX = (31 + 273) * 10,

  TARGET_TEMP_AIRCON_AUTO_MIN = (18 + 273) * 10,
  TARGET_TEMP_AIRCON_AUTO_MAX = (25 + 273) * 10,
};

enum class EnergyMix : u_int8_t {
  ENERGY_MIX_NONE = 0b00,
  ENERGY_MIX_GAS = 0b01,
  ENERGY_MIX_DIESEL = 0b01,
  ENERGY_MIX_ELECTRICITY = 0b10,
  ENERGY_MIX_MIX = 0b11,
};

enum class OperatingStatus : u_int8_t {
  OPERATING_STATUS_UNSET = 0x0,
  OPERATING_STATUS_OFF = 0x0,
  OPERATING_STATUS_WARNING = 0x1,
  OPERATING_STATUS_START_OR_COOL_DOWN = 0x4,
  // ? Gas Heating mode ?
  OPERATING_STATUS_ON_5 = 0x5,
  OPERATING_STATUS_ON_6 = 0x6,
  OPERATING_STATUS_ON_7 = 0x7,
  OPERATING_STATUS_ON_8 = 0x8,
  OPERATING_STATUS_ON_9 = 0x9,
};

enum class OperatingUnits : u_int8_t {
  OPERATING_UNITS_CELSIUS = 0x0,
  OPERATING_UNITS_FAHRENHEIT = 0x1,
};

enum class Language : u_int8_t {
  LANGUAGE_GERMAN = 0x0,
  LANGUAGE_ENGLISH = 0x1,
  LANGUAGE_FRENCH = 0x2,
  LANGUAGE_ITALY = 0x3,
};

enum class ResponseAckResult : u_int8_t {
  RESPONSE_ACK_RESULT_OKAY = 0x0,
  RESPONSE_ACK_RESULT_ERROR_INVALID_MSG = 0x2,
  // The response status frame `message_type` is unknown.
  RESPONSE_ACK_RESULT_ERROR_INVALID_ID = 0x3,
};

enum class ClockMode : u_int8_t {
  CLOCK_MODE_24H = 0x0,
  CLOCK_MODE_12H = 0x1,
};

enum class TimerActive : u_int8_t {
  TIMER_ACTIVE_ON = 0x1,
  TIMER_ACTIVE_OFF = 0x0,
};

enum class ClockSource : u_int8_t {
  // Set by user
  CLOCK_SOURCE_MANUAL = 0x1,
  // Set by message
  CLOCK_SOURCE_PROG = 0x2,
};

enum class TRUMA_COMPANY : u_int8_t {
  UNKNOWN = 0x00,
  TRUMA = 0x1E,
  ALDE = 0x1A,
};

enum class TRUMA_DEVICE : u_int8_t {
  UNKNOWN = 0x00,

  // Saphir Compact AC
  AIRCON_DEVICE = 0x01,

  // CP Plus for Combi
  CPPLUS_COMBI = 0x04,
  // CP Plus for Vario Heat
  CPPLUS_VARIO = 0x05,

  // Combi 4
  HEATER_COMBI4 = 0x02,
  // Vario Heat Comfort (non E)
  HEATER_VARIO = 0x03,
  // Old Truma CP6 (MY 2015)
  HEATER_CP6 = 0x05,
  // Combi 6 D
  HEATER_COMBI6D = 0x06,
};

enum class TRUMA_DEVICE_STATE : u_int8_t {
  OFFLINE = 0x00,
  ONLINE = 0x01,
};

enum class AirconMode : u_int8_t {
  // Auto - 18 to 25
  OFF = 0x00,
  AC_VENTILATION = 0x04,
  AC_COOLING = 0x05,
};

enum class AirconOperation : u_int8_t {
  AC_ONLY = 0x71,
  // Heater and Aircon
  AUTO = 0x72,
};

}  // namespace truma_inetbox
}  // namespace esphome