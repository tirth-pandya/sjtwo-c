#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "cpu_utilization_task.h"

#include "queue.h"

static QueueHandle_t switch_queue;
typedef enum { switch__off, switch__on } switch_e;

static switch_e get_switch_input_from_switch0(void) { return switch__on; }
void producer(void *p) {
  while (1) {
    const switch_e switch_value = get_switch_input_from_switch0();
    printf("Status : Before xQueueSend\n");
    if (!xQueueSend(switch_queue, &switch_value, 0)) {
      printf("Status : Error sending data over queue\n");
    }
    printf("Status : xQueueSend success, Data : %d\n", switch_value);
    vTaskDelay(1000);
  }
}

void consumer(void *p) {
  switch_e switch_value = switch__off;
  while (1) {
    printf("Status : Before xQueueReceive\n");
    if (xQueueReceive(switch_queue, &switch_value, portMAX_DELAY)) {
      printf("Status : Data Received, Data : %d\n", switch_value);
    }
  }
}

int main(void) {
  switch_queue = xQueueCreate(10, sizeof(switch_e));
  xTaskCreate(producer, "Producer", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(consumer, "Consumer", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
  return 0;
}
#if (0)
/**
 * This POSIX based FreeRTOS simulator is based on:
 * https://github.com/linvis/FreeRTOS-Sim
 *
 * Do not use for production!
 * There may be issues that need full validation of this project to make it production intent.
 * This is a great teaching tool though :)
 */
int main(void) {
  xTaskCreate(cpu_utilization_print_task, "cpu", 1, NULL, PRIORITY_LOW, NULL);

  puts("Starting FreeRTOS Scheduler ..... \r\n");
  vTaskStartScheduler();

  return 0;
}
#endif