#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define IMAGE_WIDTH 450
#define IMAGE_HEIGHT 291

#define TRESHOLD_COLOR 100
#define MIN_WIDTH 2 // (px)
#define MAX_WIDTH 24 // (px)
// note: limits are included limits

#define PX(x, y) ((y) * IMAGE_WIDTH + (x))

// TODO
// save pos at the end of the scanlineline
// start from prev scanline's line pos -> speed
// apply tactics for frame-to-frame detection as well

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
  // float frame[IMAGE_HEIGHT][IMAGE_WIDTH]; // intensity percentage 0->1 (float)

  int w, h, ch;
  unsigned char *frame = stbi_load("test.png", &w, &h, &ch, 1);

  if (frame == NULL) {
    printf("%ix%i %i\n", w, h, ch);
    return 1;
  }
  
  // Prep Frame
  srand(time(0));
  
  // for (int y = 0; y < IMAGE_HEIGHT; y++)
  //   for (int x = 0; x < IMAGE_WIDTH; x++)
  //     frame[PX(x, y)] = (float)(rand() % 100000) / 100000.0f;

  // Process Frame
  struct Line line[2];

  double start = now();
  int l, r; // line memory
  
  for (int i = 0; i < 1000; i++) {
    struct Line tmp;
    for (int y = h; y > 0; y--) {
      // x: pixel id in scanline
      unsigned char *scanline = &frame[(h - y - 1) * w];

      if (y == h || (l <= 0 || r <= 0) || (scanline[l] <= TRESHOLD_COLOR || scanline[r] <= TRESHOLD_COLOR)) {
        for (int x = 0; x < w; x++) {
    //  // x: pixel id in scanline
      //    const unsigned char *scanline = &frame[y * w];
          
          if (scanline[x] > TRESHOLD_COLOR) {
            tmp.start = x, tmp.width = 0;
            
            while (x < w && scanline[x] > TRESHOLD_COLOR)
              tmp.width++, x++;

            if (MIN_WIDTH <= tmp.width && tmp.width <= MAX_WIDTH) {
              if (line[0].width < line[1].width) {
                if (line[0].width < tmp.width) memcpy(&line[0], &tmp, sizeof(struct Line));
              } else {
                if (line[1].width < tmp.width) memcpy(&line[1], &tmp, sizeof(struct Line));
              }
            }
          }
        }
      } else {
        line[0].start = l;
        int start = l;
        while (start > 0 && scanline[start] > TRESHOLD_COLOR)
          start++;
        line[0].start = start;

        if (line[0].start != 0) {
          int end = l;
          while (end < w && scanline[end] > TRESHOLD_COLOR)
            end++;
          line[0].width = end - line[0].start;
        }

        line[1].start = r;
        while (line[1].start > 0 && scanline[line[1].start] > TRESHOLD_COLOR)
          line[1].start++;

        if (line[1].start != 0) {
          line[1].width = r - line[1].start;
          while (line[1].start + line[1].width < w && scanline[line[1].start + line[1].width] > TRESHOLD_COLOR)
            line[1].width++;
        }
      }

      // TODO Line-to-line memory

      const int w = 10;
      //memset(scanline + line[0].start + line[0].width / 2, 255, 2);
      if (line[0].width && line[1].width) {
        memset(scanline + line[0].start, 255, line[0].width);
        memset(scanline + line[1].start, 255, line[1].width);
      }
      
      if (line[0].width > 0 && line[1].width > 0 && line[0].start > 0 && line[1].start > 0) {
        l = line[0].start + line[0].width / 2, r = line[1].start + line[1].width / 2;
      }

      // if (line[0].width || line[1].width) {
      //   printf("Line %i\n", y);
      //   printf("Starts:   %i %i\n", line[0].start, line[1].start);
      //   printf("Widths:   %i %i\n", line[0].width, line[1].width);
      // }
      
      memset(line, 0, sizeof(struct Line) * 2);
    }
  }

  double end = now();

  printf("total: %f\n", (double)(end - start) / 1000.0);

  stbi_write_bmp("out.bmp", IMAGE_WIDTH, IMAGE_HEIGHT, 1, frame);

  return 0;
}
