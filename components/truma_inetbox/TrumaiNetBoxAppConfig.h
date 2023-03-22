#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppConfig : public TrumaStausFrameStorage<StatusFrameConfig> {
  void dump_data() const override;
};

}  // namespace truma_inetbox
}  // namespace esphome