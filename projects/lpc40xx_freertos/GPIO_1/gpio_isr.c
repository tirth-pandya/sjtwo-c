#include "gpio_isr.h"
#include "lpc40xx.h"
#include <stdio.h>

static function_pointer_t gpio0_callbacks__rising_edge[32];
static function_pointer_t gpio0_callbacks__falling_edge[32];

void gpio0__attach_interrupt(uint32_t pin, gpio_interrupt_e interrupt_type, function_pointer_t callback) {
  if (interrupt_type == GPIO_INTR__RISING_EDGE) {
    gpio0_callbacks__rising_edge[pin] = callback;
    LPC_GPIOINT->IO0IntEnR |= (1 << pin);
  }

  else {
    gpio0_callbacks__falling_edge[pin] = callback;
    LPC_GPIOINT->IO0IntEnF |= (1 << pin);
  }
}
static void clear_pin_interrupt(int pin) { LPC_GPIOINT->IO0IntClr |= (1 << pin); }
static int get_interrupt_pin(gpio_s gpio, gpio_interrupt_e *interrupt_type) {
  uint32_t val;
  int int_pin = 0;

  if (LPC_GPIOINT->IO0IntStatR) {
    val = LPC_GPIOINT->IO0IntStatR;
    *interrupt_type = GPIO_INTR__RISING_EDGE;
  } else {
    val = LPC_GPIOINT->IO0IntStatF;
    *interrupt_type = GPIO_INTR__RISING_EDGE;
  }

  while (val) {
    val = val >> 1;
    ++int_pin;
  }
  return int_pin;
}

void gpio__interrupt_dispatcher(void) {

  gpio_s gpio_to_check;
  gpio_interrupt_e int_type;

  const int pin_that_generated_interrupt = get_interrupt_pin(gpio_to_check, &int_type);
  function_pointer_t attached_user_handler;

  if (int_type == GPIO_INTR__RISING_EDGE)
    attached_user_handler = gpio0_callbacks__rising_edge[pin_that_generated_interrupt];
  else

    attached_user_handler = gpio0_callbacks__falling_edge[pin_that_generated_interrupt];

  fprintf(stderr, "ISR dispatch for GPIO 0.%d", pin_that_generated_interrupt);
  attached_user_handler();
  clear_pin_interrupt(pin_that_generated_interrupt);
}
