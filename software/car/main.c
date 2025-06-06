#include "ov7670.h"

#include <hardware/clocks.h>
#include <pico/stdio.h>
#include <pico/multicore.h>

#include <stdio.h>
#include <string.h>

uint8_t framebuffer[320 * 240 * 2];

#include "motor.h"

#define IMCAPTURE 0

// steering: 1300 1600 1900

#define TRESHOLD_COLOR 160
#define MIN_WIDTH 1 // (px)
#define MAX_WIDTH 24 // (px)

struct Line {
  int start, width;
};

uint8_t rgb565_mono(uint8_t high_byte, uint8_t low_byte) {
    // Extract 5-bit Red, 6-bit Green, and 5-bit Blue from RGB565
    uint8_t r5 = (high_byte >> 3) & 0x1F;
    uint8_t g6 = ((high_byte & 0x07) << 3) | ((low_byte >> 5) & 0x07);
    uint8_t b5 = low_byte & 0x1F;

    // Scale to 8-bit (0–255) approximation
    uint8_t r8 = (r5 * 255 + 15) / 31;
    uint8_t g8 = (g6 * 255 + 31) / 63;
    uint8_t b8 = (b5 * 255 + 15) / 31;

    // Calculate luminance using integer approximation of Rec.601
    // Y = 0.299*R + 0.587*G + 0.114*B ≈ (77*R + 150*G + 29*B) >> 8
    uint16_t gray = (77 * r8 + 150 * g8 + 29 * b8) >> 8;

    return (uint8_t)gray;
}

float road_detect(unsigned char *buf) {
    struct Line line[2];
    float steering_value = 0;
  
    for (int y = 0+115; y < 240-15; y++) {
      // x: pixel id in scanline
      unsigned char *scanline = &buf[(y * 320) * 2];
      int mid = 0;

      // find lines
      struct Line tmp;
      memset(line, 0, sizeof(struct Line) * 2);
      
      for (int x = 0 + 16; x < 320 - 16; x++) {
        tmp.start = x, tmp.width = 0;

        while (x < 320 - 16 && rgb565_mono(scanline[x * 2], scanline[x * 2 + 1]) < TRESHOLD_COLOR)
          tmp.width++, x++;

        if (MIN_WIDTH <= tmp.width && tmp.width <= MAX_WIDTH) {
          const uint8_t i = line[0].width > line[1].width;
            if (line[i].width < tmp.width) memcpy(&line[i], &tmp, sizeof(struct Line));
        }
      } // x check

      if (line[0].width && line[1].width) {
            mid = (line[0].start + line[0].width / 2 + line[1].start + line[1].width / 2) / 2;
      
        steering_value += (mid - 320/2);
      } // x check
    }

    return steering_value;
}

int clamp(int n, int min_value, int max_value) {
  if (n < min_value) {
    return min_value;
  }
  if (n > max_value) {
    return max_value;
  }
  return n;
}

void core1_entry() {
	motor_init(23);
	motor_set(23, 1600);
	
	while (1) {
		motor_set(23, 1600 + clamp((int)(-300 * (road_detect(framebuffer) / 250)), -300, 300));
	}
}

int main() {
	set_sys_clock_khz(125000, true);
	stdio_init_all();
	
	motor_init(23);
	motor_set(23, 1300);

	// Init BLDC
  sleep_ms(2000);
  motor_init(22);
  motor_set(22, 2000);
  sleep_ms(2000);
  motor_set(22, 1000);
  sleep_ms(2000);
  motor_set(22, 500);
  sleep_ms(2000);

  motor_set(22, 1200);
	sleep_ms(500);

	motor_set(22, 1080);
	sleep_ms(1000);

	motor_set(22, 1000);
	while (1);

	
	
  

	// sleep_ms(4000);

	// motor_set(22, 1000);
	// while(1);

	// sleep_ms(2000);

	// printf("\n\nBooted!\n");

	sleep_ms(2000);

	gpio_init(13);
	gpio_set_dir(13, GPIO_OUT);
	gpio_put(13, 0);

	struct ov7670_config config = {i2c0, 1, 0, 15, 10, 14, 12, 7, {5, 3, 2, 4, 6, 8, 9, 11}, framebuffer, sizeof(framebuffer)};

	ov7670_init(&config);

	ov7670_reg_write(&config, 0x12, 0x80);
	sleep_ms(100);

	ov7670_reg_write(&config, 0x3A, 0b00000001);
	// ov7670_reg_write(&config, 0x3D, 0b00000000);

	// ov7670_reg_write(&config, 0x12, 0b00010100); // qvga + rgb
	ov7670_reg_write(&config, 0x12, 0b00010100); // qvga + rgb
	ov7670_reg_write(&config, 0x40, 0b11010000); // full range + rgb565

	ov7670_reg_write(&config, 0x1E, 0b00110000); // mirror & flip

	// ov7670_reg_write(&config, 0x15, 0x00); // COM10: default polarity

	// ov7670_reg_write(&config, 0x70, 0b00001010);
	// ov7670_reg_write(&config, 0x71, 0b10001010);

	ov7670_reg_write(&config, 0x11, 0b00000011); // prescaler
	ov7670_reg_write(&config, 0x0c, 0x04);
	ov7670_reg_write(&config, 0x3e, 0x19);
	ov7670_reg_write(&config, 0x72, 0b10001);
	ov7670_reg_write(&config, 0x73, 0xf1);
	ov7670_reg_write(&config, 0xa2, 0x02);

	ov7670_reg_write(&config, 0x13, 0b10001110);
	ov7670_reg_write(&config, 0x10, 0x08);

	ov7670_reg_write(&config, 0x41, 0b110000);
	ov7670_reg_write(&config, 0x76, 0b111111);

	sleep_ms(100);

	// Init BLDC
  sleep_ms(2000);
  motor_init(22);
  motor_set(22, 2000);
  sleep_ms(2000);
  motor_set(22, 1000);
  sleep_ms(2000);
  motor_set(22, 500);
  sleep_ms(2000);

  motor_set(22, 1200);
	sleep_ms(200);

#if IMCAPTURE
	sleep_ms(2000);
	ov7670_capture_frame(&config);

	for (int i = 0; i < 320 * 240 * 2; i++) {
		printf("%u ", framebuffer[i]);
	}

	while(1);
#else
	multicore_launch_core1(core1_entry);

	while (true) {
		motor_set(22, 1050);
		ov7670_capture_frame(&config);
	}
#endif

	return 0;
}

/*
	// Process Frame
		struct Line line[2] = {};
		float steering_value = 0;

			for (int y = 0 + 16; y < 240 - 16; y++) {
				// x: pixel id in scanline
				unsigned char *scanline = &framebuffer[(y * 320) * 2];
				int mid = 0;

				// find lines
				struct Line tmp;
				for (int x = 0 + 16; x < 320 - 16; x++) {
					uint8_t mono = rgb565_mono(scanline[x * 2], scanline[x * 2 + 1]);
					if (mono < TRESHOLD_COLOR) {
						tmp.start = x, tmp.width = 0;

						while (x < 320 && mono < TRESHOLD_COLOR) {
							tmp.width++, x++;
							mono = rgb565_mono(scanline[x * 2], scanline[x * 2 + 1]);
						}

						if (MIN_WIDTH <= tmp.width && tmp.width <= MAX_WIDTH) {
							const uint8_t i = line[0].width > line[1].width;
							if (line[i].width < tmp.width) memcpy(&line[i], &tmp, sizeof(struct Line));
							mid = (line[0].start + line[1].start) / 2;
						}
					}
				}

				if (line[0].width && line[1].width) {
					steering_value += (mid - 320 / 2.0f) * y;

					memset(scanline + mid * 2, 255, 20);
					steering_value += (mid - 320 / 2.0f) * y;

					memset(scanline + line[0].start * 2, 0, line[0].width * 2);
					memset(scanline + line[1].start * 2, 0, line[1].width * 2);
					
					memset(line, 0, sizeof(struct Line) * 2);
				}
			}
			*/
