#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppAirconManual
    : public TrumaStausFrameResponseStorage<StatusFrameAirconManual, StatusFrameAirconManualResponse> {
 public:
  StatusFrameAirconManualResponse *update_prepare() override;
};

}  // namespace truma_inetbox
}  // namespace esphome