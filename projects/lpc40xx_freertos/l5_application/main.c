#include "FreeRTOS.h"
#include "board_io.h"
#include "common_macros.h"
#include "task.h"
#include <stdio.h>
//#include "gpio.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"

#define LAB_02_GPIO
//#define LAB_01_TASK
//#define LAB_DEFAULT

#ifdef LAB_02_GPIO
#include "gpio_lab.h"
#include "semphr.h"
static SemaphoreHandle_t switch_press_indication;
volatile int flag = 0;

void switch_task(void *task_parameter);
void led_task(void *task_parameter);
#endif

#ifdef LAB_01_TASK
static void task_one(void *task_param);
static void task_two(void *task_param);
#endif

#ifdef LAB_DEFAULT
static void create_blinky_tasks(void);
static void create_uart_task(void);
static void blink_task(void *params);
static void uart_task(void *params);
#endif

int main(void) {
  puts("Starting RTOS");

#ifdef LAB_02_GPIO
  switch_press_indication = xSemaphoreCreateBinary();

  static port_pin_s led[] = {{2, 3}, {1, 26}, {1, 24}, {1, 18}};
  static port_pin_s sw[] = {{1, 19}, {1, 15}, {0, 30}, {0, 29}};

  xTaskCreate(switch_task, "switch", 512, &sw, PRIORITY_HIGH, NULL);
  xTaskCreate(led_task, "led", 512, &led, PRIORITY_HIGH, NULL);
#endif

#ifdef LAB_01_TASK
  xTaskCreate(task_one, "task1", 1024, NULL, PRIORITY_HIGH, NULL);
  xTaskCreate(task_two, "task2", 1024, NULL, PRIORITY_LOW, NULL);
#endif

  vTaskStartScheduler();
  return 0;
}

#ifdef LAB_DEFAULT
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
  const size_t stack_size_bytes = 2048 / sizeof(void *); // RTOS stack size is in terms of 32-bits for ARM M4 32-bit
  CPU periodic_scheduler__initialize(stack_size_bytes, !run_1000hz); // Assuming we do not need the high rate 1000Hz
  task UNUSED(blink_task);
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

// Lab Tasks here:

#ifdef LAB_01_TASK
// Assignment 01: Multiple Tasks
static void task_one(void *task_param) {
  while (true) {
    fprintf(stderr, "AAAAAAAAAAAA");
    vTaskDelay(100);
  }
}

static void task_two(void *task_param) {
  while (true) {
    fprintf(stderr, "bbbbbbbbbbbb");
    vTaskDelay(100);
  }
}
#endif

#ifdef LAB_02_GPIO
// Assignment 02: GPIO Driver
void switch_task(void *task_parameter) {
  port_pin_s *sw = (port_pin_s *)task_parameter;
  puts("here");

  for (int i = 0; i < 4; i++)
    gpio0__set_as_input(&sw[i]); // Set all switches as input

  while (true) {
    if (gpio0__get_level(&sw[3])) {
      while (gpio0__get_level(&sw[3]) != 0)
        ; // Wait until switch is released
      flag = 3;
    }

    else if (gpio0__get_level(&sw[2])) {
      while (gpio0__get_level(&sw[2]) != 0)
        ; // Wait until switch is released
      flag = 2;
    }

    else if (gpio0__get_level(&sw[1])) {
      while (gpio0__get_level(&sw[1]) != 0)
        ; // Wait until switch is released
      flag = 1;
    }

    else if (gpio0__get_level(&sw[0])) {
      while (gpio0__get_level(&sw[0]) != 0)
        ; // Wait until switch is released
      flag = 0;
    }

    xSemaphoreGive(switch_press_indication);
    vTaskDelay(100);
  }
}
void led_task(void *task_parameter) {
  port_pin_s *led = (port_pin_s *)(task_parameter);

  for (int i = 0; i < 4; i++) {
    gpio0__set_as_output(&led[i]); // Set all led port pins as output
    gpio0__set_high(&led[i]);      // Turn all leds off initially
  }

  int8_t i = 0;

  while (true) {
    if (xSemaphoreTake(switch_press_indication, 1000)) {
      if (flag == 3) {
        // Blink LEDs from left to right
        gpio0__set_low(&led[3 - i]);
        vTaskDelay(75);
        gpio0__set_high(&led[3 - i]);
        i++;
        if (i == 4)
          i = 0;
      }

      if (flag == 2) {
        // Blink LEDs from right to left
        gpio0__set_low(&led[3 - i]);
        vTaskDelay(75);
        gpio0__set_high(&led[3 - i]);
        i--;
        if (i < 0)
          i = 3;
      }

      if (flag == 1) {
        // Turn all the LEDS on
        for (int j = 0; j < 4; j++)
          gpio0__set_low(&led[j]);
      }

      if (flag == 0) {
        // Turn all the LEDs off
        for (int j = 0; j < 4; j++)
          gpio0__set_high(&led[j]);
      }
    }
    vTaskDelay(100);
  }
}

#endif