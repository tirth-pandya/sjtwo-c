#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board_io.h"
#include "common_macros.h"
#include "gpio.h"
#include "periodic_scheduler.h"
#include "sj2_cli.h"

#ifdef LAB_04_ADC_PWM
#include "adc.h"
#include "lpc_peripherals.h"
#include "pwm1.h"
#include "queue.h"
#include "semphr.h"
#endif

//____________________
// Declarations
//____________________

#if defined(LAB_04_P0) || defined(LAB_04_P2) || defined(LAB_04_P3)
void pin_configure_pwm_channel_as_io_pin(gpio__port_e, uint8_t);
void pwm_task(void *);
#endif

#if defined(LAB_04_P1) || defined(LAB_04_P2) || defined(LAB_04_P3)
void pin_configure_adc_channel_as_io_pin(gpio__port_e port_no, uint8_t pin_no);
void adc_task(void *);
#endif

#if defined(LAB_04_P2) || defined(LAB_04_P3)
static QueueHandle_t adc_to_pwm_task_queue;
#endif

//____________________
// Main
//____________________

int main(void) {
  puts("Starting RTOS");

#ifdef LAB_04_P0
  xTaskCreate(pwm_task, "pwm_task", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#endif

#ifdef LAB_04_P1
  xTaskCreate(adc_task, "adc_task", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#endif

#ifdef LAB_04_P2
  adc_to_pwm_task_queue = xQueueCreate(10, sizeof(uint16_t));
  xTaskCreate(adc_task, "adc_task", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
  xTaskCreate(pwm_task, "pwm_task", 2048 / sizeof(void *), NULL, PRIORITY_LOW, NULL);
#endif

  vTaskStartScheduler();
  return 0;
}

//____________________
// Definitions
//____________________

#if defined(LAB_04_P0) || defined(LAB_04_P2) || defined(LAB_04_P3)
void pin_configure_pwm_channel_as_io_pin(gpio__port_e port_no, uint8_t pin_no) {
  // Configure Port pin as PWM output
  gpio__construct_with_function(port_no, pin_no, GPIO__FUNCTION_1);
}
void pwm_task(void *p) {
  pwm1__init_single_edge(1000);
  pin_configure_pwm_channel_as_io_pin(GPIO__PORT_2, 0); // P2.0 as PWM
  uint8_t percent = 0;

#ifdef LAB_04_P0
  pwm1__set_duty_cycle(PWM1__2_0, 50);
  while (1) {
    pwm1__set_duty_cycle(PWM1__2_0, percent);
    if (++percent > 100)
      percent = 0;
    vTaskDelay(100);
  }
#endif

#if defined(LAB_04_P2) || defined(LAB_04_P3)
  int adc_reading = 0;
  while (1) {
    if (xQueueReceive(adc_to_pwm_task_queue, &adc_reading, 100)) {
      percent = (adc_reading * 100) / MAX_ADC_VAL;
      pwm1__set_duty_cycle(PWM1__2_0, percent);
#ifdef LAB_04_P3
      pwm1__set_duty_cycle(PWM1__2_1, (percent + 20) % 100);
      pwm1__set_duty_cycle(PWM1__2_2, (percent + 50) % 100);
      vTaskDelay(100);
#endif
    }
  }
#endif
}
#endif

#ifdef LAB_04_ADC_PWM

#if defined(LAB_04_P1) || defined(LAB_04_P2) || defined(LAB_04_P3)
void pin_configure_adc_channel_as_io_pin(gpio__port_e port_no, uint8_t pin_no) {
  // Configure Port pin as ADC input
  gpio__construct_with_function(port_no, pin_no, GPIO__FUNCTION_1);
  LPC_IOCON->P0_25 &= ~(0x98); // P0.25 - IO Control register - mode analog input
}

void adc_task(void *p) {
  adc__initialize();
  adc__enable_burst_mode();
  pin_configure_adc_channel_as_io_pin(GPIO__PORT_0, 25); // P0.25 as ADC

#ifdef LAB_04_P1
  while (1) {
    const uint16_t adc_value = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_2);
    double volt_val = (adc_value * 3.3) / MAX_ADC_VAL; // Convert adc val to voltage
    fprintf(stderr, "ADC Value : %d, Volt Value : %f\n", adc_value, volt_val);
    vTaskDelay(100);
  }
#endif

#if defined(LAB_04_P2) || defined(LAB_04_P3)
  while (1) {
    const uint16_t adc_reading = adc__get_channel_reading_with_burst_mode(ADC__CHANNEL_2);
    xQueueSend(adc_to_pwm_task_queue, &adc_reading, 0);
    vTaskDelay(100);
  }
#endif
}
#endif

#endif