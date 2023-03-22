#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppAirconManual
    : public TrumaStausFrameResponseStorage<StatusFrameAirconManual, StatusFrameAirconManualResponse> {
 public:
  StatusFrameAirconManualResponse *update_prepare() override;
  void create_update_data(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter) override;
  void dump_data() const override;
  bool can_update() override;

  bool action_set_temp(u_int8_t temperature);
};

}  // namespace truma_inetbox
}  // namespace esphome