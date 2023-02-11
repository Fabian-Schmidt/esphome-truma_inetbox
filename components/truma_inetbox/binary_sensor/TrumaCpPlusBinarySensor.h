#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/truma_inetbox/TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

class TrumaCpPlusBinarySensor : public PollingComponent,
                                public binary_sensor::BinarySensor,
                                public Parented<TrumaiNetBoxApp> {
 public:
  void update() override;
  void dump_config() override;

 protected:
 private:
};
}  // namespace truma_inetbox
}  // namespace esphome