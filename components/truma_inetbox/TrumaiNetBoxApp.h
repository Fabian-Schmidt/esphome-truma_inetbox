#pragma once

#include <vector>
#include "LinBusProtocol.h"
#include "esphome/core/automation.h"
#include "TrumaEnums.h"
#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStausFrameStorage.h"
#include "TrumaStructs.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif  // USE_TIME

namespace esphome {
namespace truma_inetbox {

#define LIN_PID_TRUMA_INET_BOX 0x18
#define LIN_SID_RESPONSE 0x40
#define LIN_SID_READ_STATE_BUFFER 0xBA
#define LIN_SID_FIll_STATE_BUFFFER 0xBB

// Response to init are the following frames:
// - 2 * STATUS_FRAME_DEVICES
// - STATUS_FRAME_HEATER
// - STATUS_FRAME_TIMER
// - STAUTS_FRAME_CONFIG
// - STATUS_FRAME_CLOCK
#define STATUS_FRAME_RESPONSE_INIT_REQUEST 0x0A
#define STATUS_FRAME_DEVICES 0x0B
#define STATUS_FRAME_RESPONSE_ACK 0x0D
#define STATUS_FRAME_CLOCK_RESPONSE (STATUS_FRAME_CLOCK - 1)
#define STATUS_FRAME_CLOCK 0x15
// TODO: Documentation and testing of config response.
#define STAUTS_FRAME_CONFIG_RESPONSE (STAUTS_FRAME_CONFIG - 1)
#define STAUTS_FRAME_CONFIG 0x17
#define STATUS_FRAME_HEATER_RESPONSE (STATUS_FRAME_HEATER - 1)
#define STATUS_FRAME_HEATER 0x33
#define STATUS_FRAME_AIRCON_MANUAL_RESPONSE (STATUS_FRAME_AIRCON_MANUAL - 1)
#define STATUS_FRAME_AIRCON_MANUAL 0x35
#define STATUS_FRAME_AIRCON_AUTO_RESPONSE (STATUS_FRAME_AIRCON_AUTO - 1)
#define STATUS_FRAME_AIRCON_AUTO 0x37
#define STATUS_FRAME_TIMER_RESPONSE (STATUS_FRAME_TIMER - 1)
#define STATUS_FRAME_TIMER 0x3D
#define STATUS_FRAME_AIRCON_MANUAL_INIT_RESPONSE (STATUS_FRAME_AIRCON_MANUAL_INIT - 1)
#define STATUS_FRAME_AIRCON_MANUAL_INIT 0x3F
#define STATUS_FRAME_AIRCON_AUTO_INIT_RESPONSE (STATUS_FRAME_AIRCON_AUTO_INIT - 1)
#define STATUS_FRAME_AIRCON_AUTO_INIT 0x41

union StatusFrame {  // NOLINT(altera-struct-pack-align)
  u_int8_t raw[41];
  struct inner {  // NOLINT(altera-struct-pack-align)
    StatusFrameHeader genericHeader;
    union {  // NOLINT(altera-struct-pack-align)
      StatusFrameHeater heater;
      StatusFrameHeaterResponse heaterResponse;
      StatusFrameTimer timer;
      StatusFrameTimerResponse timerResponse;
      StatusFrameResponseAck responseAck;
      StatusFrameClock clock;
      StatusFrameConfig config;
      StatusFrameDevice device;
      StatusFrameAirconManual airconManual;
      StatusFrameAirconManualResponse airconManualResponse;
      StatusFrameAirconManualInit airconManualInit;
      StatusFrameAirconAuto airconAuto;
      StatusFrameAirconAutoInit airconAutoInit;
    } __attribute__((packed));
  } inner;
} __attribute__((packed));


class TrumaiNetBoxApp : public LinBusProtocol {
 public:
  void update() override;

  const std::array<u_int8_t, 4> lin_identifier() override;
  void lin_heartbeat() override;
  void lin_reset_device() override;

  bool get_status_heater_valid() { return this->heater_.data_valid_; }
  const StatusFrameHeater *get_status_heater() { return &this->heater_.data_; }

  bool get_status_timer_valid() { return this->timer_.data_valid_; }
  const StatusFrameTimer *get_status_timer() { return &this->timer_.data_; }

  bool get_status_clock_valid() { return this->clock_.data_valid_; }
  const StatusFrameClock *get_status_clock() { return &this->clock_.data_; }

  bool get_status_config_valid() { return this->config_.data_valid_; }
  const StatusFrameConfig *get_status_config() { return &this->config_.data_; }

  bool truma_heater_can_update() { return this->heater_.data_valid_; }
  StatusFrameHeaterResponse *update_heater_prepare();
  void update_heater_submit() { this->heater_.update_status_unsubmitted_ = true; }

  bool truma_timer_can_update() { return this->timer_.data_valid_; }
  StatusFrameTimerResponse *update_timer_prepare();
  void update_timer_submit() { this->timer_.update_status_unsubmitted_ = true; }

  bool truma_aircon_can_update() { return this->airconManual_.data_valid_; }
  StatusFrameAirconManualResponse *update_aircon_prepare();
  void update_aircon_submit() { this->airconManual_.update_status_unsubmitted_ = true; }

  int64_t get_last_cp_plus_request() { return this->device_registered_; }

  // Automation
  void add_on_heater_message_callback(std::function<void(const StatusFrameHeater *)> callback) {
    this->heater_.state_callback_.add(std::move(callback));
  }
  void add_on_timer_message_callback(std::function<void(const StatusFrameTimer *)> callback) {
    this->timer_.state_callback_.add(std::move(callback));
  }
  void add_on_clock_message_callback(std::function<void(const StatusFrameClock *)> callback) {
    this->clock_.state_callback_.add(std::move(callback));
  }
  void add_on_config_message_callback(std::function<void(const StatusFrameConfig *)> callback) {
    this->config_.state_callback_.add(std::move(callback));
  }
  void add_on_aircon_manual_message_callback(std::function<void(const StatusFrameAirconManual *)> callback) {
    this->airconManual_.state_callback_.add(std::move(callback));
  }
  bool action_heater_room(u_int8_t temperature, HeatingMode mode = HeatingMode::HEATING_MODE_OFF);
  bool action_heater_water(u_int8_t temperature);
  bool action_heater_water(TargetTemp temperature);
  bool action_heater_electric_power_level(u_int16_t value);
  bool action_heater_energy_mix(EnergyMix energy_mix,
                                ElectricPowerLevel el_power_level = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0);
  bool action_timer_disable();
  bool action_timer_activate(u_int16_t start, u_int16_t stop, u_int8_t room_temperature,
                             HeatingMode mode = HeatingMode::HEATING_MODE_OFF, u_int8_t water_temperature = 0,
                             EnergyMix energy_mix = EnergyMix::ENERGY_MIX_NONE,
                             ElectricPowerLevel el_power_level = ElectricPowerLevel::ELECTRIC_POWER_LEVEL_0);

#ifdef USE_TIME
  void set_time(time::RealTimeClock *time) { time_ = time; }
  bool truma_clock_can_update() { return this->clock_.data_valid_; }
  void update_clock_submit() { this->update_status_clock_unsubmitted_ = true; }
  bool action_write_time();
#endif  // USE_TIME

 protected:
  // Truma CP Plus needs init (reset). This device is not registered.
  uint32_t device_registered_ = 0;
  uint32_t init_requested_ = 0;
  uint32_t init_recieved_ = 0;
  u_int8_t message_counter = 1;

  // Truma heater conected to CP Plus.
  TRUMA_DEVICE heater_device_ = TRUMA_DEVICE::HEATER_COMBI4;
  TRUMA_DEVICE aircon_device_ = TRUMA_DEVICE::UNKNOWN;

  TrumaStausFrameResponseStorage<StatusFrameHeater, StatusFrameHeaterResponse> heater_;
  TrumaStausFrameResponseStorage<StatusFrameTimer, StatusFrameTimerResponse> timer_;
  TrumaStausFrameResponseStorage<StatusFrameAirconManual, StatusFrameAirconManualResponse> airconManual_;
  TrumaStausFrameStorage<StatusFrameConfig> config_;
  TrumaStausFrameStorage<StatusFrameClock> clock_;

  // last time CP plus was informed I got an update msg.
  uint32_t update_time_ = 0;

#ifdef USE_TIME
  time::RealTimeClock *time_ = nullptr;

  // The behaviour of `update_status_clock_unsubmitted_` is special.
  // Just an update is marked. The actual package is prepared when CP Plus asks for the data in the
  // `lin_multiframe_recieved` method.
  bool update_status_clock_unsubmitted_ = false;

  // Mark if the initial clock sync was done.
  bool update_status_clock_done = false;
#else
  const bool update_status_clock_unsubmitted_ = false;
#endif  // USE_TIME

  bool answer_lin_order_(const u_int8_t pid) override;

  bool lin_read_field_by_identifier_(u_int8_t identifier, std::array<u_int8_t, 5> *response) override;
  const u_int8_t *lin_multiframe_recieved(const u_int8_t *message, const u_int8_t message_len,
                                          u_int8_t *return_len) override;

  bool has_update_to_submit_();
};

}  // namespace truma_inetbox
}  // namespace esphome