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
// TODO: for performance we can switch to Bresenham's later if needed
//       or use point-slope and cache slope once as dy/dx and using
//       y = topY + (x - leftX) * slope
void drawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    // Degenerate case: both endpoints identical, draw a single pixel and bail
    if (dx == 0 && dy == 0) { PIXEL(x0, y0) = color; return; }

    // Sort by the dominant axis
    // x-dominant: ensure leftX < rightX
    // y-dominant: ensure topY < bottomY
    // x and y must swap together — a partial swap mixes coordinates between the endpoints
    int xDominant = (dx >= dy);
    int leftX = x0, rightX  = x1;
    int topY  = y0, bottomY = y1;
    if (xDominant ? leftX > rightX : topY > bottomY) { 
        leftX = x1; rightX  = x0;
        topY  = y1; bottomY = y0;
    }

    if (xDominant) {
        dy = bottomY - topY; // recompute minor-axis delta from the sorted endpoints
        for (int x = leftX; x <= rightX; x++) {
            float t = (float)(x - leftX) / dx; // progress along x: 0 at leftX, 1 at rightX
            int y = (int)(topY + t * dy);      // interpolated y at this x
            PIXEL(x, y) = color;
        }
    } else {
        dx = rightX - leftX; // recompute minor-axis delta from the sorted endpoints
        for (int y = topY; y <= bottomY; y++) {
            float t = (float)(y - topY) / dy; // progress along y: 0 at topY, 1 at bottomY
            int x = (int)(leftX + t * dx);    // interpolated x at this y
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

    // vertices of a 100 pixel wide, 300 pixel tall screen centered triangle
    int x0 = 250, y0 = 350; // bottom-left
    int x1 = 350, y1 = 350; // bottom-right
    int x2 = 300, y2 =  50; // apex

    drawLine(x0, y0, x1, y1, WHITE);
    drawLine(x1, y1, x2, y2, WHITE);
    drawLine(x2, y2, x0, y0, WHITE);
    
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
