#pragma once

#include "esphome/components/climate/climate.h"
#include "esphome/components/truma_inetbox/TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {
class TrumaWaterClimate : public Component, public climate::Climate, public Parented<TrumaiNetBoxApp> {
 public:
  void setup() override;

  void dump_config() override;

  void control(const climate::ClimateCall &call) override;

  climate::ClimateTraits traits() override;

 protected:
 private:
};
}  // namespace truma_inetbox
}  // namespace esphome