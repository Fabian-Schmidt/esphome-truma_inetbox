#pragma once

#include "TrumaStausFrameStorage.h"

namespace esphome {
namespace truma_inetbox {

template<typename T, typename TResponse> class TrumaStausFrameResponseStorage : public TrumaStausFrameStorage<T> {
 public:
  void reset();
  bool can_update() { return this->data_valid_; }
  virtual TResponse *update_prepare() = 0;
  void update_submit() { this->update_status_unsubmitted_ = true; }

  // Prepared means `update_status_` was copied from `data_`.
  bool update_status_prepared_ = false;
  // Prepared means an update is already awating fetch from CP plus.
  bool update_status_unsubmitted_ = false;
  // I have submitted my update request to CP plus, but I have not recieved an update with new heater values from CP
  // plus.
  bool update_status_stale_ = false;
  TResponse update_status_;
};

}  // namespace truma_inetbox
}  // namespace esphome