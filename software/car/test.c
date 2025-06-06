#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

int main() {
    uint8_t framebuffer[320 * 240];
    char filename[] = "screenlog.0"; // Replace with your filename
    int pixel_count = 0;

    // Open the file for reading
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    while (pixel_count < 320 * 240) {
        uint8_t high_byte, low_byte;
        if (fscanf(fp, "%hhu %hhu", &high_byte, &low_byte) != 2) {
            if (feof(fp)) {
                fprintf(stderr, "End of file reached before reading all pixels.\n");
                break;
            } else {
                perror("Error reading pixel data");
                fclose(fp);
                return 1;
            }
        }
        
        framebuffer[pixel_count++] = rgb565_mono(high_byte, low_byte);;
    }

    fclose(fp);

    if (pixel_count < 320 * 240) fprintf(stderr, "Warning: Only read %d out of %d expected pixels.\n", pixel_count, 320 * 240);
 
    stbi_write_bmp("out.bmp", 320, 240, 1, framebuffer);

  /* Process Frame */ {
struct Line line[2];
    float steering_value = 0;
  
    for (int y = 0+130; y < 240-15; y++) {
      // x: pixel id in scanline
      unsigned char *scanline = &framebuffer[y * 320];
      int mid = 0;

      // find lines
      struct Line tmp;
      memset(line, 0, sizeof(struct Line) * 2);
      
      for (int x = 0 + 16; x < 320 - 16; x++) {
        tmp.start = x, tmp.width = 0;

        while (x < 320 - 16 && scanline[x] < TRESHOLD_COLOR)
          tmp.width++, x++;

        if (MIN_WIDTH <= tmp.width && tmp.width <= MAX_WIDTH) {
          const uint8_t i = line[0].width > line[1].width;
            if (line[i].width < tmp.width) memcpy(&line[i], &tmp, sizeof(struct Line));
        }
      } // x check

      if (line[0].width && line[1].width) {
            mid = (line[0].start + line[0].width / 2 + line[1].start + line[1].width / 2) / 2;
      
        steering_value += (mid - 320/2);

        memset(scanline + mid - 5, 0, 10);
        memset(scanline + line[0].start, 255, line[0].width);
        memset(scanline + line[1].start, 255, line[1].width);
      }

    }

      printf("%f\n", steering_value);
    

  }
  
  stbi_write_bmp("processed.bmp", 320, 240, 1, framebuffer);

  return 0;
}
