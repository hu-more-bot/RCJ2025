#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Performance testing parameters
#define SAMPLES 1000       // Number of iterations for performance measurement
#define TEST_CASES 7       // Number of test images to process

// Image processing parameters
#define TRESHOLD_COLOR 100 // Minimum brightness value to consider as part of a line
#define MIN_WIDTH 2        // Minimum line width in pixels (inclusive)
#define MAX_WIDTH 40       // Maximum line width in pixels (inclusive)
#define MIDLINE_WIDTH 10   // Width of the midline to draw in pixels

// Utility macros for min/max operations
#define max(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define min(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

// Structure to represent a detected line
struct Line {
    int start;  // Starting x-position of the line
    int width;  // Width of the line in pixels
};

// Timer function for performance measurement
double now() {
    struct timespec res = {};
    clock_gettime(CLOCK_MONOTONIC, &res);
    double sec = (double)(1000.0f * (double)res.tv_sec + (double)res.tv_nsec / 1e6) / 1000.0;

    static double start = 0;
    if (start == 0) start = sec;
    return sec - start;
}


/* ===================== EVALUATION OF THE STEERING VALUE ============ */

// Parameters for the evaluation 
#define topCutoff 0.15
 
// the function that gives the value of the steering point at a y coordinate
// given as mid*(a*y^2+b*y+c)
// a or b should be 0 and the other should have a value 
// a=0 -> linear increase in offset b=0 -> exponensial increase in offset
#define A 1 
#define B 1
#define C 1

/**
 * gets the value of the steering offset
 * @param midPoint Midpoint where the line is
 * @param currentHeight the height its current (0 is the top height is the bottom)
 * @param pictureWidth The width of the picture
 * @param pictureHeight The height of the picture
 * @return value of the steering value that is to be added
 */
float EvaluateSteeringOffset(float midPoint, int currentHeight, int pictureWidth, int pictureHeight){
  if(currentHeight <= topCutoff * pictureHeight){
    return 0.0f;
  }
  float y = currentHeight - topCutoff*pictureHeight;

  return (A*y*y+B*y+C) * (midPoint - pictureWidth/2);
}

/* ================== IMAGE PROCESSING FUNCTIONS ================== */

/**
 * Converts a grayscale image to RGB format
 * @param gray Input grayscale image (1 channel)
 * @param width Image width
 * @param height Image height
 * @return Pointer to newly allocated RGB image (3 channels)
 */
unsigned char* convertGrayToRGB(const unsigned char* gray, int width, int height) {
    unsigned char* rgb = (unsigned char*)malloc(width * height * 3);
    for (int i = 0; i < width * height; i++) {
        rgb[i*3] = rgb[i*3+1] = rgb[i*3+2] = gray[i]; // Copy to all channels
    }
    return rgb;
}

/**
 * Detects lines in a single scanline (row) of the image
 * @param scanline Pointer to scanline data
 * @param width Width of the scanline
 * @param lines Array to store detected lines (size 2)
 * @return Midpoint between the two strongest detected lines
 */
int detectLinesInScanline(const unsigned char* scanline, int width, struct Line lines[2]) {
    struct Line tmp;
    int mid = 0;
    
    for (int x = 0; x < width; x++) {
        if (scanline[x] > TRESHOLD_COLOR) {
            // Find continuous bright pixels (potential line)
            tmp.start = x;
            tmp.width = 0;
            
            while (x < width && scanline[x] > TRESHOLD_COLOR) {
                tmp.width++;
                x++;
            }
            
            // Check if line meets size requirements
            if (MIN_WIDTH <= tmp.width && tmp.width <= MAX_WIDTH) {
                // Keep track of the two strongest lines
                const uint8_t i = lines[0].width > lines[1].width;
                if (lines[i].width < tmp.width) {
                    memcpy(&lines[i], &tmp, sizeof(struct Line));
                }
                mid = (lines[0].start + lines[1].start) / 2;
            }
        }
    }
    return mid;
}

/**
 * Draws a colored line on an RGB image
 * @param rgb RGB image data
 * @param width Image width
 * @param y Y-coordinate of the line
 * @param start Starting x-coordinate
 * @param length Length of the line to draw
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 */
void drawRGBLine(unsigned char* rgb, int width, int y, int start, int length, 
                unsigned char r, unsigned char g, unsigned char b) {
    for (int x = start; x < start + length && x < width; x++) {
        int idx = (y * width + x) * 3;
        rgb[idx] = r;
        rgb[idx+1] = g;
        rgb[idx+2] = b;
    }
}

/**
 * Draws the left border marker on an RGB image
 * @param rgb RGB image data
 * @param width Image width
 * @param height the height of the picture
 * @param y Y-coordinate of the line
 *
 */
void drawLeftBorderMarker(unsigned char* rgb, int width, int height,int y) {
    for (int x = 0; x < 3; x++) {
        int idx = (y * width + x) * 3;
        int val = 255 * (EvaluateSteeringOffset(1, y, width, height) / EvaluateSteeringOffset(1, height, width, height));
        rgb[idx] = rgb[idx+1] = rgb[idx+2] = val;
    }
}

/* ================== MAIN PROCESSING FUNCTION ================== */

/**
 * Processes a single test image
 * @param testCase Test case number (1-7)
 */
void processImage(int testCase) {
    // Generate input/output filenames
    char inputFilename[30], outputFilename[30];
    snprintf(inputFilename, sizeof(inputFilename), "test_images/test%d.png", testCase);
    snprintf(outputFilename, sizeof(outputFilename), "out/out%d.bmp", testCase);

    // Load grayscale image
    int width, height, channels;
    unsigned char* gray = stbi_load(inputFilename, &width, &height, &channels, 1);
    if (gray == NULL) {
        printf("Failed to load image %s\n", inputFilename);
        return;
    }

    // Convert to RGB for colored output
    unsigned char* rgb = convertGrayToRGB(gray, width, height);

    // Initialize processing variables
    struct Line lines[2] = {};
    float steering_value = 0;
    double start_time = now();

    // Main processing loop (with SAMPLES iterations for performance testing)
    for (int i = 0; i < SAMPLES; i++) {
        for (int y = 0; y < height; y++) {
            // Detect lines in current scanline
            int mid = detectLinesInScanline(&gray[y*width], width, lines);

            // Only process visualization on first iteration
            if (i == 0) {
              drawRGBLine(rgb, width, y, 0, width, 0,0,0);

                if (lines[0].width && lines[1].width) {
                    // Draw red midline
                    drawRGBLine(rgb, width, y, mid, MIDLINE_WIDTH, 255, 0, 0);
                    
                    // Update steering value based on line positions
                    steering_value += EvaluateSteeringOffset(mid, y, width, height);
                    
                    

                    // Highlight detected lines in white
                    drawRGBLine(rgb, width, y, lines[0].start, lines[0].width, 0, 255, 0);
                    drawRGBLine(rgb, width, y, lines[1].start, lines[1].width, 0, 0, 255);
                    
                    // Reset lines for next scanline
                    memset(lines, 0, sizeof(lines));
                }
                
                // Draw left border marker
                drawLeftBorderMarker(rgb, width, height, y);
            }
        }
    }

    // Calculate and print performance metrics
    double end_time = now();
    printf("Test case %d:\n", testCase);
    printf("  Avg time per iteration: %.6fs\n", (end_time - start_time) / SAMPLES);
    printf("  Steering value: %.2f\n", steering_value / height);

    // Save output image and clean up
    stbi_write_bmp(outputFilename, width, height, 3, rgb);
    free(gray);
    free(rgb);
}

/* ================== MAIN FUNCTION ================== */

int main() {
    printf("Starting image processing for %d test cases...\n", TEST_CASES);
    for (int i = 1; i <= TEST_CASES; i++) {
        processImage(i);
    }
    printf("Processing complete.\n");
    return 0;
}