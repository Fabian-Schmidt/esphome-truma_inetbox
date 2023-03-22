#pragma once

#include "esphome/core/automation.h"

namespace esphome {
namespace truma_inetbox {

template<typename T> class TrumaStausFrameStorage {
 public:
  bool get_status_valid() { return this->data_valid_; };
  const T *get_status() { return &this->data_; };
  virtual void set_status(T val) {
    this->data_ = val;
    this->data_valid_ = true;
    this->data_updated_ = true;
    this->dump_data();
  };
  void update() {
    if (this->data_updated_) {
      this->state_callback_.call(&this->data_);
    }
    this->data_updated_ = false;
  };
  virtual void reset() {
    this->data_valid_ = false;
    this->data_updated_ = false;
  };
  void add_on_message_callback(std::function<void(const T *)> callback) {
    this->state_callback_.add(std::move(callback));
  };
  virtual void dump_data() const = 0;

 protected:
  CallbackManager<void(const T *)> state_callback_{};
  T data_;
  bool data_valid_ = false;
  // Value has changed notify listeners.
  bool data_updated_ = false;
};

}  // namespace truma_inetbox
}  // namespace esphome