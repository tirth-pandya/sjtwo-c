#include "gpio_lab.h"
#include "lpc40xx.h"

// Pointers to GPIO port memory
static const LPC_GPIO_TypeDef *gpio_port_array[] = {LPC_GPIO0, LPC_GPIO1, LPC_GPIO2};

void gpio0__set_as_input(port_pin_s *p_s) {
  LPC_GPIO_TypeDef *temp = gpio_port_array[p_s->port];
  temp->DIR &= ~(1 << p_s->pin);
}

void gpio0__set_as_output(port_pin_s *p_s) {
  LPC_GPIO_TypeDef *temp = gpio_port_array[p_s->port];
  temp->DIR |= (1 << p_s->pin);
}

void gpio0__set_high(port_pin_s *p_s) {
  LPC_GPIO_TypeDef *temp = gpio_port_array[p_s->port];
  temp->SET = (1 << p_s->pin);
}

void gpio0__set_low(port_pin_s *p_s) {
  LPC_GPIO_TypeDef *temp = gpio_port_array[p_s->port];
  temp->CLR = (1 << p_s->pin);
}

void gpio0__set(port_pin_s *p_s, bool high) {
  if (high)
    gpio0__set_high(p_s);
  else
    gpio0__set_low(p_s);
}

bool gpio0__get_level(port_pin_s *p_s) {
  LPC_GPIO_TypeDef *temp = gpio_port_array[p_s->port];
  return (bool)(temp->PIN & (1 << p_s->pin));
}
