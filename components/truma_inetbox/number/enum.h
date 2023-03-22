#pragma once

namespace esphome {
namespace truma_inetbox {

enum class TRUMA_NUMBER_TYPE {
  UNKNOWN,

  TARGET_ROOM_TEMPERATURE,
  TARGET_WATER_TEMPERATURE,
  ELECTRIC_POWER_LEVEL,
  
  AIRCON_MANUAL_TEMPERATURE,
};

static const char *enum_to_c_str(const TRUMA_NUMBER_TYPE val) {
  switch (val) {
    case TRUMA_NUMBER_TYPE::TARGET_ROOM_TEMPERATURE:
      return "TARGET_ROOM_TEMPERATURE";
      break;
    case TRUMA_NUMBER_TYPE::TARGET_WATER_TEMPERATURE:
      return "TARGET_WATER_TEMPERATURE";
      break;
    case TRUMA_NUMBER_TYPE::ELECTRIC_POWER_LEVEL:
      return "ELECTRIC_POWER_LEVEL";
      break;

    case TRUMA_NUMBER_TYPE::AIRCON_MANUAL_TEMPERATURE:
      return "AIRCON_MANUAL_TEMPERATURE";
      break;

    default:
      return "";
      break;
  }
}

}  // namespace truma_inetbox
}  // namespace esphome