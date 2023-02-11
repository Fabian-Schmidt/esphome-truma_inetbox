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

}
}