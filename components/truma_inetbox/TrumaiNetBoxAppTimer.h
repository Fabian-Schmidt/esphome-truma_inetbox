#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppTimer : public TrumaStausFrameResponseStorage<StatusFrameTimer, StatusFrameTimerResponse> {
 public:
  StatusFrameTimerResponse *update_prepare() override;
  void create_update_data(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter) override;
  void dump_data() const override;
  
  bool action_timer_disable();
  bool action_timer_activate(u_int16_t start, u_int16_t stop, u_int8_t room_temperature,
                             HeatingMode mode = HeatingMode::HEATING_MODE_OFF, u_int8_t water_temperature = 0,
                             EnergyMix energy_mix = EnergyMix::ENERGY_MIX_NONE,
                             ElectricPowerLevel el_power_level = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0);
};

}  // namespace truma_inetbox
}  // namespace esphome