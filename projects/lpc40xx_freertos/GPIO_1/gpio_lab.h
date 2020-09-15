#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  uint8_t port;
  uint8_t pin;
} port_pin_s;

void gpio0__set_as_input(port_pin_s *p_s);
void gpio0__set_as_output(port_pin_s *p_s);

void gpio0__set_high(port_pin_s *p_s);
void gpio0__set_low(port_pin_s *p_s);
void gpio0__set(port_pin_s *p_s, bool high);

bool gpio0__get_level(port_pin_s *p_s);

// void gpio0__set_as_input(uint8_t pin_num);
// void gpio0__set_as_output(uint8_t pin_num);
// void gpio0__set_high(uint8_t pin_num);
// void gpio0__set_low(uint8_t pin_num);
// void gpio0__set(uint8_t pin_num, bool high);
// bool gpio0__get_level(uint8_t pin_num);