#include "TrumaiNetBoxAppAldeStatus.h"
#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "helpers.h"
#include "TrumaiNetBoxApp.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxAppAldeStatus";

StatusFrameAldeStatusResponse *TrumaiNetBoxAppAldeStatus::update_prepare() {
  // An update is currently going on.
  if (this->update_status_prepared_ || this->update_status_stale_) {
    return &this->update_status_;
  }

  // prepare status heater response
  this->update_status_ = {};
  this->update_status_.heater_mode = this->data_.heater_mode;
  this->update_status_.unknown_01 = this->data_.unknown_01;
  this->update_status_.target_temp_room = this->data_.target_temp_room;
  this->update_status_.unknown_04 = this->data_.unknown_04;
  this->update_status_.water_mode = this->data_.water_mode;
  this->update_status_.el_mode = this->data_.el_mode;
  this->update_status_.gas_mode = this->data_.gas_mode;
  this->update_status_.heater_prio = this->data_.heater_prio;
  this->update_status_.unknown_10 = this->data_.unknown_10;
  this->update_status_.unknown_11 = this->data_.unknown_11;
  this->update_status_.unknown_12 = this->data_.unknown_12;
  this->update_status_.unknown_13 = this->data_.unknown_13;
  this->update_status_.unknown_14 = this->data_.unknown_14;
  this->update_status_.message_counter = this->data_.message_counter + 1;

  this->update_status_prepared_ = true;
  return &this->update_status_;
}

void TrumaiNetBoxAppAldeStatus::create_update_data(StatusFrame *response, u_int8_t *response_len,
                                                   u_int8_t command_counter) {
  status_frame_create_empty(response, STATUS_FRAME_ALDE_STATUS_RESPONSE, sizeof(StatusFrameAldeStatusResponse),
                            command_counter);

  response->aldeStatusResponse.heater_mode = this->update_status_.heater_mode;
  response->aldeStatusResponse.unknown_01 = this->update_status_.unknown_01;
  response->aldeStatusResponse.target_temp_room = this->update_status_.target_temp_room;
  response->aldeStatusResponse.unknown_04 = this->update_status_.unknown_04;
  response->aldeStatusResponse.water_mode = this->update_status_.water_mode;
  response->aldeStatusResponse.el_mode = this->update_status_.el_mode;
  response->aldeStatusResponse.gas_mode = this->update_status_.gas_mode;
  response->aldeStatusResponse.heater_prio = this->update_status_.heater_prio;
  response->aldeStatusResponse.unknown_10 = this->update_status_.unknown_10;
  response->aldeStatusResponse.unknown_11 = this->update_status_.unknown_11;
  response->aldeStatusResponse.unknown_12 = this->update_status_.unknown_12;
  response->aldeStatusResponse.unknown_13 = this->update_status_.unknown_13;
  response->aldeStatusResponse.unknown_14 = this->update_status_.unknown_14;
  response->aldeStatusResponse.message_counter = this->update_status_.message_counter;

  status_frame_calculate_checksum(response);
  (*response_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameAldeStatusResponse);

  TrumaStausFrameResponseStorage<StatusFrameAldeStatus, StatusFrameAldeStatusResponse>::update_submitted();
}

void TrumaiNetBoxAppAldeStatus::dump_data() const {}

bool TrumaiNetBoxAppAldeStatus::can_update() {
  return TrumaStausFrameResponseStorage<StatusFrameAldeStatus, StatusFrameAldeStatusResponse>::can_update() &&
         // this->parent_->get_heater_device() != TRUMA_DEVICE::UNKNOWN;
         this->parent_->get_is_alde_device();
}

bool TrumaiNetBoxAppAldeStatus::action_heater_room(u_int8_t temperature) {
  if (!this->can_update()) {
    ESP_LOGW(TAG, "Cannot update Alde.");
    return false;
  }
  auto heater = this->update_prepare();

  heater->target_temp_room = decimal_to_room_temp(temperature);

  this->update_submit();
  return true;
}

// bool TrumaiNetBoxAppAldeStatus::action_heater_water(u_int8_t temperature) {
//   if (!this->can_update()) {
//     ESP_LOGW(TAG, "Cannot update Truma.");
//     return false;
//   }
//   auto heater = this->update_prepare();

//   heater->target_temp_water = decimal_to_water_temp(temperature);

//   // Ensure `energy_mix_a` is set.
//   if (heater->target_temp_water != TargetTemp::TARGET_TEMP_OFF && heater->energy_mix_a == EnergyMix::ENERGY_MIX_NONE)
//   {
//     heater->energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
//   }

//   this->update_submit();
//   return true;
// }

// bool TrumaiNetBoxAppAldeStatus::action_heater_water(TargetTemp temperature) {
//   if (!this->can_update()) {
//     ESP_LOGW(TAG, "Cannot update Truma.");
//     return false;
//   }
//   auto heater = this->update_prepare();

//   // If parameter `temperature` contains a valid mode use it or else use `OFF`.
//   if (temperature == TargetTemp::TARGET_TEMP_WATER_ECO || temperature == TargetTemp::TARGET_TEMP_WATER_HIGH ||
//       temperature == TargetTemp::TARGET_TEMP_WATER_BOOST) {
//     heater->target_temp_water = temperature;
//   } else {
//     heater->target_temp_water = TargetTemp::TARGET_TEMP_OFF;
//   }

//   // Ensure `energy_mix_a` is set.
//   if (heater->target_temp_water != TargetTemp::TARGET_TEMP_OFF && heater->energy_mix_a == EnergyMix::ENERGY_MIX_NONE)
//   {
//     heater->energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
//   }

//   this->update_submit();
//   return true;
// }

// bool TrumaiNetBoxAppAldeStatus::action_heater_electric_power_level(u_int16_t value) {
//   if (!this->can_update()) {
//     ESP_LOGW(TAG, "Cannot update Truma.");
//     return false;
//   }
//   auto heater = this->update_prepare();

//   heater->el_power_level_a = decimal_to_el_power_level(value);
//   if (heater->el_power_level_a != ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0) {
//     if (heater->energy_mix_a != EnergyMix::ENERGY_MIX_MIX &&
//         heater->energy_mix_a != EnergyMix::ENERGY_MIX_ELECTRICITY) {
//       heater->energy_mix_a = EnergyMix::ENERGY_MIX_MIX;
//     }
//   } else {
//     heater->energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
//   }

//   this->update_submit();
//   return true;
// }

// bool TrumaiNetBoxAppAldeStatus::action_heater_energy_mix(EnergyMix energy_mix, ElectricPowerLevel el_power_level) {
//   if (!this->can_update()) {
//     ESP_LOGW(TAG, "Cannot update Truma.");
//     return false;
//   }
//   auto heater = this->update_prepare();

//   // If parameter `el_power_level` contains a valid mode use it.
//   if (el_power_level == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0 ||
//       el_power_level == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_900 ||
//       el_power_level == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_1800) {
//     heater->el_power_level_a = el_power_level;
//   }

//   if (energy_mix == EnergyMix::ENERGY_MIX_GAS) {
//     heater->energy_mix_a = energy_mix;
//     heater->el_power_level_a = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0;
//   } else if (energy_mix == EnergyMix::ENERGY_MIX_MIX || energy_mix == EnergyMix::ENERGY_MIX_ELECTRICITY) {
//     heater->energy_mix_a = energy_mix;
//     // Electric energy is requested by user without a power level. Set it to minimum.
//     if (heater->el_power_level_a == ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0) {
//       heater->el_power_level_a = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_900;
//     }
//   }

//   // This last check is reached if invalid `energy_mix` parameter was submitted.
//   if (heater->el_power_level_a != ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0) {
//     if (heater->energy_mix_a != EnergyMix::ENERGY_MIX_MIX &&
//         heater->energy_mix_a != EnergyMix::ENERGY_MIX_ELECTRICITY) {
//       heater->energy_mix_a = EnergyMix::ENERGY_MIX_MIX;
//     }
//   } else {
//     heater->energy_mix_a = EnergyMix::ENERGY_MIX_GAS;
//   }

//   this->update_submit();
//   return true;
// }

}  // namespace truma_inetbox
}  // namespace esphome