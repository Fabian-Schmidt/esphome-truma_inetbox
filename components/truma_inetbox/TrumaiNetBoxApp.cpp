#include "TrumaiNetBoxApp.h"
#include "TrumaStatusFrame.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxApp";

TrumaiNetBoxApp::TrumaiNetBoxApp(u_int8_t expected_listener_count) {
  this->listeners_heater_.reserve(expected_listener_count);
}

void TrumaiNetBoxApp::update() {
  // Call listeners in after method 'lin_multiframe_recieved' call.
  // Because 'lin_multiframe_recieved' is time critical an all these sensors can take some time.
  if (this->status_heater_updated_ || this->status_timer_updated_ || this->status_clock_updated_ ||
      this->status_config_updated_) {
    // Run through listeners
    for (auto &listener : this->listeners_heater_) {
      if (this->status_heater_updated_ && listener.on_heater_change != nullptr) {
        listener.on_heater_change(&this->status_heater_);
      }
      if (this->status_timer_updated_ && listener.on_timer_change != nullptr) {
        listener.on_timer_change(&this->status_timer_);
      }
      if (this->status_clock_updated_ && listener.on_clock_change != nullptr) {
        listener.on_clock_change(&this->status_clock_);
      }
      if (this->status_config_updated_ && listener.on_config_change != nullptr) {
        listener.on_config_change(&this->status_config_);
      }
    }

    // Run through callbacks
    if (this->status_heater_updated_) {
      this->state_heater_callback_.call(&this->status_heater_);
    }
    // update is handeld
    this->status_heater_updated_ = false;
    this->status_timer_updated_ = false;
    this->status_clock_updated_ = false;
    this->status_config_updated_ = false;
  }
}

const std::array<uint8_t, 4> TrumaiNetBoxApp::lin_identifier() {
  // Supplier Id: 0x4617 - Truma (Phone: +49 (0)89 4617-0)
  // Unkown:
  // 17.46.01.03 - Unkown more comms required for init.
  // 17.46.10.03 - Unkown more comms required for init.
  // Heater:
  // 17.46.40.03 - H2.00.01 - 0340.xx Combi 4/6
  // Aircon:
  // 17.46.00.0C - A23.70.0 - 0C00.xx (with light option: OFF/1..5)
  // 17.46.01.0C - A23.70.0 - 0C01.xx
  // 17.46.04.0C - A23.70.0 - 0C04.xx (with light option: OFF/1..5)
  // 17.46.05.0C - A23.70.0 - 0C05.xx
  // 17.46.06.0C - A23.70.0 - 0C06.xx (with light option: OFF/1..5)
  // 17.46.07.0C - A23.70.0 - 0C07.xx (with light option: OFF/1..5)
  // iNet Box:
  // 17.46.00.1F - T23.70.0 - 1F00.xx iNet Box
  return {0x17 /*Supplied Id*/, 0x46 /*Supplied Id*/, 0x00 /*Function Id*/, 0x1F /*Function Id*/};
}

void TrumaiNetBoxApp::lin_reset_device() {
  this->device_registered_ = esp_timer_get_time();
  this->init_recieved_ = 0;

  this->status_heater_valid_ = false;
  this->status_heater_updated_ = false;
  this->status_timer_valid_ = false;
  this->status_timer_updated_ = false;
  this->status_clock_valid_ = false;
  this->status_clock_updated_ = false;
  this->status_config_valid_ = false;
  this->status_config_updated_ = false;

  this->update_time_ = 0;

  this->update_status_heater_prepared_ = false;
  this->update_status_heater_unsubmitted_ = false;
  this->update_status_heater_stale_ = false;

  this->update_status_timer_prepared_ = false;
  this->update_status_timer_unsubmitted_ = false;
  this->update_status_timer_stale_ = false;
}

void TrumaiNetBoxApp::register_listener(const std::function<void(const StatusFrameHeater *)> &func) {
  auto listener = StatusFrameListener{
      .on_heater_change = func,
  };
  this->listeners_heater_.push_back(std::move(listener));

  if (this->status_heater_valid_) {
    func(&this->status_heater_);
  }
}
void TrumaiNetBoxApp::register_listener(const std::function<void(const StatusFrameTimer *)> &func) {
  auto listener = StatusFrameListener{
      .on_timer_change = func,
  };
  this->listeners_heater_.push_back(std::move(listener));

  if (this->status_timer_valid_) {
    func(&this->status_timer_);
  }
}
void TrumaiNetBoxApp::register_listener(const std::function<void(const StatusFrameClock *)> &func) {
  auto listener = StatusFrameListener{
      .on_clock_change = func,
  };
  this->listeners_heater_.push_back(std::move(listener));

  if (this->status_clock_valid_) {
    func(&this->status_clock_);
  }
}
void TrumaiNetBoxApp::register_listener(const std::function<void(const StatusFrameConfig *)> &func) {
  auto listener = StatusFrameListener{
      .on_config_change = func,
  };
  this->listeners_heater_.push_back(std::move(listener));

  if (this->status_config_valid_) {
    func(&this->status_config_);
  }
}

StatusFrameHeaterResponse *TrumaiNetBoxApp::update_heater_prepare() {
  // An update is currently going on.
  if (this->update_status_heater_prepared_ || this->update_status_heater_stale_) {
    return &this->update_status_heater_;
  }

  // prepare status heater response
  this->update_status_heater_ = {};
  this->update_status_heater_.target_temp_room = this->status_heater_.target_temp_room;
  this->update_status_heater_.heating_mode = this->status_heater_.heating_mode;
  this->update_status_heater_.el_power_level_a = this->status_heater_.el_power_level_a;
  this->update_status_heater_.target_temp_water = this->status_heater_.target_temp_water;
  this->update_status_heater_.el_power_level_b = this->status_heater_.el_power_level_b;
  this->update_status_heater_.energy_mix_a = this->status_heater_.energy_mix_a;
  this->update_status_heater_.energy_mix_b = this->status_heater_.energy_mix_b;

  this->update_status_heater_prepared_ = true;
  return &this->update_status_heater_;
}

StatusFrameTimerResponse *TrumaiNetBoxApp::update_timer_prepare() {
  // An update is currently going on.
  if (this->update_status_timer_prepared_ || this->update_status_timer_stale_) {
    return &this->update_status_timer_;
  }

  // prepare status heater response
  this->update_status_timer_ = {};
  this->update_status_timer_.timer_target_temp_room = this->status_timer_.timer_target_temp_room;
  this->update_status_timer_.timer_heating_mode = this->status_timer_.timer_heating_mode;
  this->update_status_timer_.timer_el_power_level_a = this->status_timer_.timer_el_power_level_a;
  this->update_status_timer_.timer_target_temp_water = this->status_timer_.timer_target_temp_water;
  this->update_status_timer_.timer_el_power_level_b = this->status_timer_.timer_el_power_level_b;
  this->update_status_timer_.timer_energy_mix_a = this->status_timer_.timer_energy_mix_a;
  this->update_status_timer_.timer_energy_mix_b = this->status_timer_.timer_energy_mix_b;
  this->update_status_timer_.timer_resp_active = this->status_timer_.timer_active;
  this->update_status_timer_.timer_resp_start_minutes = this->status_timer_.timer_start_minutes;
  this->update_status_timer_.timer_resp_start_hours = this->status_timer_.timer_start_hours;
  this->update_status_timer_.timer_resp_stop_minutes = this->status_timer_.timer_stop_minutes;
  this->update_status_timer_.timer_resp_stop_hours = this->status_timer_.timer_stop_hours;

  this->update_status_timer_prepared_ = true;
  return &this->update_status_timer_;
}

bool TrumaiNetBoxApp::answer_lin_order_(const u_int8_t pid) {
  // Alive message
  if (pid == LIN_PID_TRUMA_INET_BOX) {
    std::array<u_int8_t, 8> response = this->lin_empty_response_;

    if (this->updates_to_send_.empty() && !this->has_update_to_submit_()) {
      response[0] = 0xFE;
    }
    this->write_lin_answer_(response.data(), sizeof(response));
    return true;
  }
  return LinBusProtocol::answer_lin_order_(pid);
}

bool TrumaiNetBoxApp::lin_read_field_by_identifier_(u_int8_t identifier, std::array<u_int8_t, 5> *response) {
  this->device_registered_ = esp_timer_get_time();
  if (identifier == 0x00 /* LIN Product Identification */) {
    auto lin_identifier = this->lin_identifier();
    (*response)[0] = lin_identifier[0];
    (*response)[1] = lin_identifier[1];
    (*response)[2] = lin_identifier[2];
    (*response)[3] = lin_identifier[3];
    (*response)[4] = 0x01;  // Hardware revision
    return true;
  } else if (identifier == 0x20 /* Product details to display in CP plus */) {
    auto lin_identifier = this->lin_identifier();
    // Only the first three parts are used.
    (*response)[0] = lin_identifier[0];
    (*response)[1] = lin_identifier[1];
    (*response)[2] = lin_identifier[2];
    // (*response)[3] = // unkown
    // (*response)[4] = // unkown
    return true;
  } else if (identifier == 0x22 /* unkown usage */) {
    // Init is failing if missing
    // Data can be anything?
    return true;
  }
  return false;
}

const u_int8_t *TrumaiNetBoxApp::lin_multiframe_recieved(const u_int8_t *message, const u_int8_t message_len,
                                                         u_int8_t *return_len) {
  static u_int8_t response[48] = {};
  // Validate message prefix.
  if (message_len < truma_message_header.size()) {
    return nullptr;
  }
  for (u_int8_t i = 1; i < truma_message_header.size(); i++) {
    if (message[i] != truma_message_header[i]) {
      return nullptr;
    }
  }

  if (message[0] == LIN_SID_READ_STATE_BUFFER) {
    // Example: BA.00.1F.00.1E.00.00.22.FF.FF.FF (11)
    memset(response, 0, sizeof(response));
    auto response_frame = reinterpret_cast<StatusFrame *>(response);

    // The order must match with the method 'has_update_to_submit_'.
    if (this->init_recieved_ == 0) {
      status_frame_create_init(response_frame, return_len, this->message_counter++);
      return response;
    }
    if (this->update_status_heater_unsubmitted_) {
      status_frame_create_update_heater(
          response_frame, return_len, this->message_counter++, this->update_status_heater_.target_temp_room,
          this->update_status_heater_.target_temp_water, this->update_status_heater_.heating_mode,
          this->update_status_heater_.energy_mix_a, this->update_status_heater_.el_power_level_a);

      this->update_time_ = 0;
      this->update_status_heater_prepared_ = false;
      this->update_status_heater_unsubmitted_ = false;
      this->update_status_heater_stale_ = true;
      return response;
    }
    if (this->update_status_timer_unsubmitted_) {
      status_frame_create_update_timer(
          response_frame, return_len, this->message_counter++, this->update_status_timer_.timer_resp_active,
          this->update_status_timer_.timer_resp_start_hours, this->update_status_timer_.timer_resp_start_minutes,
          this->update_status_timer_.timer_resp_stop_hours, this->update_status_timer_.timer_resp_stop_minutes,
          this->update_status_timer_.timer_target_temp_room, this->update_status_timer_.timer_target_temp_water,
          this->update_status_timer_.timer_heating_mode, this->update_status_timer_.timer_energy_mix_a,
          this->update_status_timer_.timer_el_power_level_a);

      this->update_time_ = 0;
      this->update_status_timer_prepared_ = false;
      this->update_status_timer_unsubmitted_ = false;
      this->update_status_timer_stale_ = true;
      return response;
    }
    if (this->update_status_clock_unsubmitted_) {
      // read time live
      auto now = this->time_->now();

      status_frame_create_update_clock(response_frame, return_len, this->message_counter++, now.hour, now.minute,
                                       now.second, this->status_clock_.clock_mode);

      this->update_status_clock_unsubmitted_ = false;
      return response;
    }
  }

  if (message_len < sizeof(StatusFrame) && message[0] == LIN_SID_FIll_STATE_BUFFFER) {
    return nullptr;
  }

  auto statusFrame = reinterpret_cast<const StatusFrame *>(message);
  auto header = &statusFrame->inner.genericHeader;
  // Validate Truma frame checksum
  if (header->checksum != data_checksum(&statusFrame->raw[10], sizeof(StatusFrame) - 10, (0xFF - header->checksum)) ||
      header->header_2 != 'T' || header->header_3 != 0x01) {
    ESP_LOGE(TAG, "Truma checksum fail.");
    return nullptr;
  }

  // create acknowledge response.
  response[0] = (header->service_identifier | LIN_SID_RESPONSE);
  (*return_len) = 1;

  if (header->message_type == STATUS_FRAME_HEATER && header->message_length == sizeof(StatusFrameHeater)) {
    ESP_LOGI(TAG, "StatusFrameHeater");
    // Example:
    // SID<---------PREAMBLE --------->|<---MSG HEAD --->|tRoom|mo|  |elecA|tWate|elecB|mi|mi|cWate|cRoom|st|err  |  |
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.14.33.00.12.00.00.00.00.00.00.00.00.00.00.01.01.CC.0B.6C.0B.00.00.00.00
    this->status_heater_ = statusFrame->inner.heater;
    this->status_heater_valid_ = true;
    this->status_heater_updated_ = true;

    this->update_status_heater_stale_ = false;
    return response;
  } else if (header->message_type == STATUS_FRAME_TIMER && header->message_length == sizeof(StatusFrameTimer)) {
    ESP_LOGI(TAG, "StatusFrameTimer");
    // EXAMPLE:
    // SID<---------PREAMBLE --------->|<---MSG HEAD --->|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.18.3D.00.1D.18.0B.01.00.00.00.00.00.00.00.01.01.00.00.00.00.00.00.00.01.00.08.00.09
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.18.3D.00.13.18.0B.0B.00.00.00.00.00.00.00.01.01.00.00.00.00.00.00.00.01.00.08.00.09
    this->status_timer_ = statusFrame->inner.timer;
    this->status_timer_valid_ = true;
    this->status_timer_updated_ = true;

    this->update_status_timer_stale_ = false;

    ESP_LOGV(TAG, "StatusFrameTimer target_temp_room: %f target_temp_water: %f %u:%u -> %u:%u %s",
             temp_code_to_decimal(this->status_timer_.timer_target_temp_room),
             temp_code_to_decimal(this->status_timer_.timer_target_temp_water), this->status_timer_.timer_start_hours,
             this->status_timer_.timer_start_minutes, this->status_timer_.timer_stop_hours,
             this->status_timer_.timer_stop_minutes, ((u_int8_t) this->status_timer_.timer_active ? " ON" : " OFF"));

    return response;
  } else if (header->message_type == STATUS_FRAME_RESPONSE_ACK &&
             header->message_length == sizeof(StatusFrameResponseAck)) {
    // Example:
    // SID<---------PREAMBLE --------->|<---MSG HEAD --->|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.02.0D.01.98.02.00
    auto data = statusFrame->inner.responseAck;

    if (data.error_code != ResponseAckResult::RESPONSE_ACK_RESULT_OKAY) {
      ESP_LOGW(TAG, "StatusFrameResponseAck");
    } else {
      ESP_LOGI(TAG, "StatusFrameResponseAck");
    }
    ESP_LOGV(TAG, "StatusFrameResponseAck %02x %s %02x", statusFrame->inner.genericHeader.command_counter,
             data.error_code == ResponseAckResult::RESPONSE_ACK_RESULT_OKAY ? " OKAY " : " FAILED ",
             (u_int8_t) data.error_code);

    if (data.error_code != ResponseAckResult::RESPONSE_ACK_RESULT_OKAY) {
      // I tried to update something and it failed. Read current state again to validate and hold any updates for now.
      this->lin_reset_device();
    }

    return response;
  } else if (header->message_type == STATUS_FRAME_CLOCK && header->message_length == sizeof(StatusFrameClock)) {
    ESP_LOGI(TAG, "StatusFrameClock");
    // Example:
    // SID<---------PREAMBLE --------->|<---MSG HEAD --->|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.15.00.5B.0D.20.00.01.01.00.00.01.00.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.15.00.71.16.00.00.01.01.00.00.02.00.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.15.00.2B.16.1F.28.01.01.00.00.01.00.00
    this->status_clock_ = statusFrame->inner.clock;
    this->status_clock_valid_ = true;
    this->status_clock_updated_ = true;

    ESP_LOGV(TAG, "StatusFrameClock %02d:%02d:%02d", this->status_clock_.clock_hour, this->status_clock_.clock_minute,
             this->status_clock_.clock_second);

    return response;
  } else if (header->message_type == STAUTS_FRAME_CONFIG && header->message_length == sizeof(StatusFrameConfig)) {
    ESP_LOGI(TAG, "StatusFrameConfig");
    // Example:
    // SID<---------PREAMBLE --------->|<---MSG HEAD --->|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.17.00.0F.06.01.B4.0A.AA.0A.00.00.00.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.17.00.41.06.01.B4.0A.78.0A.00.00.00.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.17.00.0F.06.01.B4.0A.AA.0A.00.00.00.00
    this->status_config_ = statusFrame->inner.config;
    this->status_config_valid_ = true;
    this->status_config_updated_ = true;

    ESP_LOGV(TAG, "StatusFrameConfig Offset: %d", offset_code_to_decimal(this->status_config_.temp_offset));

    return response;
  } else if (header->message_type == STATUS_FRAME_DEVICES && header->message_length == sizeof(StatusFrameDevice)) {
    ESP_LOGI(TAG, "StatusFrameDevice");
    // This message is special. I recieve one response per registered (at CP plus) device.
    // Example:
    // SID<---------PREAMBLE --------->|<---MSG HEAD --->|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0C.0B.00.79.02.00.01.00.50.00.00.04.03.02.AD.10 - C4.03.02 0050.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0C.0B.00.27.02.01.01.00.40.03.22.02.00.01.00.00 - H2.00.01 0340.22
    auto device = statusFrame->inner.device;

    this->init_recieved_ = esp_timer_get_time();

    ESP_LOGV(TAG, "StatusFrameDevice %d/%d - %d.%02d.%02d %04x.%02x", device.device_id + 1, device.device_count,
             device.software_revision[0], device.software_revision[1], device.software_revision[2],
             device.hardware_revision_major, device.hardware_revision_minor);

    return response;
  } else {
    ESP_LOGW(TAG, "Unkown message type %02x", header->message_type);
  }
  (*return_len) = 1;
  return nullptr;
}

bool TrumaiNetBoxApp::has_update_to_submit_() {
  if (this->init_requested_ == 0) {
    this->init_requested_ = esp_timer_get_time();
    ESP_LOGV(TAG, "Requesting initial data.");
    return true;
  } else if (this->init_recieved_ == 0) {
    auto init_wait_time = esp_timer_get_time() - this->init_requested_;
    // it has been 5 seconds and i am still awaiting the init data.
    if (init_wait_time > 1000 * 1000 * 5) {
      ESP_LOGV(TAG, "Requesting initial data again.");
      this->init_requested_ = esp_timer_get_time();
      return true;
    }
  } else if (this->update_status_heater_unsubmitted_ || this->update_status_timer_unsubmitted_ ||
             this->update_status_clock_unsubmitted_) {
    if (this->update_time_ == 0) {
      ESP_LOGV(TAG, "Notify CP Plus I got updates.");
      this->update_time_ = esp_timer_get_time();
      return true;
    }
    auto update_wait_time = esp_timer_get_time() - this->update_time_;
    if (update_wait_time > 1000 * 1000 * 5) {
      ESP_LOGV(TAG, "Notify CP Plus again I still got updates.");
      this->update_time_ = esp_timer_get_time();
      return true;
    }
  }
  return false;
}

}  // namespace truma_inetbox
}  // namespace esphome