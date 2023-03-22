#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppHeater : public TrumaStausFrameResponseStorage<StatusFrameHeater, StatusFrameHeaterResponse> {
 public:
  StatusFrameHeaterResponse* update_prepare() override;
};

}  // namespace truma_inetbox
}  // namespace esphome