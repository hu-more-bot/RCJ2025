#pragma once

#include <stdint.h>

void bldc_init(uint8_t pin);

void bldc_set_speed(uint8_t pin, uint16_t us);

