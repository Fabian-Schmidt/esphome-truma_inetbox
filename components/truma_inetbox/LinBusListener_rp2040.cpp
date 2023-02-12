#ifdef USE_RP2040
#include "LinBusListener.h"
#include "esphome/core/log.h"
#include "esphome/components/uart/truma_uart_component_rp2040.h"
#include "esphome/components/uart/uart_component_rp2040.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.LinBusListener";

void LinBusListener::setup_framework() {
  // truma_RP2040UartComponent
}
}  // namespace truma_inetbox
}  // namespace esphome

#endif  // USE_RP2040
