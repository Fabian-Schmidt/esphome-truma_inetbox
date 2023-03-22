#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppTimer : public TrumaStausFrameResponseStorage<StatusFrameTimer, StatusFrameTimerResponse> {
 public:
  StatusFrameTimerResponse *update_prepare() override;
};

}  // namespace truma_inetbox
}  // namespace esphome