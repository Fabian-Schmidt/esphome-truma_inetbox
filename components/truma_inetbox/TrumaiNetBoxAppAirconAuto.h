#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppAirconAuto
    : public TrumaStausFrameResponseStorage<StatusFrameAirconAuto, StatusFrameAirconAutoResponse> {
 public:
  StatusFrameAirconAutoResponse *update_prepare() override;
};

}  // namespace truma_inetbox
}  // namespace esphome