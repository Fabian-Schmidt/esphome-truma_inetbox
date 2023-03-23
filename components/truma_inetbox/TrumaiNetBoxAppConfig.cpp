#include "TrumaiNetBoxAppConfig.h"
#include "esphome/core/log.h"
#include "helpers.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.TrumaiNetBoxAppConfig";

void TrumaiNetBoxAppConfig::dump_data() const {
  ESP_LOGD(TAG, "StatusFrameConfig Offset: %.1f", temp_code_to_decimal(this->data_.temp_offset));
}

}  // namespace truma_inetbox
}  // namespace esphome