#pragma once

namespace esphome {
namespace truma_inetbox {

enum class TRUMA_BINARY_SENSOR_TYPE {
  HEATER_ROOM,
  HEATER_WATER,
  HEATER_GAS,
  HEATER_MIX_1,
  HEATER_MIX_2,
  HEATER_ELECTRICITY,

  TIMER_ACTIVE,
  TIMER_ROOM,
  TIMER_WATER,
};

static const char *enum_to_c_str(const TRUMA_BINARY_SENSOR_TYPE val) {
  switch (val) {
    case TRUMA_BINARY_SENSOR_TYPE::HEATER_ROOM:
      return "HEATER_ROOM";
      break;
    case TRUMA_BINARY_SENSOR_TYPE::HEATER_WATER:
      return "HEATER_WATER";
      break;
    case TRUMA_BINARY_SENSOR_TYPE::HEATER_GAS:
      return "HEATER_GAS";
      break;
    case TRUMA_BINARY_SENSOR_TYPE::HEATER_MIX_1:
      return "HEATER_MIX_1";
      break;
    case TRUMA_BINARY_SENSOR_TYPE::HEATER_MIX_2:
      return "HEATER_MIX_2";
      break;
    case TRUMA_BINARY_SENSOR_TYPE::HEATER_ELECTRICITY:
      return "HEATER_ELECTRICITY";
      break;

    case TRUMA_BINARY_SENSOR_TYPE::TIMER_ACTIVE:
      return "TIMER_ACTIVE";
      break;
    case TRUMA_BINARY_SENSOR_TYPE::TIMER_ROOM:
      return "TIMER_ROOM";
      break;
    case TRUMA_BINARY_SENSOR_TYPE::TIMER_WATER:
      return "TIMER_WATER";
      break;
    default:
      return "";
      break;
  }
}

}  // namespace truma_inetbox
}  // namespace esphome