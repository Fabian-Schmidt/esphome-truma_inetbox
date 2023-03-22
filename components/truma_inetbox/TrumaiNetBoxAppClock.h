#pragma once

#include "TrumaStausFrameResponseStorage.h"
#include "TrumaStructs.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxAppClock : public TrumaStausFrameStorage<StatusFrameClock> {};

}  // namespace truma_inetbox
}  // namespace esphome