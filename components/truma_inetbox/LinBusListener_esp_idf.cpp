#ifdef USE_ESP32_FRAMEWORK_ESP_IDF
#include "LinBusListener.h"
#include "esphome/core/log.h"
#include "esphome/components/uart/truma_uart_component_esp_idf.h"
#include "esphome/components/uart/uart_component_esp_idf.h"

namespace esphome {
namespace truma_inetbox {

static const char *const TAG = "truma_inetbox.LinBusListener";

void LinBusListener::setup_framework() {
  // uartSetFastReading
  auto uartComp = ((*uart::truma_IDFUARTComponent) this->parent_);

  // Tweak the fifo settings so data is available as soon as the first byte is recieved.
  // If not it will wait either until fifo is filled or a certain time has passed.
  uart_intr_config_t uart_intr;
  uart_intr.intr_enable_mask = 0;  // UART_RXFIFO_FULL_INT_ENA_M | UART_RXFIFO_TOUT_INT_ENA_M | UART_FRM_ERR_INT_ENA_M |
                                   // UART_RXFIFO_OVF_INT_ENA_M | UART_BRK_DET_INT_ENA_M | UART_PARITY_ERR_INT_ENA_M;
  uart_intr.rxfifo_full_thresh =
      1;  // UART_FULL_THRESH_DEFAULT,  //120 default!! aghh! need receive 120 chars before we see them
  uart_intr.rx_timeout_thresh =
      1;  // UART_TOUT_THRESH_DEFAULT,  //10 works well for my short messages I need send/receive
  uart_intr.txfifo_empty_intr_thresh = 10;  // UART_EMPTY_THRESH_DEFAULT
  uart_intr_config(uartComp->get_hw_serial_number(), &uart_intr);
}
}  // namespace truma_inetbox
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF