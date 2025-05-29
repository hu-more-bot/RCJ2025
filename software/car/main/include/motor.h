#pragma once

#include <stdint.h>

void motor_init(uint8_t pin);

void motor_set(uint8_t pin, uint16_t us);

