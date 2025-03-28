#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SAMPLES 1000

#define TRESHOLD_COLOR 100
#define MIN_WIDTH 2 // (px)
#define MAX_WIDTH 24 // (px)
// note: limits are included limits

double now() {
    struct timespec res = {};
    clock_gettime(CLOCK_MONOTONIC, &res);
    double sec = (double)(1000.0f * (double)res.tv_sec + (double)res.tv_nsec / 1e6) / 1000.0;

    static double start = 0;
    if (start == 0)
        start = sec;
    return sec - start;
}

struct Line {
  int start, width;
};

int main() {
  int w, h, ch;
  unsigned char *frame = stbi_load("test.png", &w, &h, &ch, 1);

  if (frame == NULL) {
    printf("%ix%i %i\n", w, h, ch);
    return 1;
  }

  // Process Frame
  struct Line line[2];

  double start = now();
  
  for (int i = 0; i < SAMPLES; i++) {
    struct Line tmp;
    for (int y = 0; y < h; y++) {
      memset(line, 0, sizeof(struct Line) * 2);
            
      // x: pixel id in scanline
      unsigned char *scanline = &frame[y * w];

      for (int x = 0; x < w; x++) {
        if (scanline[x] > TRESHOLD_COLOR) {
          tmp.start = x, tmp.width = 0;
            
          while (x < w && scanline[x] > TRESHOLD_COLOR)
            tmp.width++, x++;

          if (MIN_WIDTH <= tmp.width && tmp.width <= MAX_WIDTH) {
            const uint8_t i = line[0].width < line[1].width;
            if (line[i].width < tmp.width) memcpy(&line[i], &tmp, sizeof(struct Line));
          }
        }
      }

      if (line[0].width && line[1].width) {
        printf("%u %u\n", line[0].start, line[1].start);
        memset(scanline + line[0].start, 255, line[0].width);
        memset(scanline + line[1].start, 255, line[1].width);
      }
    }
  }

  double end = now();

  printf("total: %f\n", (double)(end - start) / (double)SAMPLES);

  stbi_write_bmp("out.bmp", w, h, 1, frame);

  return 0;
}
