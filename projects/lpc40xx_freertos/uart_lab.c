
#include "uart_lab.h"
#include "gpio.h"
#include "lpc40xx.h"

#include "FreeRTOS.h"
#include "lpc_peripherals.h"
#include "queue.h"

static LPC_UART_TypeDef *LPC_UART_x = LPC_UART2; // Initialized to avoid warning

#if defined(LAB_06_P2) || defined(LAB_06_P3)
static QueueHandle_t uart_rx_queue;

static void uart_ch2_rx_interrupt(void) {
  uint8_t iir_int_stat_bit = (1 << 0);
  uint8_t iir_int_id_bit_mask = (0b111 << 1);
  uint8_t rx_interrupt_id = 0;
  uint8_t lsr_rdr_bit = (1 << 0);

  if (!(LPC_UART2->IIR & iir_int_stat_bit)) {
    rx_interrupt_id = (LPC_UART2->IIR & iir_int_id_bit_mask);
    rx_interrupt_id = (rx_interrupt_id >> 1);
    if (rx_interrupt_id == 2 && (LPC_UART2->LSR & lsr_rdr_bit)) {
      const char byte = LPC_UART2->RBR;
      xQueueSendFromISR(uart_rx_queue, &byte, NULL);
    }
  } else {
    fprintf(stderr, "UART Interrupt, Unknown Channel Configuration : %x\n", rx_interrupt_id);
  }
}
static void uart_ch3_rx_interrupt(void) {
  uint8_t iir_int_stat_bit = (1 << 0);
  uint8_t iir_int_id_bit_mask = (0b111 << 1);
  uint8_t rx_interrupt_id = 0;
  uint8_t lsr_rdr_bit = (1 << 0);

  if (!(LPC_UART3->IIR & iir_int_stat_bit)) {
    rx_interrupt_id = (LPC_UART3->IIR & iir_int_id_bit_mask);
    rx_interrupt_id = (rx_interrupt_id >> 1);
    if (rx_interrupt_id == 2 && (LPC_UART3->LSR & lsr_rdr_bit)) {
      const char byte = LPC_UART3->RBR;
      xQueueSendFromISR(uart_rx_queue, &byte, NULL);
    }
  } else {
    fprintf(stderr, "UART Interrupt, Unknown Channel Configuration : %x\n", rx_interrupt_id);
  }
}

void uart__enable_recieve_interrupt(uart_number_e uart_number) {
  LPC_UART_TypeDef *lpc_uart_x;
  uint8_t ier_rda_int_enable_bit = (1 << 0);
  if (uart_number == UART_2) {
    lpc_uart_x = LPC_UART2;
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART2, uart_ch2_rx_interrupt, "UART Interrupt");
  } else {
    lpc_uart_x = LPC_UART3;
    lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__UART3, uart_ch3_rx_interrupt, "UART Interrupt");
  }
  lpc_uart_x->IER |= ier_rda_int_enable_bit;
  uart_rx_queue = xQueueCreate(10, sizeof(char));
  fprintf(stderr, "Created Queue\n");
}

bool uart_lab__get_char_from_queue(char *input_byte, uint32_t timeout) {
  return xQueueReceive(uart_rx_queue, input_byte, timeout);
}
#endif

//_______________________________________
// The functions below are basic UART functions and used in all the parts of this assignment
//_______________________________________
void uart_lab__init(uart_number_e uart, uint32_t peripheral_clock, uint32_t baud_rate) {

  const uint32_t uart_2_power_bit = (1 << 24);
  const uint32_t uart_3_power_bit = (1 << 25);

  const uint8_t lcr_dlab_bit = (1 << 7);    // DLAB bit to enable Divisor Latches
  const uint8_t lcr_8bit_bit = (0b11 << 0); // Word Length : 8 bits

  const uint32_t fdr_mul_val_1 = (1 << 4);
  const uint16_t divider_16_bit = peripheral_clock / (16 * baud_rate);

  switch (uart) {
  case UART_2:
    LPC_UART_x = LPC_UART2;
    LPC_SC->PCONP |= uart_2_power_bit;
    gpio__construct_with_function(GPIO__PORT_2, 8, GPIO__FUNCTION_2);
    gpio__construct_with_function(GPIO__PORT_2, 9, GPIO__FUNCTION_2);
    break;

  case UART_3:
    LPC_UART_x = LPC_UART3;
    LPC_SC->PCONP |= uart_3_power_bit;
    gpio__construct_with_function(GPIO__PORT_4, 28, GPIO__FUNCTION_2);
    gpio__construct_with_function(GPIO__PORT_4, 29, GPIO__FUNCTION_2);
    break;

  default:
    fprintf(stderr, "UART select Error\n");
  }

  LPC_UART_x->LCR = lcr_dlab_bit;
  LPC_UART_x->DLM = (divider_16_bit >> 8) & 0xff;
  LPC_UART_x->DLL = (divider_16_bit >> 0) & 0xff;
  LPC_UART_x->FDR = fdr_mul_val_1;
  LPC_UART_x->LCR = lcr_8bit_bit;
}

bool uart_lab__polled_get(uart_number_e uart, char *input_byte) {
  bool status = false;
  uint8_t lsr_rdr_bit = (1 << 0);
  if (uart == UART_2)
    LPC_UART_x = LPC_UART2;
  if (uart == UART_3)
    LPC_UART_x = LPC_UART3;

  while (!(LPC_UART_x->LSR & lsr_rdr_bit))
    ;
  *input_byte = LPC_UART_x->RBR;
  return status = true;
}

bool uart_lab__polled_put(uart_number_e uart, char output_byte) {
  bool status = false;
  uint8_t lsr_thre_bit = (1 << 5);
  if (uart == UART_2)
    LPC_UART_x = LPC_UART2;
  if (uart == UART_3)
    LPC_UART_x = LPC_UART3;

  while (!(LPC_UART_x->LSR & lsr_thre_bit))
    ;
  LPC_UART_x->THR = output_byte;
  return status = true;
}
