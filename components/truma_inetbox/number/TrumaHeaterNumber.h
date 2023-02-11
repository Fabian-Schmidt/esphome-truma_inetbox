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