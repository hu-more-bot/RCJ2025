#include "imu.h"

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

const uint8_t CTRL_REG_1 = 0x20;
const uint8_t CTRL_REG_4 = 0x23;
const uint8_t TEMP_CFG_REG = 0x1F;

void lis3dh_init(i2c_inst_t *i2c, int sda, int scl, uint8_t addr) {
    i2c_init(i2c, 400 * 1000);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);
    
    uint8_t buf[2];

    // Turn normal mode and 1.344kHz data rate on
    buf[0] = CTRL_REG_1;
    buf[1] = 0x97;
    i2c_write_blocking(i2c, addr, buf, 2, false);

    // Turn block data update on (for temperature sensing)
    buf[0] = CTRL_REG_4;
    buf[1] = 0x80;
    i2c_write_blocking(i2c, addr, buf, 2, false);

    // Turn auxiliary ADC on
    buf[0] = TEMP_CFG_REG;
    buf[1] = 0xC0;
    i2c_write_blocking(i2c, addr, buf, 2, false);
}

void lis3dh_calc_value(uint16_t raw_value, float *final_value, bool isAccel) {
    // Convert with respect to the value being temperature or acceleration reading 
    float scaling;
    float senstivity = 0.004f; // g per unit

    if (isAccel == true) {
        scaling = 64 / senstivity;
    } else {
        scaling = 64;
    }

    // raw_value is signed
    *final_value = (float) ((int16_t) raw_value) / scaling;
}

void lis3dh_read_data(i2c_inst_t *i2c, uint8_t reg, float *final_value, bool IsAccel) {
    // Read two bytes of data and store in a 16 bit data structure
    uint8_t lsb;
    uint8_t msb;
    uint16_t raw_accel;
    i2c_write_blocking(i2c_default, i2c, &reg, 1, true);
    i2c_read_blocking(i2c_default, i2c, &lsb, 1, false);

    reg |= 0x01;
    i2c_write_blocking(i2c_default, i2c, &reg, 1, true);
    i2c_read_blocking(i2c_default, i2c, &msb, 1, false);

    raw_accel = (msb << 8) | lsb;

    lis3dh_calc_value(raw_accel, final_value, IsAccel);
}

void lis3dh_read(i2c_inst_t *i2c, float *accel[3], float *temp) {
    lis3dh_read_data(i2c, 0x28, accel[0], true);
    lis3dh_read_data(i2c, 0x2A, accel[1], true);
    lis3dh_read_data(i2c, 0x2C, accel[2], true);
    lis3dh_read_data(i2c, 0x0C, temp, false);
}
