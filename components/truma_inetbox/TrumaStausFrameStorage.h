#pragma once

#include "esphome/core/automation.h"

namespace esphome {
namespace truma_inetbox {

template<typename T> class TrumaStausFrameStorage {
 public:
  CallbackManager<void(const T *)> state_callback_{};
  T data_;
  bool data_valid_ = false;
  // Value has changed notify listeners.
  bool data_updated_ = false;

  // bool get_status_valid() { return this->data_valid_; }
  // const T *get_status() { return &this->data_; }
  void update();
  void reset();
};

}  // namespace truma_inetbox
}  // namespace esphome