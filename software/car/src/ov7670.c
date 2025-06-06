#include "ov7670.h"

#include "ov7670_init.h"
#include "hardware/dma.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include <hardware/gpio.h>
#include <hardware/clocks.h>
#include <stdio.h> // Required for printf

#define MIN_XCLK_FREQ 10000000  // Define the minimum XCLK frequency (Hz)

void ov7670_init(struct ov7670_config *config) {
	// XCLK generation (Min Freq)
    gpio_set_function(config->pin_xclk, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(config->pin_xclk);

    // Ensure that values won't be negative.
    uint32_t system_clock = clock_get_hz(clk_sys);
    float divider = (float)system_clock / MIN_XCLK_FREQ;

    // Check if calculation comes with floating value
    uint32_t wrap = (uint32_t)divider - 1;

    // All numbers are greater
    pwm_set_wrap(slice_num, wrap);
    pwm_set_gpio_level(config->pin_xclk, wrap / 2);  // 50% duty cycle
    pwm_set_enabled(slice_num, true);
    

  // SCCB I2C @ 100 kHz
  gpio_set_function(config->pin_sioc, GPIO_FUNC_I2C);
  gpio_set_function(config->pin_siod, GPIO_FUNC_I2C);
  gpio_pull_up(config->pin_sioc);
  gpio_pull_up(config->pin_siod);
  i2c_init(config->sccb, 25 * 1000);

  // Initialise reset pin
  gpio_init(config->pin_resetb);
  gpio_set_dir(config->pin_resetb, GPIO_OUT);

  // Reset camera, and give it some time to wake back up
  gpio_put(config->pin_resetb, 0);
  sleep_ms(100);
  gpio_put(config->pin_resetb, 1);
  sleep_ms(100);

  gpio_init(config->pin_vsync);
	gpio_set_dir(config->pin_vsync, GPIO_IN);
  
  gpio_init(config->pin_href);
  gpio_set_dir(config->pin_href, GPIO_IN);

	for (int i = 0; i < 8; i++) {
		gpio_init(config->pins_data[i]);
		gpio_set_dir(config->pins_data[i], GPIO_IN);
	}
}

// void ov2640_reg_write(struct ov2640_config *config, uint8_t reg, uint8_t value) {
// 	uint8_t data[] = {reg, value};
// 	i2c_write_blocking(config->sccb, 0x42 >> 1, data, sizeof(data), false);
// }

void ov7670_reg_write(struct ov7670_config *config, uint8_t reg, uint8_t value) {
	uint8_t data[] = {reg, value};
	i2c_write_blocking(config->sccb, 0x42 >> 1, data, sizeof(data), false);
}

uint8_t ov7670_reg_read(struct ov7670_config *config, uint8_t reg) {
  int result = i2c_write_blocking(config->sccb, 0x42 >> 1, &reg, 1, false);
  if (result != 1) {
    printf("I2C write error: %d\n", result);
    // return 0xFF; // Or some other error indicator
  }

  uint8_t value = 0xff;
  result = i2c_read_blocking(config->sccb, 0x42 >> 1, &value, 1, false);
  if (result != 1) {
    printf("I2C read error: %d\n", result);
    // return 0xFF; // Or some other error indicator
  }

  return value;
}

// uint8_t ov7670_read_pixel(struct ov7670_config *config) {
// 	uint8_t data = 0;
// 	for (int i = 0; i < 8; i++) data |= gpio_get(config->pins[i]) << i;
// 	return data;
// }

// uint8_t ov7670_read_frame(struct ov7670_config *config, uint8_t *framebuffer) {
// // Wait for VSYNC to go low (start of frame)
//     while (gpio_get(config->vsync) == 1);
//     while (gpio_get(config->vsync) == 0);

//     // Read image data row by row
//     for (int y = 0; y < 100 * 3; ++y) {
//       // Wait for HREF to go high (start of row)
//       while (gpio_get(config->href) == 0);

//       for (int x = 0; x < 640; ++x) {
//         // Wait for PCLK to rise
//         while (gpio_get(config->pclk) == 0);

//         // Read pixel data (grayscale)
//         framebuffer[y * 640 + x] = ov7670_read_pixel(config);

//         // Wait for PCLK to fall
//         while (gpio_get(config->pclk) == 1);
//       }
//       // Wait for HREF to go low (end of row)
//       while (gpio_get(config->href) == 1);
//     }
// }

void ov7670_regs_write(struct ov7670_config *config, const uint8_t (*regs_list)[2]) {
	while (1) {
		uint8_t reg = (*regs_list)[0];
		uint8_t value = (*regs_list)[1];

		if (reg == 0x00 && value == 0x00) {
			break;
		}

		ov7670_reg_write(config, reg, value);

		regs_list++;
	}
}

void ov7670_capture_frame(struct ov7670_config *config) {
    // Wait for VSYNC falling edge
    while (gpio_get(config->pin_vsync) == 1);
    while (gpio_get(config->pin_vsync) == 0);

    for (int y = 0; y < 240; ++y) {
        // Wait for HREF high
        while (gpio_get(config->pin_href) == 0);

        for (int x = 0; x < 320; ++x) {
					for (int b = 0; b < 2; ++b) {  // Read 2 bytes per pixel
							while (gpio_get(config->pin_pclk) == 0); // Wait for PCLK high

							uint8_t data = 0;
							for (int i = 0; i < 8; i++) data |= gpio_get(config->pins_data[i]) << i;

							config->image_buf[(y * 320 + x) * 2 + b] = data;

							while (gpio_get(config->pin_pclk) == 1); // Wait for PCLK low
					}
			}

        // Wait for HREF low (end of row)
        while (gpio_get(config->pin_href) == 1);
    }
}

