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

typedef enum {
    BLACK, WHITE, RED, GREEN, BLUE,
    YELLOW, CYAN, MAGENTA,
    GRAY, SILVER,
    ORANGE, PURPLE, MAROON, NAVY, TEAL,
    COLOR_COUNT
} Color;

uint32_t colors[] = {
    0xFF000000, // BLACK
    0xFFFFFFFF, // WHITE
    0xFFFF0000, // RED
    0xFF00FF00, // GREEN
    0xFF0000FF, // BLUE
    0xFFFFFF00, // YELLOW
    0xFF00FFFF, // CYAN
    0xFFFF00FF, // MAGENTA
    0xFF808080, // GRAY
    0xFFC0C0C0, // SILVER
    0xFFFF8000, // ORANGE
    0xFF800080, // PURPLE
    0xFF800000, // MAROON
    0xFF000080, // NAVY
    0xFF008080, // TEAL
};

uint32_t buffer[WIDTH * HEIGHT];

float LERP_INT_ROUNDING = 0.5f;

// Draw a straight line from one endpoint in the buffer to another
// Does not do bounds checking - up to the caller to pass valid indices
// TODO: for performance we can switch to Bresenham's later if needed
//       or use point-slope and cache slope once as dy/dx and using
//       y = topY + (x - leftX) * slope for each pixel
void drawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    // Degenerate case: both endpoints identical, draw a single pixel and bail
    // dx == 0 alone is not sufficient — a vertical line has dx == 0 but dy > 0
    // and draws correctly via the y-branch.
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
            float t = (float)(x - leftX) / dx;                // progress along x: 0 at leftX, 1 at rightX
            int y = (int)(topY + t * dy + LERP_INT_ROUNDING); // interpolated y at this x, rounded
            PIXEL(x, y) = color;
        }
    } else {
        dx = rightX - leftX; // recompute minor-axis delta from the sorted endpoints
        for (int y = topY; y <= bottomY; y++) {
            float t = (float)(y - topY) / dy;     // progress along y: 0 at topY, 1 at bottomY
            int x = (int)(leftX + t * dx + LERP_INT_ROUNDING); // interpolated x at this y, rounded
            PIXEL(x, y) = color;
        }
    }
}

// Returns 1 if the application should quit, 0 otherwise
int handleInput(struct fenster *f) {
    if (f->keys[27]) return 1;                  // Escape
    if (f->keys['Q'] && (f->mod & 8)) return 1; // Cmd+Q  (macOS)
    if (f->keys['Q'] && (f->mod & 1)) return 1; // Ctrl+Q (Windows/Linux)

    return 0;
}

int main()
{
    struct fenster window = {
        .title  = "Hello, World!",
        .width  = WIDTH,
        .height = HEIGHT,
        .buf    = buffer
    };
    
    // x-dominant (wide/flat)        — top-left cell
    drawLine( 20, 170, 180, 170, colors[RED]);
    drawLine(180, 170, 100,  30, colors[RED]);
    drawLine(100,  30,  20, 170, colors[RED]);

    // y-dominant (tall/narrow)      — top-middle cell
    drawLine(270, 180, 330, 180, colors[GREEN]);
    drawLine(330, 180, 300,  20, colors[GREEN]);
    drawLine(300,  20, 270, 180, colors[GREEN]);

    // one vertical edge              — top-right cell
    drawLine(410,  20, 410, 180, colors[BLUE]);
    drawLine(410, 180, 580, 100, colors[BLUE]);
    drawLine(580, 100, 410,  20, colors[BLUE]);

    // one horizontal edge            — bottom-left cell
    drawLine( 20, 210, 180, 210, colors[YELLOW]);
    drawLine(180, 210, 100, 380, colors[YELLOW]);
    drawLine(100, 380,  20, 210, colors[YELLOW]);

    // right triangle                 — bottom-middle cell
    drawLine(220, 220, 220, 380, colors[CYAN]);
    drawLine(220, 380, 380, 380, colors[CYAN]);
    drawLine(380, 380, 220, 220, colors[CYAN]);

    // two coincident vertices        — bottom-right cell
    drawLine(500, 220, 500, 220, colors[MAGENTA]);
    drawLine(500, 220, 420, 370, colors[MAGENTA]);
    drawLine(420, 370, 500, 220, colors[MAGENTA]);

    // single point — tucked in corner of bottom-right cell
    drawLine(570, 370, 570, 370, colors[ORANGE]);
    drawLine(570, 370, 570, 370, colors[ORANGE]);
    drawLine(570, 370, 570, 370, colors[ORANGE]);

    // Open a system window using the given window specifications
    if (fenster_open(&window) < 0) return 1;

    // Keeps track of the current second so we can print FPS at the end
    int64_t secondStartMS   = fenster_time();
    // Absolute target timestamp for next frame; advanced by exactly 1000/FPS
    // each iteration so sleep overshoots cancel out across frames
    double  nextFrameTime   = (double)secondStartMS;
    int     frameCount      = 0;
    while (fenster_loop(&window) == 0) {
        frameCount++;

        // Time since last FPS print
        int64_t elapsedMS = fenster_time() - secondStartMS;
        if (elapsedMS >= 1000) {
            printf("fps: %.1f\n", frameCount * 1000.0f / elapsedMS);
            frameCount    = 0;
            secondStartMS = fenster_time();
        }

        if (handleInput(&window)) break;

        // Sleep until we reach desired frame time
        // TODO: if nextFrameTime falls far behind due to program stall, clamp it
        //       forward instead of spinning no-sleep frames to catch up
        nextFrameTime += 1000.0 / FPS;
        double remainingMS = nextFrameTime - (double)fenster_time();
        if (remainingMS > 0) fenster_sleep(remainingMS);
    }

    fenster_close(&window);
}
