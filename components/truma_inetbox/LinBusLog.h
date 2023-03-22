#pragma once

#include "esphome/core/log.h"

#define truma_log(_log_msg_) xQueueSend(this->log_queue_, (void *) &_log_msg_, QUEUE_WAIT_DONT_BLOCK);

#define truma_logfromisr(_log_msg_) xQueueSendFromISR(this->log_queue_, (void *) &_log_msg_, QUEUE_WAIT_DONT_BLOCK);

#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERY_VERBOSE
#define TRUMA_LOGVV(_log_msg_) truma_log(_log_msg_)
#define TRUMA_LOGVV_ISR(_log_msg_) truma_logfromisr(_log_msg_)
#else
#define TRUMA_LOGVV(_log_msg_)
#define TRUMA_LOGVV_ISR(_log_msg_)
#endif

#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
#define TRUMA_LOGV(_log_msg_) truma_log(_log_msg_)
#define TRUMA_LOGV_ISR(_log_msg_) truma_logfromisr(_log_msg_)
#else
#define TRUMA_LOGV(_log_msg_)
#define TRUMA_LOGV_ISR(_log_msg_)
#endif

#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_INFO
#define TRUMA_LOGI(_log_msg_) truma_log(_log_msg_)
#define TRUMA_LOGI_ISR(_log_msg_) truma_logfromisr(_log_msg_)
#else
#define TRUMA_LOGI(_log_msg_)
#define TRUMA_LOGI_ISR(_log_msg_)
#endif

#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_WARN
#define TRUMA_LOGW(_log_msg_) truma_log(_log_msg_)
#define TRUMA_LOGW_ISR(_log_msg_) truma_logfromisr(_log_msg_)
#else
#define TRUMA_LOGW(_log_msg_)
#define TRUMA_LOGW_ISR(_log_msg_)
#endif

#if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_ERROR
#define TRUMA_LOGE(_log_msg_) truma_log(_log_msg_)
#define TRUMA_LOGE_ISR(_log_msg_) truma_logfromisr(_log_msg_)
#else
#define TRUMA_LOGE(_log_msg_)
#define TRUMA_LOGE_ISR(_log_msg_)
#endif

enum class QUEUE_LOG_MSG_TYPE {
  UNKNOWN,
  ERROR_LIN_ANSWER_CAN_WRITE_LIN_ANSWER,
  ERROR_LIN_ANSWER_TOO_LONG,
  VERBOSE_LIN_ANSWER_RESPONSE,
  ERROR_CHECK_FOR_LIN_FAULT_DETECTED,
  INFO_CHECK_FOR_LIN_FAULT_FIXED,
  ERROR_READ_LIN_FRAME_UNABLE_TO_ANSWER,
  ERROR_READ_LIN_FRAME_LOST_MSG,
  VV_READ_LIN_FRAME_BREAK_EXPECTED,
  VV_READ_LIN_FRAME_SYNC_EXPECTED,
  WARN_READ_LIN_FRAME_SID_CRC,
  WARN_READ_LIN_FRAME_LINv1_CRC,
  WARN_READ_LIN_FRAME_LINv2_CRC,
  VERBOSE_READ_LIN_FRAME_MSG,
};

// Log messages generated during interrupt are pushed to log queue.
struct QUEUE_LOG_MSG {
  QUEUE_LOG_MSG_TYPE type;
  u_int8_t current_PID;
  u_int8_t data[9];
  u_int8_t len;
#ifdef ESPHOME_LOG_HAS_VERBOSE
  bool current_data_valid;
  bool message_source_know;
  bool message_from_master;
#endif  // ESPHOME_LOG_HAS_VERBOSE
};
