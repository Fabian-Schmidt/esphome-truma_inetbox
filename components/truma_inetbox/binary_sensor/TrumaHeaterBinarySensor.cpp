#include "TrumaHeaterBinarySensor.h"
#include "esphome/core/log.h"
#include "esphome/components/truma_inetbox/helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.heater_binary_sensor";

void TrumaHeaterBinarySensor::setup() {
  this->parent_->get_heater()->add_on_message_callback([this](const StatusFrameHeater *status_heater) {
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
      case TRUMA_BINARY_SENSOR_TYPE::HEATER_DIESEL:
        this->publish_state(status_heater->energy_mix_a == EnergyMix::ENERGY_MIX_DIESEL);
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
      case TRUMA_BINARY_SENSOR_TYPE::HEATER_HAS_ERROR:
        this->publish_state(status_heater->error_code_high != 0x00);
        break;
      default:
        break;
    }
  });
}

void TrumaHeaterBinarySensor::dump_config() {
  LOG_BINARY_SENSOR("", "Truma Heater Binary Sensor", this);
  ESP_LOGCONFIG(TAG, "  Type '%s'", enum_to_c_str(this->type_));
}
}  // namespace truma_inetbox
}  // namespace esphome