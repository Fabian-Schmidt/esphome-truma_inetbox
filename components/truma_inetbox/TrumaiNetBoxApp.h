#pragma once

#include "LinBusProtocol.h"
#include "TrumaStructs.h"
#include "TrumaiNetBoxAppAirconAuto.h"
#include "TrumaiNetBoxAppAirconManual.h"
#include "TrumaiNetBoxAppClock.h"
#include "TrumaiNetBoxAppConfig.h"
#include "TrumaiNetBoxAppHeater.h"
#include "TrumaiNetBoxAppTimer.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif  // USE_TIME

namespace esphome {
namespace truma_inetbox {

#define LIN_PID_TRUMA_INET_BOX 0x18

class TrumaiNetBoxApp : public LinBusProtocol {
 public:
  TrumaiNetBoxApp();
  void update() override;

  const std::array<u_int8_t, 4> lin_identifier() override;
  void lin_heartbeat() override;
  void lin_reset_device() override;

  TRUMA_DEVICE get_heater_device() const { return this->heater_device_; }
  TRUMA_DEVICE get_aircon_device() const { return this->aircon_device_; }

  TrumaiNetBoxAppAirconAuto *get_aircon_auto() { return &this->airconAuto_; }
  TrumaiNetBoxAppAirconManual *get_aircon_manual() { return &this->airconManual_; }
  TrumaiNetBoxAppClock *get_clock() { return &this->clock_; }
  TrumaiNetBoxAppConfig *get_config() { return &this->config_; }
  TrumaiNetBoxAppHeater *get_heater() { return &this->heater_; }
  TrumaiNetBoxAppTimer *get_timer() { return &this->timer_; }

  int64_t get_last_cp_plus_request() { return this->device_registered_; }

#ifdef USE_TIME
  void set_time(time::RealTimeClock *time) { time_ = time; }
  time::RealTimeClock *get_time() const { return time_; }
#endif  // USE_TIME

 protected:
  // Truma CP Plus needs init (reset). This device is not registered.
  uint32_t device_registered_ = 0;
  uint32_t init_requested_ = 0;
  uint32_t init_recieved_ = 0;
  u_int8_t message_counter = 1;

  // Truma heater conected to CP Plus.
  TRUMA_COMPANY company_ = TRUMA_COMPANY::TRUMA;
  TRUMA_DEVICE heater_device_ = TRUMA_DEVICE::UNKNOWN;
  TRUMA_DEVICE aircon_device_ = TRUMA_DEVICE::UNKNOWN;

  TrumaiNetBoxAppAirconAuto airconAuto_;
  TrumaiNetBoxAppAirconManual airconManual_;
  TrumaiNetBoxAppClock clock_;
  TrumaiNetBoxAppConfig config_;
  TrumaiNetBoxAppHeater heater_;
  TrumaiNetBoxAppTimer timer_;

  // last time CP plus was informed I got an update msg.
  uint32_t update_time_ = 0;

#ifdef USE_TIME
  time::RealTimeClock *time_ = nullptr;

  // Mark if the initial clock sync was done.
  bool update_status_clock_done = false;
#endif  // USE_TIME

  bool answer_lin_order_(const u_int8_t pid) override;

  bool lin_read_field_by_identifier_(u_int8_t identifier, std::array<u_int8_t, 5> *response) override;
  const u_int8_t *lin_multiframe_recieved(const u_int8_t *message, const u_int8_t message_len,
                                          u_int8_t *return_len) override;

  bool has_update_to_submit_();
};

}  // namespace truma_inetbox
}  // namespace esphome