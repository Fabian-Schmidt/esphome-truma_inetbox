#pragma once

#include "TrumaStausFrameStorage.h"
#include "TrumaStructs.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace truma_inetbox {

class TrumaiNetBoxApp;

template<typename T, typename TResponse>
class TrumaStausFrameResponseStorage : public TrumaStausFrameStorage<T>, public Parented<TrumaiNetBoxApp> {
 public:
  void reset() override {
    TrumaStausFrameStorage<T>::reset();
    this->update_status_prepared_ = false;
    this->update_status_unsubmitted_ = false;
    this->update_status_stale_ = false;
  }
  virtual bool can_update() { return this->data_valid_; }
  virtual TResponse *update_prepare() = 0;
  void update_submit() { this->update_status_unsubmitted_ = true; }
  bool has_update() const { return this->update_status_unsubmitted_; }
  void set_status(T val) override {
    TrumaStausFrameStorage<T>::set_status(val);
    this->update_status_stale_ = false;
  };
  virtual void create_update_data(StatusFrame *response, u_int8_t *response_len, u_int8_t command_counter) = 0;

 protected:
  inline void update_submitted() {
    this->update_status_prepared_ = false;
    this->update_status_unsubmitted_ = false;
    this->update_status_stale_ = true;
  }

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