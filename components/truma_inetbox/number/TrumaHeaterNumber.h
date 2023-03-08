#pragma once

#include "esphome/components/number/number.h"
#include "esphome/components/truma_inetbox/TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

enum class TRUMA_NUMBER_TYPE {
  TARGET_ROOM_TEMPERATURE,
  TARGET_WATER_TEMPERATURE,
  ELECTRIC_POWER_LEVEL,
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
    default:
      return "";
      break;
  }
}

class TrumaHeaterNumber : public Component, public number::Number, public Parented<TrumaiNetBoxApp> {
 public:
  void setup() override;
  void dump_config() override;

  void set_type(TRUMA_NUMBER_TYPE val) { this->type_ = val; }

 protected:
  TRUMA_NUMBER_TYPE type_;

  void control(float value) override;

 private:
};
}  // namespace truma_inetbox
}  // namespace esphome