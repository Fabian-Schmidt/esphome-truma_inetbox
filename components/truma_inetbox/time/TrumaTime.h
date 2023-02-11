#pragma once

#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/truma_inetbox/TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

class TrumaTime : public time::RealTimeClock, public Parented<TrumaiNetBoxApp> {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  bool read_time();

  void set_auto_disable(bool val) { this->auto_disable_ = val; }

 protected:
  bool auto_disable_ = false;
  u_int8_t auto_disable_count_ = 3;

 private:
};
}  // namespace truma_inetbox
}  // namespace esphome