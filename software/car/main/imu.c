#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// Define the Chip Select (CS) pin
static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(17, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(17, 1);
    asm volatile("nop \n nop \n nop");
}

// Write a value to a register
static void write_register(uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    buf[0] = 0b01111111 & reg;  // Mask to ensure the correct register address
    buf[1] = data;
    cs_select();
    spi_write_blocking(spi0, buf, 2);
    cs_deselect();
    sleep_ms(10);  // Small delay to ensure the write completes
}

// Read multiple registers into a buffer
static void read_registers(uint8_t reg, uint8_t *buf, uint16_t len) {
    uint8_t b = 0b10000000 | reg;  // Set the read bit (MSB = 1)
    cs_select();
    spi_write_blocking(spi0, &b, 1);  // Send register address
    sleep_ms(10);  // Short delay
    spi_read_blocking(spi0, 0, buf, len);  // Read the data into the buffer
    cs_deselect();
    sleep_ms(10);  // Short delay
}

int main() {
    stdio_init_all();
    sleep_ms(2000);  // Wait for the sensor to power up

    printf("Hello, LIS3DH! Reading raw data from registers via SPI...\n");

    // Initialize SPI (SPI0) with 10 MHz clock speed
    spi_init(spi0, 10000 * 1000);
    gpio_set_function(16, GPIO_FUNC_SPI);  // SCK
    gpio_set_function(18, GPIO_FUNC_SPI);  // MOSI
    gpio_set_function(19, GPIO_FUNC_SPI);  // MISO

    // Chip Select (CS) is active low, so initialize it to high (inactive)
    gpio_init(17);
    gpio_set_dir(17, GPIO_OUT);
    gpio_put(17, 1);

    sleep_ms(10);  // Give some time before writing to the sensor

    // Initialize the LIS3DH sensor by configuring the necessary registers
    write_register(0x20, 0x27);  // CTRL_REG1: Normal mode, 100Hz, enable X, Y, Z axes
    write_register(0x23, 0x00);  // CTRL_REG4: ±2g full-scale range (0x00)
    write_register(0x22, 0x00);  // CTRL_REG3: Disable high-pass filter (optional)
    
    // Check the chip's status via the STATUS_REG (0x27)
    while (1) {
        uint8_t status = 0;
        read_registers(0x27, &status, 1);  // Read STATUS_REG to check data readiness

        if (status & 0x08) {  // Data available for reading (ZYXDA bit is set)
            uint8_t axis_data[6];  // 6 bytes for X, Y, Z axes data (2 bytes per axis)
            read_registers(0x28, axis_data, 6);  // Read 6 bytes starting from OUT_X_L

            // Combine the low and high bytes for each axis
            int16_t x = (int16_t)(axis_data[1] << 8 | axis_data[0]);
            int16_t y = (int16_t)(axis_data[3] << 8 | axis_data[2]);
            int16_t z = (int16_t)(axis_data[5] << 8 | axis_data[4]);

            // Print the raw axis values
            printf("Raw X: %d, Y: %d, Z: %d\n", x, y, z);

            // Apply scaling factor for ±2g range (16-bit signed values, ±2g = ±32768)
            float x_g = (float)x / 32768.0f;
            float y_g = (float)y / 32768.0f;
            float z_g = (float)z / 32768.0f;

            // Print the scaled values in g
            printf("Scaled X: %.4f g, Y: %.4f g, Z: %.4f g\n", x_g, y_g, z_g);
        } else {
            printf("Data not ready yet.\n");
        }

        sleep_ms(100);  // Poll every 100ms
    }

    return 0;
}

