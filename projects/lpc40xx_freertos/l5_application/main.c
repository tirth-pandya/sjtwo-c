#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "gpio.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"

#include "uart_lab.h"
#include <stdlib.h>
#include <string.h>

// Select the UART Channel HERE:
//#define UART_CH UART_3
#define UART_CH UART_2


//___________________________
// TASKS for PART 3 HERE:
//___________________________
#ifdef LAB_06_P3

// FOR PART 3 : Select Board Config HERE:
//#define BOARD_AS_SENDER
#define BOARD_AS_RECEIVER

void sender_board_task(void *p) {
  char number_as_string[16] = {0};

  while (1) {
    const int number = rand();
    sprintf(number_as_string, "%i", number);

    for (int i = 0; i <= strlen(number_as_string); i++) {
      uart_lab__polled_put(UART_CH, number_as_string[i]);
      printf("Sent: %c\n", number_as_string[i]);
    }
    printf("Sent: %i over UART to other board\n", number);

    vTaskDelay(3000);
  }
}

void receiver_board_task(void *p) {
  char number_as_string[16] = {0};
  int counter = 0;

  while (true) {
    char byte = 0;
    uart_lab__get_char_from_queue(&byte, portMAX_DELAY);
    printf("Received : %c\n", byte);

    if ('\0' == byte) {
      number_as_string[counter] = '\0';
      counter = 0;
      printf("Received this number from the other board : %s\n", number_as_string);
    } else {
      number_as_string[counter] = byte;
      counter++;
    }
  }
}
#endif
//___________________________

//___________________________
// TASKS for PART 2 HERE:
//___________________________
#ifdef LAB_06_P2
void uart_read_task(void *p) {
  while (1) {
    char data_rx = 0;
    while (uart_lab__get_char_from_queue(&data_rx, 100)) {
      fprintf(stderr, "Interrupt => Rx Data : %x\n", data_rx);
    }
    vTaskDelay(500);
  }
}

void uart_write_task(void *p) {
  char data_tx = 0;
  while (1) {
    while (!(uart_lab__polled_put(UART_CH, data_tx)))
      ;
    fprintf(stderr, "Tx Data : %x\n", data_tx);
    vTaskDelay(500);
    data_tx++;
  }
}
#endif
//___________________________

//___________________________
// TASKS for Part 1 HERE:
//___________________________
#ifdef LAB_06_P1
void uart_read_task(void *p) {
  char data_rx;
  while (1) {
    uart_lab__polled_get(UART_CH, &data_rx);
    vTaskDelay(500);
    printf("Rx Data : %d\n", data_rx);
  }
}

void uart_write_task(void *p) {
  char data_tx = 0;
  while (1) {
    uart_lab__polled_put(UART_CH, data_tx);
    vTaskDelay(500);
    printf("Data sent : %d \n", data_tx);
    data_tx++;
  }
}
#endif
//___________________________

int main(void) {
  uart_lab__init(UART_CH, clock__get_peripheral_clock_hz(), 115200);

#ifdef LAB_06_P3

#ifdef BOARD_AS_SENDER
  xTaskCreate(sender_board_task, "send_data", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#endif
#ifdef BOARD_AS_RECEIVER
  uart__enable_recieve_interrupt(UART_CH);
  xTaskCreate(receiver_board_task, "receive_data", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#endif

#endif

#ifdef LAB_06_P2
  uart__enable_recieve_interrupt(UART_CH);
#endif

#if defined(LAB_06_P1) || defined(LAB_06_P2)
  xTaskCreate(uart_read_task, "uart_read", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(uart_write_task, "uart_write", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#endif
  vTaskStartScheduler();
  return 0;
}
#if (0)
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);

int main(void) {
  create_blinky_tasks();
  create_uart_task();

  puts("Starting RTOS");
  vTaskStartScheduler(); // This function never returns unless RTOS scheduler runs out of memory and fails

  return 0;
}

static void create_blinky_tasks(void) {
  /**
   * Use '#if (1)' if you wish to observe how two tasks can blink LEDs
   * Use '#if (0)' if you wish to use the 'periodic_scheduler.h' that will spawn 4 periodic tasks, one for each LED
   */
#if (1)
  // These variables should not go out of scope because the 'blink_task' will reference this memory
  static gpio_s led0, led1;

  led0 = board_io__get_led0();
  led1 = board_io__get_led1();

  xTaskCreate(blink_task, "led0", configMINIMAL_STACK_SIZE, (void *)&led0, PRIORITY_LOW, NULL);
  xTaskCreate(blink_task, "led1", configMINIMAL_STACK_SIZE, (void *)&led1, PRIORITY_LOW, NULL);
#else
  const bool run_1000hz = true;
  const size_t stack_size_bytes = 2048 / sizeof(void *); // RTOS stack size is in terms of 32-bits for ARM M4 32-bit CPU
  periodic_scheduler__initialize(stack_size_bytes, !run_1000hz); // Assuming we do not need the high rate 1000Hz task
  UNUSED(blink_task);
#endif
}

static void create_uart_task(void) {
  // It is advised to either run the uart_task, or the SJ2 command-line (CLI), but not both
  // Change '#if (0)' to '#if (1)' and vice versa to try it out
#if (0)
  // printf() takes more stack space, size this tasks' stack higher
  xTaskCreate(uart_task, "uart", (512U * 8) / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#else
  sj2_cli__init();
  UNUSED(uart_task); // uart_task is un-used in if we are doing cli init()
#endif
}

static void blink_task(void *params) {
  const gpio_s led = *((gpio_s *)params); // Parameter was input while calling xTaskCreate()

  // Warning: This task starts with very minimal stack, so do not use printf() API here to avoid stack overflow
  while (true) {
    gpio__toggle(led);
    vTaskDelay(500);
  }
}

// This sends periodic messages over printf() which uses system_calls.c to send them to UART0
static void uart_task(void *params) {
  TickType_t previous_tick = 0;
  TickType_t ticks = 0;

  while (true) {
    // This loop will repeat at precise task delay, even if the logic below takes variable amount of ticks
    vTaskDelayUntil(&previous_tick, 2000);

    /* Calls to fprintf(stderr, ...) uses polled UART driver, so this entire output will be fully
     * sent out before this function returns. See system_calls.c for actual implementation.
     *
     * Use this style print for:
     *  - Interrupts because you cannot use printf() inside an ISR
     *    This is because regular printf() leads down to xQueueSend() that might block
     *    but you cannot block inside an ISR hence the system might crash
     *  - During debugging in case system crashes before all output of printf() is sent
     */
    ticks = xTaskGetTickCount();
    fprintf(stderr, "%u: This is a polled version of printf used for debugging ... finished in", (unsigned)ticks);
    fprintf(stderr, " %lu ticks\n", (xTaskGetTickCount() - ticks));

    /* This deposits data to an outgoing queue and doesn't block the CPU
     * Data will be sent later, but this function would return earlier
     */
    ticks = xTaskGetTickCount();
    printf("This is a more efficient printf ... finished in");
    printf(" %lu ticks\n\n", (xTaskGetTickCount() - ticks));
  }
}
#endif