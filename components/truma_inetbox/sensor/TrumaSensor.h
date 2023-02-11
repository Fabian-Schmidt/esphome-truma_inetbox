#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/truma_inetbox/TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {
enum class TRUMA_SENSOR_TYPE {
  CURRENT_ROOM_TEMPERATURE,
  CURRENT_WATER_TEMPERATURE,
  TARGET_ROOM_TEMPERATURE,
  TARGET_WATER_TEMPERATURE,
  HEATING_MODE,
  ELECTRIC_POWER_LEVEL,
  ENERGY_MIX,
  OPERATING_STATUS,
};

class TrumaSensor : public Component, public sensor::Sensor, public Parented<TrumaiNetBoxApp> {
 public:
  void setup() override;
  void dump_config() override;

  void set_type(TRUMA_SENSOR_TYPE val) { this->type_ = val; }

 protected:
  TRUMA_SENSOR_TYPE type_;

 private:
};
}  // namespace truma_inetbox
}  // namespace esphome