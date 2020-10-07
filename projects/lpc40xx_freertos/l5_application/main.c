#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "gpio.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"

// Includes for Assignment 05 - SPI
#include "lpc_peripherals.h"
#include "semphr.h"
#include "ssp_lab.h"
//

#if defined(LAB_05_P1) || defined(LAB_05_P2) || defined(LAB_05_P3)
typedef struct {
  uint8_t manufacturer_id;
  uint8_t device_id_1;
  uint8_t device_id_2;
  uint8_t extended_device_id;
} adesto_flash_id_s;

adesto_flash_id_s adesto_read_signature(void);
#endif

//______________
// TASK for PART 03 HERE:
//______________
#ifdef LAB_05_P3
SemaphoreHandle_t spi_bus_mutex;

void spi_erase_and_write_task(void *p) {
  uint32_t block_address = 0x000000;
  uint32_t page_address = 0x000001;
  uint8_t page_data_to_write = 0x77;
  while (1) {
    if (xSemaphoreTake(spi_bus_mutex, 1000)) {
      adesto_block_erase_64kb(block_address);
      adesto_page_program(page_address, page_data_to_write);
      xSemaphoreGive(spi_bus_mutex);
    }
    printf("\nErased => Block : 0x%lx\n", block_address);
    printf("Write =>\tPage address : 0x%lx\t Data : 0x%x\n", page_address, page_data_to_write);
    vTaskDelay(500);
  }
}

void spi_read_task(void *p) {
  uint32_t page_address = 0x000001;
  uint8_t page_data_retrieved = 0;
  while (1) {
    if (xSemaphoreTake(spi_bus_mutex, 1000)) {
      page_data_retrieved = adesto_page_read(page_address);
      xSemaphoreGive(spi_bus_mutex);
    }
    printf("Read =>\t\tPage address : 0x%lx\t Data : 0x%x\n", page_address, page_data_retrieved);
    vTaskDelay(500);
  }
}
#endif
//______________

//______________
// TASK for PART 02 HERE:
//______________
#ifdef LAB_05_P2
SemaphoreHandle_t spi_bus_mutex;

void spi_id_verification_task(void *p) {
  while (1) {
    if (xSemaphoreTake(spi_bus_mutex, 1000)) {
      const adesto_flash_id_s id = adesto_read_signature();

      // When we read a manufacturer ID we do not expect, we will kill this task
      if (id.manufacturer_id != 0x1F) {
        fprintf(stderr, "Manufacturer ID read failure\n");
        vTaskSuspend(NULL); // Kill this task
      }
      xSemaphoreGive(spi_bus_mutex);
      printf("'Semaphore' Adesto man_id:%x\tid_1:%x\t\tid_2:%x\t\textended_id:%x\n", id.manufacturer_id, id.device_id_1,
             id.device_id_2, id.extended_device_id);
      vTaskDelay(100);
    }
  }
}
#endif
//______________

//______________
// TASK for PART 01 HERE:
//______________
#ifdef LAB_05_P1
void spi_task(void *p) {
  const uint32_t spi_clock_mhz = 24;
  ssp__init(spi_clock_mhz);

  while (1) {
    adesto_flash_id_s id = adesto_read_signature();
    printf("Adesto man_id:%x\tid_1:%x\t\tid_2:%x\t\textended_id:%x\n", id.manufacturer_id, id.device_id_1,
           id.device_id_2, id.extended_device_id);
    vTaskDelay(500);
  }
}
#endif
//______________

int main(void) {

  // SPI Initialization
  const uint32_t spi_clock_mhz = 6;
  ssp__init(spi_clock_mhz);
  //

#ifdef LAB_05_P3
  spi_bus_mutex = xSemaphoreCreateMutex();
  xTaskCreate(spi_erase_and_write_task, "spi_erase_write", 2048 / sizeof(void *), NULL, PRIORITY_MEDIUM, NULL);
  xTaskCreate(spi_read_task, "spi_read", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#endif

#ifdef LAB_05_P2
  spi_bus_mutex = xSemaphoreCreateMutex();

  xTaskCreate(spi_id_verification_task, "spi_id_1", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(spi_id_verification_task, "spi_id_2", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#endif

#ifdef LAB_05_P1
  xTaskCreate(spi_task, "spi", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#endif

  vTaskStartScheduler();
  return 0;
}

#if defined(LAB_05_P1) || defined(LAB_05_P2) || defined(LAB_05_P3)
adesto_flash_id_s adesto_read_signature(void) {
  adesto_flash_id_s data = {0};
  const uint8_t manufacturer_id_opcode = 0x9f;
  const uint8_t dummy_byte_to_read_values = 0xaa;
  adesto_cs();
  {
    ssp__exchange_byte(manufacturer_id_opcode);
    data.manufacturer_id = ssp__exchange_byte(dummy_byte_to_read_values);
    data.device_id_1 = ssp__exchange_byte(dummy_byte_to_read_values);
    data.device_id_2 = ssp__exchange_byte(dummy_byte_to_read_values);
    data.extended_device_id = ssp__exchange_byte(dummy_byte_to_read_values);
  }
  adesto_ds();
  return data;
}
#endif

//****************************
// END OF ASSIGNMENT
//****************************
#if (0) // Default main.c file
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