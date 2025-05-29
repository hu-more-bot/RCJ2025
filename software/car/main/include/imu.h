#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct i2c_inst i2c_inst_t;

// Initialize lis3dh (default addr=0x18)
void lis3dh_init(i2c_inst_t *i2c, uint8_t sda, uint8_t scl, uint8_t addr);

// Read Data from Register
void lis3dh_read_data(i2c_inst_t *i2c, uint8_t reg, float *final_value, bool IsAccel);

// Read All Data
void lis3dh_read(i2c_inst_t *i2c, float *accel[3], float *temp);
