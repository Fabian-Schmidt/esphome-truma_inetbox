#include "TrumaiNetBoxApp.h"
#include "TrumaStatusFrameBuilder.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxApp";

TrumaiNetBoxApp::TrumaiNetBoxApp() {
  this->airconAuto_.set_parent(this);
  this->airconManual_.set_parent(this);
  this->clock_.set_parent(this);
  // this->config_.set_parent(this);
  this->heater_.set_parent(this);
  this->timer_.set_parent(this);
}

void TrumaiNetBoxApp::update() {
  // Call listeners in after method 'lin_multiframe_recieved' call.
  // Because 'lin_multiframe_recieved' is time critical an all these sensors can take some time.

  // Run through callbacks
  this->airconAuto_.update();
  this->airconManual_.update();
  this->clock_.update();
  this->config_.update();
  this->heater_.update();
  this->timer_.update();

  LinBusProtocol::update();

#ifdef USE_TIME
  // Update time of CP Plus automatically when
  // - Time component configured
  // - Update was not done
  // - 30 seconds after init data recieved
  if (this->time_ != nullptr && !this->update_status_clock_done && this->init_recieved_ > 0) {
    if (micros() > ((30 * 1000 * 1000) + this->init_recieved_ /* 30 seconds after init recieved */)) {
      this->update_status_clock_done = true;
      this->clock_.action_write_time();
    }
  }
#endif  // USE_TIME
}

const std::array<uint8_t, 4> TrumaiNetBoxApp::lin_identifier() {
  // Supplier Id: 0x4617 - Truma (Phone: +49 (0)89 4617-0)
  // Unknown:
  // 17.46.01.03 - old Combi model
  // 17.46.10.03 - Unknown more comms required for init.
  // 17.46.20.03 - Unknown more comms required for init.
  // Heater:
  // 17.46.40.03 - H2.00.01 - 0340.xx Combi 4/6
  // Aircon:
  // 17.46.00.0C - A23.70.0 - 0C00.xx (with light option: OFF/1..5)
  // 17.46.01.0C - A23.70.0 - 0C01.xx
  // 17.46.02.0C
  // 17.46.03.0C
  // 17.46.04.0C - A23.70.0 - 0C04.xx (with light option: OFF/1..5)
  // 17.46.05.0C - A23.70.0 - 0C05.xx
  // 17.46.06.0C - A23.70.0 - 0C06.xx (with light option: OFF/1..5)
  // 17.46.07.0C - A23.70.0 - 0C07.xx (with light option: OFF/1..5)
  // iNet Box:
  // 17.46.00.1F - T23.70.0 - 1F00.xx iNet Box
  return {0x17 /*Supplied Id*/, 0x46 /*Supplied Id*/, 0x00 /*Function Id*/, 0x1F /*Function Id*/};
  // 17.46.30.03 - Alde Compact 3020 HE
  // DE.41.01.01 - Alde-Paneel 3020 113
  // DE.41.02.01
}

const std::array<u_int8_t, 3> TrumaiNetBoxApp::lin_identifier_version() { return {0x02, 0x0E, 0x00}; }

void TrumaiNetBoxApp::lin_heartbeat() { this->device_registered_ = micros(); }

void TrumaiNetBoxApp::lin_reset_device() {
  LinBusProtocol::lin_reset_device();
  this->device_registered_ = micros();
  this->init_recieved_ = 0;
  this->init_state_ = 0;
  this->alde_device_ = false;

  this->airconAuto_.reset();
  this->airconManual_.reset();
  this->clock_.reset();
  this->config_.reset();
  this->heater_.reset();
  this->timer_.reset();

  this->update_time_ = 0;
}

bool TrumaiNetBoxApp::answer_lin_order_(const u_int8_t pid) {
  // Alive message
  if (pid == LIN_PID_TRUMA_INET_BOX) {
    std::array<u_int8_t, 8> response = this->lin_empty_response_;

    if (this->updates_to_send_.empty() && !this->has_update_to_submit_()) {
      response[0] = 0xFE;
    }
    this->write_lin_answer_(response.data(), (u_int8_t) sizeof(response));
    return true;
  }
  return LinBusProtocol::answer_lin_order_(pid);
}

bool TrumaiNetBoxApp::lin_read_field_by_identifier_(u_int8_t identifier, std::array<u_int8_t, 5> *response) {
  if (identifier == 0x00 /* LIN Product Identification */) {
    auto lin_identifier = this->lin_identifier();
    (*response)[0] = lin_identifier[0];
    (*response)[1] = lin_identifier[1];
    (*response)[2] = lin_identifier[2];
    (*response)[3] = lin_identifier[3];
    (*response)[4] = 0x01;  // Variant
    return true;
  } else if (identifier == 0x20 /* Product details to display in CP plus */) {
    auto lin_identifier_version = this->lin_identifier_version();
    // Only the first three parts are displayed.
    (*response)[0] = lin_identifier_version[0];
    (*response)[1] = lin_identifier_version[1];
    (*response)[2] = lin_identifier_version[2];
    (*response)[3] = 0xFF;  // Empty. Package length by INet Box is 2 shorter.
    (*response)[4] = 0xFF;  // Empty. Package length by INet Box is 2 shorter.
    return true;
  } else if (identifier == 0x22 /* unknown usage */) {
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
    ESP_LOGE(TAG, "Message header too short.");
    return nullptr;
  }
  for (u_int8_t i = 1; i < truma_message_header.size() - 3; i++) {
    if (i == 4) {
      // Ignore 5.byte
    } else if (message[i] != truma_message_header[i]) {
      ESP_LOGE(TAG, "Message header incorrect.");
      return nullptr;
    }
  }

  if (message[0] == LIN_SID_READ_STATE_BUFFER) {
    // Example: BA.00.1F.00.1E.00.00.22.FF.FF.FF (11)
    // Alde ex: BA.00.1F.00.1A.00.00.22.FF.FF.FF (11)
    memset(response, 0, sizeof(response));
    auto response_frame = reinterpret_cast<StatusFrame *>(response);

    // if (this->DEBUG_SUBMIT) {
    //   ESP_LOGD(TAG, "Requested read: DEBUG");

    //   status_frame_create_empty(response_frame, STATUS_FRAME_ALDE_HEATER_DAY_RESPONSE,
    //   sizeof(StatusFrameAldeHeaterDay),
    //                             this->message_counter++);

    //   response_frame->aldeHeaterDay.target_temp_room = decimal_to_temp((float) this->DEBUG_VALUE);

    //   status_frame_calculate_checksum(response_frame);
    //   (*return_len) = sizeof(StatusFrameHeader) + sizeof(StatusFrameAldeHeaterDay);
    //   this->update_time_ = 0;

    //   this->DEBUG_SUBMIT = false;
    //   return response;
    // }

    // The order must match with the method 'has_update_to_submit_'.
    if (this->init_recieved_ == 0) {
      // message[4] is length.
      if (message[4] == 0x1A) {
        // ALDE init
        // if (this->init_state_debug_ == 0) {
        //   this->init_state_++;
        //   if (this->init_state_ == 0x64) {
        //     this->init_state_ = 0;
        //   }
        // this->init_state_debug_ = 1;
        // Preinit send x25 long empty (xFF) package
        // Or is this response when I am asked for an update without signaling that I have one?
        ESP_LOGD(TAG, "Requested read: Flush empty response");
        this->message_counter = 0x00;
        status_frame_create_null(response_frame, return_len);
        return response;
        // } else {
        //   // DEBUG: try all possible status frame message_type as init.
        //   this->init_state_debug_ = 0;
        //   status_frame_create_init_debug(response_frame, this->init_state_, return_len, this->message_counter++);
        //   return response;
        // }
      } else {
        // TRUMA init
        ESP_LOGD(TAG, "Requested read: Sending init");
        status_frame_create_init(response_frame, return_len, this->message_counter++);
        return response;
      }

    } else if (this->heater_.has_update()) {
      ESP_LOGD(TAG, "Requested read: Sending heater update");
      this->heater_.create_update_data(response_frame, return_len, this->message_counter++);
      this->update_time_ = 0;
      return response;
    } else if (this->timer_.has_update()) {
      ESP_LOGD(TAG, "Requested read: Sending timer update");
      this->timer_.create_update_data(response_frame, return_len, this->message_counter++);
      this->update_time_ = 0;
      return response;
    } else if (this->airconManual_.has_update()) {
      ESP_LOGD(TAG, "Requested read: Sending aircon manual update");
      this->airconManual_.create_update_data(response_frame, return_len, this->message_counter++);
      this->update_time_ = 0;
      return response;
    } else if (this->airconAuto_.has_update()) {
      ESP_LOGD(TAG, "Requested read: Sending aircon auto update");
      this->airconAuto_.create_update_data(response_frame, return_len, this->message_counter++);
      this->update_time_ = 0;
      return response;
#ifdef USE_TIME
    } else if (this->clock_.has_update()) {
      ESP_LOGD(TAG, "Requested read: Sending clock update");
      this->clock_.create_update_data(response_frame, return_len, this->message_counter++);
      this->update_time_ = 0;
      return response;
#endif  // USE_TIME
    } else {
      ESP_LOGW(TAG, "Requested read: CP Plus asks for an update, but I have none.");
    }
  } else if (message[0] != LIN_SID_FIll_STATE_BUFFFER) {
    ESP_LOGE(TAG, "Unknown SID %02X .", message[0]);
    return nullptr;
  }

  if (message_len < sizeof(StatusFrameHeader)) {
    ESP_LOGE(TAG, "Message too short.");
    return nullptr;
  }

  auto statusFrame = reinterpret_cast<const StatusFrame *>(message);
  auto header = &statusFrame->genericHeader;
  // Validate Truma frame checksum
  if (header->checksum != data_checksum(&statusFrame->raw[10], message_len - 10, (0xFF - header->checksum)) ||
      header->header_2 != 'T' || header->header_3 != 0x01) {
    // auto crc = data_checksum(&statusFrame->raw[10], message_len - 10, (0xFF - header->checksum));
    // ESP_LOGE(TAG, "Truma checksum fail. - %02X %02X %02X %02X ", header->checksum, crc, header->header_2,
    //          header->header_3);
    // ESP_LOGE(TAG, "%s", format_hex_pretty(&statusFrame->raw[10], message_len - 10).c_str());
    ESP_LOGE(TAG, "Truma checksum fail.");
    return nullptr;
  }

  // create acknowledge response.
  response[0] = (header->service_identifier | LIN_SID_RESPONSE);
  (*return_len) = 1;

  if (header->message_type == STATUS_FRAME_HEATER && header->message_length == sizeof(StatusFrameHeater)) {
    ESP_LOGI(TAG, "StatusFrameHeater");
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|tRoom|mo|  |elecA|tWate|elecB|mi|mi|cWate|cRoom|st|err  |  |
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.14.33.00.12.00.00.00.00.00.00.00.00.00.00.01.01.CC.0B.6C.0B.00.00.00.00
    this->heater_.set_status(statusFrame->heater);
    return response;
  } else if (header->message_type == STATUS_FRAME_AIRCON_MANUAL &&
             header->message_length == sizeof(StatusFrameAirconManual)) {
    ESP_LOGI(TAG, "StatusFrameAirconManual");
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|
    // - ac temps form 16 - 30 C in +2 steps
    // - activation and deactivation of the ac ventilating
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.AA.00.00.71.01.00.00.00.00.86.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.A5.00.00.71.01.00.00.00.00.8B.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.A5.00.00.71.01.00.00.00.00.8B.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.4B.05.00.71.01.4A.0B.00.00.8B.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.37.05.00.71.01.5E.0B.00.00.8B.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.24.05.00.71.01.72.0B.00.00.8A.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.13.05.00.71.01.86.0B.00.00.87.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.FC.05.00.71.01.9A.0B.00.00.89.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.E8.05.00.71.01.AE.0B.00.00.89.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.D5.05.00.71.01.C2.0B.00.00.88.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.C1.05.00.71.01.D6.0B.00.00.88.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.A7.00.00.71.01.00.00.00.00.89.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.C2.04.00.71.01.D6.0B.00.00.88.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.13.04.00.71.01.86.0B.00.00.88.0B.00.00.00.00.00.00.AA.0A
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.35.00.A8.00.00.71.01.00.00.00.00.88.0B.00.00.00.00.00.00.AA.0A
    this->airconManual_.set_status(statusFrame->airconManual);
    return response;
  } else if (header->message_type == STATUS_FRAME_AIRCON_MANUAL_INIT &&
             header->message_length == sizeof(StatusFrameAirconManualInit)) {
    ESP_LOGI(TAG, "StatusFrameAirconManualInit");
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.16.3F.00.E2.00.00.71.01.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00
    return response;
  } else if (header->message_type == STATUS_FRAME_AIRCON_AUTO &&
             header->message_length == sizeof(StatusFrameAirconAuto)) {
    ESP_LOGI(TAG, "StatusFrameAirconAuto");
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.12.37.00.BF.01.00.01.00.00.00.00.00.00.00.00.00.00.00.49.0B.40.0B
    this->airconAuto_.set_status(statusFrame->airconAuto);
    return response;
  } else if (header->message_type == STATUS_FRAME_AIRCON_AUTO_INIT &&
             header->message_length == sizeof(StatusFrameAirconAutoInit)) {
    ESP_LOGI(TAG, "StatusFrameAirconAutoInit");
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.14.41.00.53.01.00.01.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00.00
    return response;
  } else if (header->message_type == STATUS_FRAME_TIMER && header->message_length == sizeof(StatusFrameTimer)) {
    ESP_LOGI(TAG, "StatusFrameTimer");
    // EXAMPLE:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|tRoom|mo|??|elecA|tWate|elecB|mi|mi|<--response-->|??|??|on|start|stop-|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.18.3D.00.1D.18.0B.01.00.00.00.00.00.00.00.01.01.00.00.00.00.00.00.00.01.00.08.00.09
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.18.3D.00.13.18.0B.0B.00.00.00.00.00.00.00.01.01.00.00.00.00.00.00.00.01.00.08.00.09
    this->timer_.set_status(statusFrame->timer);
    return response;

  } else if (header->message_type == STATUS_FRAME_CLOCK && header->message_length == sizeof(StatusFrameClock)) {
    ESP_LOGI(TAG, "StatusFrameClock");
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.15.00.5B.0D.20.00.01.01.00.00.01.00.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.15.00.71.16.00.00.01.01.00.00.02.00.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.15.00.2B.16.1F.28.01.01.00.00.01.00.00
    this->clock_.set_status(statusFrame->clock);
    return response;
  } else if (header->message_type == STAUTS_FRAME_CONFIG && header->message_length == sizeof(StatusFrameConfig)) {
    ESP_LOGI(TAG, "StatusFrameConfig");
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.17.00.0F.06.01.B4.0A.AA.0A.00.00.00.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.17.00.41.06.01.B4.0A.78.0A.00.00.00.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0A.17.00.0F.06.01.B4.0A.AA.0A.00.00.00.00
    this->config_.set_status(statusFrame->config);
    return response;
  } else if (header->message_type == STATUS_FRAME_RESPONSE_ACK &&
             header->message_length == sizeof(StatusFrameResponseAck)) {
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.02.0D.01.98.02.00
    auto data = statusFrame->responseAck;

    if (data.error_code != ResponseAckResult::RESPONSE_ACK_RESULT_OKAY) {
      ESP_LOGW(TAG, "StatusFrameResponseAck");
    } else {
      ESP_LOGI(TAG, "StatusFrameResponseAck");
    }
    ESP_LOGD(TAG, "StatusFrameResponseAck %02X %s %02X", statusFrame->genericHeader.command_counter,
             data.error_code == ResponseAckResult::RESPONSE_ACK_RESULT_OKAY ? " OKAY " : " FAILED ",
             (u_int8_t) data.error_code);

    if (data.error_code != ResponseAckResult::RESPONSE_ACK_RESULT_OKAY) {
      // I tried to update something and it failed. Read current state again to validate and hold any updates for now.
      this->lin_reset_device();
    }

    return response;
  } else if ((header->message_type == STATUS_FRAME_DEVICES || header->message_type == STATUS_FRAME_DEVICES_ALDE) &&
             header->message_length == sizeof(StatusFrameDevice)) {
    if (header->message_type == STATUS_FRAME_DEVICES_ALDE) {
      ESP_LOGI(TAG, "StatusFrameDeviceAlde");
      this->alde_device_ = true;
    } else {
      ESP_LOGI(TAG, "StatusFrameDevice");
      this->alde_device_ = false;
    }
    // This message is special. I recieve one response per registered (at CP plus) device.
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|count|st|??|Hardware|Software|??|??
    // Combi4
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0C.0B.00.79.02.00.01.00.50.00.00.04.03.02.AD.10 - C4.03.02 0050.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0C.0B.00.27.02.01.01.00.40.03.22.02.00.01.00.00 - H2.00.01 0340.22
    // VarioHeat Comfort w/o E-Kit
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0C.0B.00.C2.02.00.01.00.51.00.00.05.01.00.66.10 - P5.01.00 0051.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0C.0B.00.64.02.01.01.00.20.06.02.03.00.00.00.00 - H3.00.00 0620.02
    // Combi6DE + Saphir Compact AC
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0C.0B.00.C7.03.00.01.00.50.00.00.04.03.00.60.10
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0C.0B.00.71.03.01.01.00.10.03.02.06.00.02.00.00
    // BB.00.1F.00.1E.00.00.22.FF.FF.FF.54.01.0C.0B.00.7C.03.02.01.00.01.0C.00.01.02.01.00.00
    // ALDE Device
    // BB.00.1F.00.14.00.00.22.FF.FF.FF.54.01.0E.0C.00.58.02.00.01.FF.DE.41.01.00.00.04.10.00 - Alde-Paneel 3020 113
    // BB.00.1F.00.14.00.00.22.FF.FF.FF.54.01.0E.0C.00.C9.02.01.01.FF.DE.41.30.03.00.02.6D.00 - Alde Compact 3020 HE
    auto device = statusFrame->device;

    ESP_LOGD(TAG, "StatusFrameDevice %d/%d - %d.%02d.%02d %04X.%02X (%02X %02X)", device.device_id + 1,
             device.device_count, device.software_revision[0], device.software_revision[1], device.software_revision[2],
             device.hardware_revision_major, device.hardware_revision_minor, device.unknown_2, device.unknown_3);

    const auto truma_device = static_cast<TRUMA_DEVICE>(device.software_revision[0]);
    {
      bool found_unknown_value = false;
      if (device.unknown_1 != 0x00)
        found_unknown_value = true;
      if (truma_device != TRUMA_DEVICE::AIRCON_DEVICE && truma_device != TRUMA_DEVICE::HEATER_COMBI4 &&
          truma_device != TRUMA_DEVICE::HEATER_VARIO && truma_device != TRUMA_DEVICE::CPPLUS_COMBI &&
          truma_device != TRUMA_DEVICE::CPPLUS_VARIO && truma_device != TRUMA_DEVICE::HEATER_COMBI6D)
        found_unknown_value = true;

      if (found_unknown_value)
        ESP_LOGW(TAG, "Unknown information in StatusFrameDevice found. Please report.");
    }

    // first submitted device is CP Plus device (or ALDE paneel)
    const auto is_master_device = device.device_id == 0;

    if (!is_master_device) {
      // Assumption first device is Heater
      if (device.device_id == 1) {
        this->heater_device_ = truma_device;
      }
      // Assumption second device is Aircon
      if (device.device_id == 2) {
        // TODO: Detect aircon because they have a 0x0C as Hardware id?
        this->aircon_device_ = TRUMA_DEVICE::AIRCON_DEVICE;
      }
    }

    if (device.device_id + 1 == device.device_count) {
      // Got all device messages.
      this->init_recieved_ = micros();
    }

    return response;
    //
    // ALDE SUPPORT START
    //
  } else if (header->message_type == STATUS_FRAME_ALDE_STATUS &&
             header->message_length == sizeof(StatusFrameAldeStatus)) {
    ESP_LOGI(TAG, "StatusFrameAldeStatus");
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|
    // BB.00.1F.00.22.00.00.22.FF.FF.FF.54.01.1C.51.00.86.01.00.5E.0B.FE.FF.00.00.37.01.00.64.FF.FF.FF.12.71.0B.FE.FF.FA.0A.00.1E.FF.FF.FF.FF
    // (45)
    // BB.00.1F.00.22.00.00.22.FF.FF.FF.54.01.1C.51.00.35.01.00.59.0B.FE.FF.00.00.37.01.00.64.FF.FF.FF.1C.81.0B.FE.FF.36.0B.00.1E.FF.FF.FF.FF
    // (45)
    // BB.00.1F.00.22.00.00.22.FF.FF.FF.54.01.1C.51.00.34.01.00.59.0B.FE.FF.00.00.37.01.00.64.FF.FF.FF.1C.82.0B.FE.FF.36.0B.00.1E.FF.FF.FF.FF
    // (45)
    // BB.00.1F.00.22.00.00.22.FF.FF.FF.54.01.1C.51.00.30.01.00.68.0B.FE.FF.00.00.37.00.00.64.FF.FF.FF.2A.7E.0B.FE.FF.22.0B.00.1E.FF.FF.FF.FF
    // (45) - Nachtmodus Temperatur auf 18 Grad geändert
    // BB.00.1F.00.22.00.00.22.FF.FF.FF.54.01.1C.51.00.39.01.00.5E.0B.FE.FF.00.00.37.00.00.64.FF.FF.FF.2B.7E.0B.FE.FF.22.0B.00.1E.FF.FF.FF.FF
    // (45) - Nachtmodus Temperatur auf 19 Grad geändert
    // BB.00.1F.00.22.00.00.22.FF.FF.FF.54.01.1C.51.00.3B.01.00.5E.0B.FE.FF.00.00.37.00.00.64.FF.FF.FF.29.7E.0B.FE.FF.22.0B.00.1E.FF.FF.FF.FF
    // (45) - Nachtmodus eingeschaltet
    // BB.00.1F.00.22.00.00.22.FF.FF.FF.54.01.1C.51.00.50.01.00.4A.0B.FE.FF.00.00.37.00.00.64.FF.FF.FF.28.7E.0B.FE.FF.22.0B.00.1E.FF.FF.FF.FF
    // (45) - Nachtmodus ausgeschaltet
    // BB.00.1F.00.22.00.00.22.FF.FF.FF.54.01.1C.51.00.32.01.00.63.0B.FE.FF.00.00.37.00.00.64.FF.FF.FF.22.7F.0B.FE.FF.2C.0B.00.1E.FF.FF.FF.FF
    // (45)
    // BB.00.1F.00.22.00.00.22.FF.FF.FF.54.01.1C.51.00.32.01.00.63.0B.FE.FF.00.00.37.01.00.64.FF.FF.FF.21.7F.0B.FE.FF.2C.0B.00.1E.FF.FF.FF.FF
    // (45)
    ESP_LOGD(TAG,
             "StatusFrameAldeStatus target_temp: %f, message_counter: %u, current_temp_inside: %f, "
             "current_temp_outside: %f, el: %u, gas: %s,",
             temp_code_to_decimal(statusFrame->aldeStatus.target_temp_room), statusFrame->aldeStatus.message_counter,
             temp_code_to_decimal(statusFrame->aldeStatus.current_temp_inside),
             temp_code_to_decimal(statusFrame->aldeStatus.current_temp_outside),
             ((u_int8_t) statusFrame->aldeStatus.el_mode) * 100,
             statusFrame->aldeStatus.gas_mode == GasModeAlde::GAS_MODE_ALDE_OFF ? "OFF" : "ON");
    return response;
  } else if (header->message_type == STATUS_FRAME_ALDE_ADDON && header->message_length == sizeof(StatusFrameAldeAddon)) {
    ESP_LOGI(TAG, "StatusFrameAldeAddon");
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|
    //
    return response;
  } else if (header->message_type == STATUS_FRAME_ALDE_HEATER_NIGHT &&
             header->message_length == sizeof(StatusFrameAldeHeaterNight)) {
    ESP_LOGI(TAG, "StatusFrameAldeHeaterNight");
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|
    //
    ESP_LOGD(TAG, "StatusFrameAldeHeaterNight target_temp_room: %f",
             temp_code_to_decimal(statusFrame->aldeHeaterNight.target_temp_room));
    return response;

  } else if (header->message_type == STATUS_FRAME_ALDE_HEATER_DAY &&
             header->message_length == sizeof(StatusFrameAldeHeaterDay)) {
    ESP_LOGI(TAG, "StatusFrameAldeHeaterDay");
    ESP_LOGD(TAG, "StatusFrameAldeHeaterDay target_temp_room: %f",
             temp_code_to_decimal(statusFrame->aldeHeaterDay.target_temp_room));
    // Example:
    // SID<---------PREAMBLE---------->|<---MSG_HEAD---->|
    //
    return response;

    //
    // ALDE SUPPORT END
    //
  } else {
    ESP_LOGW(TAG, "Unknown message type %02X", header->message_type);
  }
  (*return_len) = 0;
  return nullptr;
}

bool TrumaiNetBoxApp::has_update_to_submit_() {
  // No logging in this message!
  // It is called by interrupt. Logging is a blocking operation (especially when Wifi Logging).
  // If logging is necessary use logging queue of LinBusListener class.
  if (this->init_requested_ == 0) {
    // Init request is send 20 seconds after boot.
    if (micros() < 1000 * 1000 * 20 /* seconds */) {
      return false;
    }
    this->init_requested_ = micros();
    // ESP_LOGD(TAG, "Requesting initial data.");
    return true;
  } else if (this->init_recieved_ == 0) {
    auto init_wait_time = micros() - this->init_requested_;
    // it has been 15 seconds and I am still awaiting the init data.
    if (init_wait_time > 1000 * 1000 * 15) {
      // ESP_LOGD(TAG, "Requesting initial data again.");
      this->init_requested_ = micros();
      return true;
    }
  } else if (this->airconAuto_.has_update() || this->airconManual_.has_update() || this->clock_.has_update() ||
             this->heater_.has_update() || this->timer_.has_update() || this->alde_status_.has_update()) {
    if (this->update_time_ == 0) {
      // ESP_LOGD(TAG, "Notify CP Plus I got updates.");
      this->update_time_ = micros();
      return true;
    }
    auto update_wait_time = micros() - this->update_time_;
    if (update_wait_time > 1000 * 1000 * 5) {
      // ESP_LOGD(TAG, "Notify CP Plus again I still got updates.");
      this->update_time_ = micros();
      return true;
    }
  }
  return false;
}

}  // namespace truma_inetbox
}  // namespace esphome