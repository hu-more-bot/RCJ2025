#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"

struct ov7670_config {
	i2c_inst_t *sccb;
	uint pin_sioc;
	uint pin_siod;
	
	uint pin_resetb;
	uint pin_xclk;
	uint pin_vsync;
	uint pin_href;
	uint pin_pclk;

	uint pins_data[8];

	uint8_t *image_buf;
	size_t image_buf_size;
};

void ov7670_init(struct ov7670_config *config);

// void ov7670_capture_frame(struct ov7670_config *config);

void ov7670_reg_write(struct ov7670_config *config, uint8_t reg, uint8_t value);
uint8_t ov7670_reg_read(struct ov7670_config *config, uint8_t reg);
void ov7670_regs_write(struct ov7670_config *config, const uint8_t (*regs_list)[2]);

// uint8_t ov7670_read_pixel(struct ov7670_config *config);
// uint8_t ov7670_read_frame(struct ov7670_config *config, uint8_t *framebuffer);
void ov7670_capture_frame(struct ov7670_config *config);
