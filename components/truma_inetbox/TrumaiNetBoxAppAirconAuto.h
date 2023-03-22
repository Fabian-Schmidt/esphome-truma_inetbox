#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppAirconAuto
    : public TrumaStausFrameResponseStorage<StatusFrameAirconAuto, StatusFrameAirconAutoResponse> {
 public:
  StatusFrameAirconAutoResponse *update_prepare() override;
  void create_update_data(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter) override;
  void dump_data() const override;
  bool can_update() override;
};

}  // namespace truma_inetbox
}  // namespace esphome