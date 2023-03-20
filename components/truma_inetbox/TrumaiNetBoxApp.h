#pragma once

#include <vector>
#include "LinBusProtocol.h"
#include "esphome/core/automation.h"

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
#define STATUS_FRAME_AIRCON_RESPONSE (STATUS_FRAME_AIRCON - 1)
#define STATUS_FRAME_AIRCON 0x35
#define STATUS_FRAME_AIRCON_2_RESPONSE (STATUS_FRAME_AIRCON_2 - 1)
#define STATUS_FRAME_AIRCON_2 0x37
#define STATUS_FRAME_TIMER_RESPONSE (STATUS_FRAME_TIMER - 1)
#define STATUS_FRAME_TIMER 0x3D
#define STATUS_FRAME_AIRCON_INIT_RESPONSE (STATUS_FRAME_AIRCON_INIT - 1)
#define STATUS_FRAME_AIRCON_INIT 0x3F
#define STATUS_FRAME_AIRCON_INIT_2_RESPONSE (STATUS_FRAME_AIRCON_INIT_2 - 1)
#define STATUS_FRAME_AIRCON_INIT_2 0x41

enum class HeatingMode : u_int16_t {
  HEATING_MODE_OFF = 0x0,
  // COMBI
  HEATING_MODE_ECO = 0x1,
  // Vario Heat
  HEATING_MODE_VARIO_HEAT_NIGHT = 0x2,
  // Vario Heat
  HEATING_MODE_VARIO_HEAT_AUTO = 0x3,
  // COMBI
  HEATING_MODE_HIGH = 0xA,
  // COMBI, Vario Heat
  HEATING_MODE_BOOST = 0xB,

  // Feedback Invalid message only with following `heating_mode`. Others are ignored no feedback.
  // 00FF
  // 01FE
  // 02FD
  // ....
  // FD02
  // FE01
  // FF00
};

enum class ElectricPowerLevel : u_int16_t {
  ELECTRIC_POWER_LEVEL_0 = 0,
  ELECTRIC_POWER_LEVEL_900 = 900,
  ELECTRIC_POWER_LEVEL_1800 = 1800,
};

enum class TargetTemp : u_int16_t {
  TARGET_TEMP_OFF = 0x0,

  // 40C
  TARGET_TEMP_WATER_ECO = (40 + 273) * 10,
  // 60C
  TARGET_TEMP_WATER_HIGH = (60 + 273) * 10,
  // 200C
  TARGET_TEMP_WATER_BOOST = (200 + 273) * 10,

  TARGET_TEMP_05C = (5 + 273) * 10,
  TARGET_TEMP_06C = (6 + 273) * 10,
  TARGET_TEMP_07C = (7 + 273) * 10,
  TARGET_TEMP_08C = (8 + 273) * 10,
  TARGET_TEMP_09C = (9 + 273) * 10,
  TARGET_TEMP_10C = (10 + 273) * 10,
  TARGET_TEMP_11C = (11 + 273) * 10,
  TARGET_TEMP_12C = (12 + 273) * 10,
  TARGET_TEMP_13C = (13 + 273) * 10,
  TARGET_TEMP_14C = (14 + 273) * 10,
  TARGET_TEMP_15C = (15 + 273) * 10,
  TARGET_TEMP_16C = (16 + 273) * 10,
  TARGET_TEMP_17C = (17 + 273) * 10,
  TARGET_TEMP_18C = (18 + 273) * 10,
  TARGET_TEMP_19C = (19 + 273) * 10,
  TARGET_TEMP_20C = (20 + 273) * 10,
  TARGET_TEMP_21C = (21 + 273) * 10,
  TARGET_TEMP_22C = (22 + 273) * 10,
  TARGET_TEMP_23C = (23 + 273) * 10,
  TARGET_TEMP_24C = (24 + 273) * 10,
  TARGET_TEMP_25C = (25 + 273) * 10,
  TARGET_TEMP_26C = (26 + 273) * 10,
  TARGET_TEMP_27C = (27 + 273) * 10,
  TARGET_TEMP_28C = (28 + 273) * 10,
  TARGET_TEMP_29C = (29 + 273) * 10,
  TARGET_TEMP_30C = (30 + 273) * 10,
  TARGET_TEMP_31C = (31 + 273) * 10,

  TARGET_TEMP_ROOM_MIN = (5 + 273) * 10,
  TARGET_TEMP_ROOM_MAX = (30 + 273) * 10,

  TARGET_TEMP_AIRCON_MIN = (16 + 273) * 10,
  TARGET_TEMP_AIRCON_MAX = (31 + 273) * 10,

  TARGET_TEMP_AIRCON_AUTO_MIN = (18 + 273) * 10,
  TARGET_TEMP_AIRCON_AUTO_MAX = (25 + 273) * 10,
};

enum class EnergyMix : u_int8_t {
  ENERGY_MIX_NONE = 0b00,
  ENERGY_MIX_GAS = 0b01,
  ENERGY_MIX_ELECTRICITY = 0b10,
  ENERGY_MIX_MIX = 0b11,
};

enum class OperatingStatus : u_int8_t {
  OPERATING_STATUS_UNSET = 0x0,
  OPERATING_STATUS_OFF = 0x0,
  OPERATING_STATUS_WARNING = 0x1,
  OPERATING_STATUS_START_OR_COOL_DOWN = 0x4,
  // ? Gas Heating mode ?
  OPERATING_STATUS_ON_5 = 0x5,
  OPERATING_STATUS_ON_6 = 0x6,
  OPERATING_STATUS_ON_7 = 0x7,
  OPERATING_STATUS_ON_8 = 0x8,
  OPERATING_STATUS_ON_9 = 0x9,
};

enum class OperatingUnits : u_int8_t {
  OPERATING_UNITS_CELSIUS = 0x0,
  OPERATING_UNITS_FAHRENHEIT = 0x1,
};

enum class Language : u_int8_t {
  LANGUAGE_GERMAN = 0x0,
  LANGUAGE_ENGLISH = 0x1,
  LANGUAGE_FRENCH = 0x2,
  LANGUAGE_ITALY = 0x3,
};

enum class ResponseAckResult : u_int8_t {
  RESPONSE_ACK_RESULT_OKAY = 0x0,
  RESPONSE_ACK_RESULT_ERROR_INVALID_MSG = 0x2,
  // The response status frame `message_type` is unknown.
  RESPONSE_ACK_RESULT_ERROR_INVALID_ID = 0x3,
};

enum class ClockMode : u_int8_t {
  CLOCK_MODE_24H = 0x0,
  CLOCK_MODE_12H = 0x1,
};

enum class TimerActive : u_int8_t {
  TIMER_ACTIVE_ON = 0x1,
  TIMER_ACTIVE_OFF = 0x0,
};

enum class ClockSource : u_int8_t {
  // Set by user
  CLOCK_SOURCE_MANUAL = 0x1,
  // Set by message
  CLOCK_SOURCE_PROG = 0x2,
};

struct StatusFrameHeader {  // NOLINT(altera-struct-pack-align)
  // sid
  u_int8_t service_identifier;
  u_int8_t header[10];
  u_int8_t header_2;
  u_int8_t header_3;
  // after checksum
  u_int8_t message_length;
  u_int8_t message_type;
  u_int8_t command_counter;
  u_int8_t checksum;
} __attribute__((packed));

// Length 20 (0x14)
struct StatusFrameHeater {  // NOLINT(altera-struct-pack-align)
  TargetTemp target_temp_room;
  // Room
  HeatingMode heating_mode;
  ElectricPowerLevel el_power_level_a;
  TargetTemp target_temp_water;
  ElectricPowerLevel el_power_level_b;
  EnergyMix energy_mix_a;
  // Ignored by response
  EnergyMix energy_mix_b;
  u_int16_t current_temp_water;
  u_int16_t current_temp_room;
  OperatingStatus operating_status;
  u_int8_t error_code_low;
  u_int8_t error_code_high;
  u_int8_t heater_unknown_2;
} __attribute__((packed));

// Length 12 (0x0C)
struct StatusFrameHeaterResponse {  // NOLINT(altera-struct-pack-align)
  TargetTemp target_temp_room;
  // Room
  HeatingMode heating_mode;
  ElectricPowerLevel el_power_level_a;
  TargetTemp target_temp_water;
  ElectricPowerLevel el_power_level_b;
  EnergyMix energy_mix_a;
  // Ignored?
  EnergyMix energy_mix_b;
} __attribute__((packed));

// Length 24 (0x18)
struct StatusFrameTimer {  // NOLINT(altera-struct-pack-align)
  TargetTemp timer_target_temp_room;
  HeatingMode timer_heating_mode;
  ElectricPowerLevel timer_el_power_level_a;
  TargetTemp timer_target_temp_water;
  ElectricPowerLevel timer_el_power_level_b;
  EnergyMix timer_energy_mix_a;
  EnergyMix timer_energy_mix_b;
  // used by timer response message
  u_int8_t unused[5];
  u_int8_t timer_unknown_3;
  u_int8_t timer_unknown_4;
  TimerActive timer_active;
  u_int8_t timer_start_minutes;
  u_int8_t timer_start_hours;
  u_int8_t timer_stop_minutes;
  u_int8_t timer_stop_hours;
} __attribute__((packed));

// Length 13 (0x0D)
struct StatusFrameTimerResponse {  // NOLINT(altera-struct-pack-align)
  TargetTemp timer_target_temp_room;
  HeatingMode timer_heating_mode;
  ElectricPowerLevel timer_el_power_level_a;
  TargetTemp timer_target_temp_water;
  ElectricPowerLevel timer_el_power_level_b;
  EnergyMix timer_energy_mix_a;
  EnergyMix timer_energy_mix_b;
  // set by response message to active timer
  TimerActive timer_resp_active;
  // set by response message to active timer
  u_int8_t timer_resp_start_minutes;
  // set by response message to active timer
  u_int8_t timer_resp_start_hours;
  // set by response message to active timer
  u_int8_t timer_resp_stop_minutes;
  // set by response message to active timer
  u_int8_t timer_resp_stop_hours;
} __attribute__((packed));

// Length 2 (0x02)
struct StatusFrameResponseAck {  // NOLINT(altera-struct-pack-align)
  ResponseAckResult error_code;
  u_int8_t unknown;
} __attribute__((packed));

// Length 10 (0x0A)
struct StatusFrameClock {  // NOLINT(altera-struct-pack-align)
  u_int8_t clock_hour;
  u_int8_t clock_minute;
  u_int8_t clock_second;
  // MUST be 0x1, 0x2, 0x3..? (lower than 0x9)
  u_int8_t display_1;
  // MUST be 0x1
  u_int8_t display_2;
  u_int8_t display_3;
  ClockMode clock_mode;
  ClockSource clock_source;
  u_int8_t display_4;
  u_int8_t display_5;
} __attribute__((packed));

// Length 10 (0x0A)
struct StatusFrameConfig {  // NOLINT(altera-struct-pack-align)
  // 0x01 .. 0x0A
  u_int8_t display_brightness;
  Language language;
  // Mit „AC SET“ wird ein Offset zwischen Kühlen und Heizen eingestellt.
  // Die Einstellung ist in Schritten von 0,5 °C im Bereich von 0 °C bis +5 °C möglich.
  TargetTemp ac_offset;
  TargetTemp temp_offset;
  OperatingUnits temp_units;
  u_int8_t unknown_6;
  u_int8_t unknown_7;
  u_int8_t unknown_8;
} __attribute__((packed));

enum class TRUMA_DEVICE : u_int8_t {
  UNKNOWN = 0x00,
  // Saphir Compact AC
  AIRCON_DEVICE = 0x01,
  // Combi 4
  HEATER_COMBI4 = 0x02,
  // Vario Heat Comfort (non E)
  HEATER_VARIO = 0x03,
  // CP Plus for Combi
  CPPLUS_COMBI = 0x04,
  // CP Plus for Vario Heat
  CPPLUS_VARIO = 0x05,
  // Combi 6 D
  HEATER_COMBI6D = 0x06,
};

// Length 12 (0x0C)
struct StatusFrameDevice {  // NOLINT(altera-struct-pack-align)
  u_int8_t device_count;
  u_int8_t device_id;
  // 0x01 - Maybe active or found
  u_int8_t unknown_0;
  // 0x00
  u_int8_t unknown_1;
  u_int16_t hardware_revision_major;
  u_int8_t hardware_revision_minor;
  // `software_revision[0].software_revision[1].software_revision[2]`
  // software_revision[0] -> TRUMA_DEVICE
  u_int8_t software_revision[3];
  // 0xAD, 0x0B on CPplus with Combi4 or 0x66 on CPplus with Vario Heat Comfort ohne E
  // 0x00 on Combi4, Vario Heat
  u_int8_t unknown_2;
  // 0x10, 0x12 on CPplus
  // 0x00 on Combi4, Vario Heat
  u_int8_t unknown_3;
} __attribute__((packed));

enum class AirconMode : u_int8_t {
  // Auto - 18 to 25
  OFF = 0x00,
  AC_VENTILATION = 0x04,
  AC_COOLING = 0x05,
};

enum class AirconOperation : u_int8_t {
  AC_ONLY = 0x71,
  // Heater and Aircon
  AUTO = 0x72,
};

// Length 18 (0x12)
// TODO
struct StatusFrameAircon {  // NOLINT(altera-struct-pack-align)
  AirconMode mode;
  // 0x00
  u_int8_t unknown_02;
  AirconOperation operation;
  EnergyMix energy_mix;
  TargetTemp target_temp_aircon;
  // 0x00
  u_int8_t unknown_07;
  // 0x00
  u_int8_t unknown_08;
  // No idea why two current_temp
  TargetTemp current_temp_aircon;
  // 0x00
  u_int8_t unknown_11;
  // 0x00
  u_int8_t unknown_12;
  ElectricPowerLevel el_power_level;
  // 0x00
  u_int8_t unknown_15;
  // 0x00
  u_int8_t unknown_16;
  TargetTemp current_temp_room;
} __attribute__((packed));

struct StatusFrameAirconResponse {  // NOLINT(altera-struct-pack-align)
  AirconMode mode;
  // 0x00
  u_int8_t unknown_02;
  AirconOperation operation;
  EnergyMix energy_mix;
  TargetTemp target_temp_aircon;
} __attribute__((packed));

// Length 18 (0x12)
// TODO
struct StatusFrameAircon2 {  // NOLINT(altera-struct-pack-align)
  EnergyMix energy_mix_a;
  u_int8_t unknown_02;  // 0x00
  EnergyMix energy_mix_b;
  u_int8_t unknown_04;  // 0x00
  u_int8_t unknown_05;  // 0x00
  u_int8_t unknown_06;  // 0x00
  TargetTemp target_temp_aircon_auto;
  ElectricPowerLevel el_power_level_a;
  u_int8_t unknown_11;  // 0x00
  u_int8_t unknown_12;  // 0x00
  ElectricPowerLevel el_power_level_b;
  TargetTemp current_temp;
  TargetTemp target_temp;
} __attribute__((packed));

// Length 22 (0x16)
// TODO
struct StatusFrameAirconInit {  // NOLINT(altera-struct-pack-align)
  u_int8_t unknown_01;          // 0x00
  u_int8_t unknown_02;          // 0x00
  AirconOperation operation;
  EnergyMix energy_mix;
  u_int8_t unknown_05;  // 0x00
  u_int8_t unknown_06;  // 0x00
  u_int8_t unknown_07;  // 0x00
  u_int8_t unknown_08;  // 0x00
  u_int8_t unknown_09;  // 0x00
  u_int8_t unknown_10;  // 0x00
  u_int8_t unknown_11;  // 0x00
  u_int8_t unknown_12;  // 0x00
  u_int8_t unknown_13;  // 0x00
  u_int8_t unknown_14;  // 0x00
  u_int8_t unknown_15;  // 0x00
  u_int8_t unknown_16;  // 0x00
  u_int8_t unknown_17;  // 0x00
  u_int8_t unknown_18;  // 0x00
  u_int8_t unknown_19;  // 0x00
  u_int8_t unknown_20;  // 0x00
  u_int8_t unknown_21;  // 0x00
  u_int8_t unknown_22;  // 0x00
} __attribute__((packed));

// Length 20 (0x14)
// TODO
struct StatusFrameAirconInit2 {  // NOLINT(altera-struct-pack-align)
  EnergyMix energy_mix_a;
  u_int8_t unknown_02;  // 0x00
  EnergyMix energy_mix_b;
  u_int8_t unknown_04;  // 0x00
  u_int8_t unknown_05;  // 0x00
  u_int8_t unknown_06;  // 0x00
  u_int8_t unknown_07;  // 0x00
  u_int8_t unknown_08;  // 0x00
  u_int8_t unknown_09;  // 0x00
  u_int8_t unknown_10;  // 0x00
  u_int8_t unknown_11;  // 0x00
  u_int8_t unknown_12;  // 0x00
  u_int8_t unknown_13;  // 0x00
  u_int8_t unknown_14;  // 0x00
  u_int8_t unknown_15;  // 0x00
  u_int8_t unknown_16;  // 0x00
  u_int8_t unknown_17;  // 0x00
  u_int8_t unknown_18;  // 0x00
  u_int8_t unknown_19;  // 0x00
  u_int8_t unknown_20;  // 0x00
} __attribute__((packed));

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
      StatusFrameAircon aircon;
      StatusFrameAirconResponse airconResponse;
      StatusFrameAirconInit airconInit;
      StatusFrameAircon2 aircon2;
      StatusFrameAirconInit2 airconInit2;
    } __attribute__((packed));
  } inner;
} __attribute__((packed));

template<typename T> class TrumaStausFrameStorage {
 public:
  CallbackManager<void(const T *)> state_callback_{};
  T data_;
  bool data_valid_ = false;
  // Value has changed notify listeners.
  bool data_updated_ = false;

  // bool get_status_valid() { return this->data_valid_; }
  // const T *get_status() { return &this->data_; }
  void update();
  void reset();
};

template<typename T, typename TResponse> class TrumaStausFrameResponseStorage : public TrumaStausFrameStorage<T> {
 public:
  void reset();

  // Prepared means `update_status_` was copied from `data_`.
  bool update_status_prepared_ = false;
  // Prepared means an update is already awating fetch from CP plus.
  bool update_status_unsubmitted_ = false;
  // I have submitted my update request to CP plus, but I have not recieved an update with new heater values from CP
  // plus.
  bool update_status_stale_ = false;
  TResponse update_status_;
};

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

  bool truma_aircon_can_update() { return this->status_aircon_valid_; }
  StatusFrameAirconResponse *update_aircon_prepare();
  void update_aircon_submit() { this->update_status_aircon_unsubmitted_ = true; }

  bool truma_timer_can_update() { return this->timer_.data_valid_; }
  StatusFrameTimerResponse *update_timer_prepare();
  void update_timer_submit() { this->timer_.update_status_unsubmitted_ = true; }

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
  TrumaStausFrameStorage<StatusFrameConfig> config_;
  TrumaStausFrameStorage<StatusFrameClock> clock_;

  bool status_aircon_valid_ = false;
  // Value has changed notify listeners.
  bool status_aircon_updated_ = false;
  StatusFrameAircon status_aircon_;

  // last time CP plus was informed I got an update msg.
  uint32_t update_time_ = 0;

  bool update_status_aircon_prepared_ = false;
  bool update_status_aircon_unsubmitted_ = false;
  bool update_status_aircon_stale_ = false;
  StatusFrameAirconResponse update_status_aircon_;

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