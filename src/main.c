/* **** main.c ************************************************************** *
 * SoftRend3D                                                                 *
 * https://github.com/TurpeNescire/softrend-3d                                *
 * https://turpenescire.github.io/softrend-3d                                 *
 *                                                                            *
 * v0.1.1                                                                     *
 *                                                                            *
 * MIT License                                                                *
 *                                                                            *
 * Copyright (c) 2026 TurpeNescire                                            *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included    *
 * in all copies or substantial portions of the Software.                     *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS    *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF                 *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.     *
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY       *
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT  *
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR   *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                 *
 * ************************************************************************** */
#include "../include/fenster.h"

#define WIDTH  600
#define HEIGHT 400

#define FPS 60 // Targeted frame rate (frequency in hz)

// Framebuffer is row-major
#define PIXEL(x, y) buffer[(y) * WIDTH + (x)]

#define WHITE 0xFFFFFFFF

uint32_t buffer[WIDTH * HEIGHT];

// Draw a straight line from one endpoint in the buffer to another
// Does not do bounds checking - up to the caller to pass valid indices
void drawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    // Iterate over the axis with larger delta
    if (dx >= dy) {
        // More horizontal - iterate over x
        if (x0 > x1) { // make sure x0 is on the left
            int tmp = x0; x0 = x1; x1 = tmp;
            tmp = y0; y0 = y1; y1 = tmp;
        }
        dx = x1 - x0; dy = y1 - y0;
        for (int x = x0; x <= x1; x++) {
            float t = (float)(x - x0) / dx;
            int y = (int)(y0 + t * dy);
            PIXEL(x, y) = color;
        }
    } else {
        // More vertical - iterate over y
        if (y0 > y1) { // make sure y0 is higher
            int tmp = x0; x0 = x1; x1 = tmp;
            tmp = y0; y0 = y1; y1 = tmp;
        }
        dx = x1 - x0; dy = y1 - y0;
        for (int y = y0; y <= y1; y++) {
            float t = (float)(y - y0) / dy;
            int x = (int)(x0 + t * dx);
            PIXEL(x, y) = color;
        }
    }
}

int main()
{
    struct fenster window = {
        .title  = "Hello, World!",
        .width  = WIDTH,
        .height = HEIGHT,
        .buf    = buffer
    };

    // define's the vertices of a 300 pixel wide, 200 pixel high screen centered triangle
    int x0 = 150, y0 = 300; // bottom-left
    int x1 = 450, y1 = 300; // bottom-right
    int x2 = 300, y2 = 100; // apex
    drawLine(x0, y0, x1, y1, WHITE);
    drawLine(x1, y1, x2, y2, WHITE);
    drawLine(x2, y2, x0, y0, WHITE);

    // PIXEL(x0, y0) = WHITE;
    // PIXEL(x1, y1) = WHITE;
    // PIXEL(x2, y2) = WHITE;

    // Open a system window using the given window specifications
    if (fenster_open(&window) < 0) return 1;

    // Keeps track of the current second so we can print FPS at the end
    int64_t secondStartMS   = fenster_time();
    // Absolute target timestamp for next frame; advanced by exactly 1000/FPS
    // each iteration so sleep overshoots cancel out across frames
    double  nextFrameTime   = (double)secondStartMS;
    int     frameCount      = 0;
    while (fenster_loop(&window) == 0) {
        int64_t frameStartMS = fenster_time();
        frameCount++;

        // Time since last FPS print
        int64_t elapsedMS = frameStartMS - secondStartMS;
        if (elapsedMS >= 1000) {
            printf("fps: %.1f\n", frameCount * 1000.0f / elapsedMS);
            frameCount    = 0;
            secondStartMS = frameStartMS;
        }

        // sleep until we reach desired FRAME_TIME 
        nextFrameTime += 1000.0 / FPS;
        double remainingMS = nextFrameTime - (double)fenster_time();
        if (remainingMS > 0) fenster_sleep(remainingMS);
    }

    fenster_close(&window);
}
