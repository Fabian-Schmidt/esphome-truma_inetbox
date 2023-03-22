#pragma once

#include "esphome/core/component.h"
#include "TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

template<typename... Ts> class HeaterRoomTempAction : public Action<Ts...>, public Parented<TrumaiNetBoxApp> {
 public:
  TEMPLATABLE_VALUE(u_int8_t, temperature)
  TEMPLATABLE_VALUE(HeatingMode, heating_mode)

  void play(Ts... x) override {
    this->parent_->get_heater()->action_heater_room(this->temperature_.value_or(x..., 0),
                                                    this->heating_mode_.value_or(x..., HeatingMode::HEATING_MODE_OFF));
  }
};

template<typename... Ts> class HeaterWaterTempAction : public Action<Ts...>, public Parented<TrumaiNetBoxApp> {
 public:
  TEMPLATABLE_VALUE(u_int8_t, temperature)

  void play(Ts... x) override {
    this->parent_->get_heater()->action_heater_water(this->temperature_.value_or(x..., 0));
  }
};

template<typename... Ts> class HeaterWaterTempEnumAction : public Action<Ts...>, public Parented<TrumaiNetBoxApp> {
 public:
  TEMPLATABLE_VALUE(TargetTemp, temperature)

  void play(Ts... x) override {
    this->parent_->get_heater()->action_heater_water(this->temperature_.value_or(x..., TargetTemp::TARGET_TEMP_OFF));
  }
};

template<typename... Ts> class HeaterElecPowerLevelAction : public Action<Ts...>, public Parented<TrumaiNetBoxApp> {
 public:
  TEMPLATABLE_VALUE(u_int16_t, watt)

  void play(Ts... x) override {
    this->parent_->get_heater()->action_heater_electric_power_level(this->watt_.value_or(x..., 0));
  }
};

template<typename... Ts> class HeaterEnergyMixAction : public Action<Ts...>, public Parented<TrumaiNetBoxApp> {
 public:
  TEMPLATABLE_VALUE(EnergyMix, energy_mix)
  TEMPLATABLE_VALUE(ElectricPowerLevel, watt)

  void play(Ts... x) override {
    this->parent_->get_heater()->action_heater_energy_mix(
        this->energy_mix_.value_or(x..., EnergyMix::ENERGY_MIX_GAS),
        this->watt_.value_or(x..., ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0));
  }
};

template<typename... Ts> class AirconManualTempAction : public Action<Ts...>, public Parented<TrumaiNetBoxApp> {
 public:
  TEMPLATABLE_VALUE(u_int8_t, temperature)

  void play(Ts... x) override {
    this->parent_->get_aircon_manual()->action_set_temp(this->temperature_.value_or(x..., 0));
  }
};

template<typename... Ts> class TimerDisableAction : public Action<Ts...>, public Parented<TrumaiNetBoxApp> {
 public:
  void play(Ts... x) override { this->parent_->get_timer()->action_timer_disable(); }
};

template<typename... Ts> class TimerActivateAction : public Action<Ts...>, public Parented<TrumaiNetBoxApp> {
 public:
  TEMPLATABLE_VALUE(u_int16_t, start)
  TEMPLATABLE_VALUE(u_int16_t, stop)
  TEMPLATABLE_VALUE(u_int8_t, room_temperature)
  TEMPLATABLE_VALUE(HeatingMode, heating_mode)
  TEMPLATABLE_VALUE(u_int8_t, water_temperature)
  TEMPLATABLE_VALUE(EnergyMix, energy_mix)
  TEMPLATABLE_VALUE(ElectricPowerLevel, watt)

  void play(Ts... x) override {
    this->parent_->get_timer()->action_timer_activate(
        this->start_.value(x...), this->stop_.value(x...), this->room_temperature_.value(x...),
        this->heating_mode_.value_or(x..., HeatingMode::HEATING_MODE_OFF), this->water_temperature_.value_or(x..., 0),
        this->energy_mix_.value_or(x..., EnergyMix::ENERGY_MIX_NONE),
        this->watt_.value_or(x..., ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0));
  }
};

#ifdef USE_TIME
template<typename... Ts> class WriteTimeAction : public Action<Ts...>, public Parented<TrumaiNetBoxApp> {
 public:
  void play(Ts... x) override { this->parent_->get_clock()->action_write_time(); }
};
#endif  // USE_TIME

class TrumaiNetBoxAppHeaterMessageTrigger : public Trigger<const StatusFrameHeater *> {
 public:
  explicit TrumaiNetBoxAppHeaterMessageTrigger(TrumaiNetBoxApp *parent) {
    parent->get_heater()->add_on_message_callback([this](const StatusFrameHeater *message) { this->trigger(message); });
  }
};

}  // namespace truma_inetbox
}  // namespace esphome