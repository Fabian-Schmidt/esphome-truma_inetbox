#include "TrumaHeaterBinarySensor.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.binary_sensor";

void TrumaHeaterBinarySensor::setup() {
  this->parent_->register_listener([this](const StatusFrameHeater *status_heater) {
    switch (this->type_) {
      case TRUMA_BINARY_SENSOR_TYPE::HEATER_ROOM:
        this->publish_state(status_heater->target_temp_room != TargetTemp::TARGET_TEMP_OFF);
        break;
      case TRUMA_BINARY_SENSOR_TYPE::HEATER_WATER:
        this->publish_state(status_heater->target_temp_water != TargetTemp::TARGET_TEMP_OFF);
        break;
      case TRUMA_BINARY_SENSOR_TYPE::HEATER_GAS:
        this->publish_state(status_heater->energy_mix_a == EnergyMix::ENERGY_MIX_GAS);
        break;
      case TRUMA_BINARY_SENSOR_TYPE::HEATER_MIX_1:
        this->publish_state(status_heater->energy_mix_a == EnergyMix::ENERGY_MIX_MIX &&
                            status_heater->el_power_level_a == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_900);
        break;
      case TRUMA_BINARY_SENSOR_TYPE::HEATER_MIX_2:
        this->publish_state(status_heater->energy_mix_a == EnergyMix::ENERGY_MIX_MIX &&
                            status_heater->el_power_level_a == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_1800);
        break;
      case TRUMA_BINARY_SENSOR_TYPE::HEATER_ELECTRICITY:
        this->publish_state(status_heater->energy_mix_a == EnergyMix::ENERGY_MIX_ELECTRICITY);
        break;
      default:
        break;
    }
  });
}

void TrumaHeaterBinarySensor::dump_config() {
  ESP_LOGCONFIG("", "Truma Heater Binary Sensor");
  ESP_LOGCONFIG(TAG, "Type %u", this->type_);
}
}  // namespace truma_inetbox
}  // namespace esphome