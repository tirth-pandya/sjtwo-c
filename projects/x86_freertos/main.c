#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "cpu_utilization_task.h"

#include "queue.h"

static QueueHandle_t q;

void sender_task(void *p) {
  int sent_data = 0;
  while (1) {
    if (!xQueueSend(q, &sent_data, 0)) {
      printf("Couldn't send to the queue\n");
    }
    sent_data++;
    vTaskDelay(1000);
  }
}

void receiver_task(void *p) {
  int received_data;
  while (1) {
    xQueueReceive(q, &received_data, portMAX_DELAY);
    printf("Received %d\n", received_data);
  }
}

int main(void) {
  q = xQueueCreate(10, sizeof(int));
  xTaskCreate(sender_task, "sender", 1024, NULL, PRIORITY_LOW, NULL);
  xTaskCreate(receiver_task, "receiver", 1024, NULL, PRIORITY_LOW, NULL);
  vTaskStartScheduler();
  return 0;
}
