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

typedef struct Vec2 {
    int x;
    int y;
} Vec2;

typedef struct Triangle {
    Vec2 v0;
    Vec2 v1;
    Vec2 v2;
} Triangle;

// Utility math functions
static inline int int_max(int x, int y) {
    return (x > y) ? x : y;
}

static inline int int_min(int x, int y) {
    return (x > y) ? y : x;
}

// Draw a straight line from one endpoint in the buffer to another
// Does not do bounds checking - up to the caller to pass valid indices
// TODO: for performance we can switch to Bresenham's later if needed
//       or use point-slope and cache slope once as dy/dx and using
//       y = topY + (x - leftX) * slope for each pixel
void drawLine(const Vec2 *v0, const Vec2 *v1, uint32_t color) {
    int x0 = v0->x, y0 = v0->y;
    int x1 = v1->x, y1 = v1->y;

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
            float t = (float)(x - leftX) / dx;   // progress along x: 0 at leftX, 1 at rightX
            int y = (int)(topY + t * dy + 0.5f); // interpolated y at this x, rounded
            PIXEL(x, y) = color;
        }
    } else {
        dx = rightX - leftX; // recompute minor-axis delta from the sorted endpoints
        for (int y = topY; y <= bottomY; y++) {
            float t = (float)(y - topY) / dy;     // progress along y: 0 at topY, 1 at bottomY
            int x = (int)(leftX + t * dx + 0.5f); // interpolated x at this y, rounded
            PIXEL(x, y) = color;
        }
    }
}

// Takes a triangle's edge as two Vec2 values, tests if the line crosses the current scanline.
// Returns true and sets *x to that value if so, otherwise returns false and *x has a garbage value.
static inline bool crossesScanlineAt(const Vec2 *v0, const Vec2 *v1, int scanline_y, int *x) {
    int dy = v1->y - v0->y;
    if (dy == 0) return false; // The edge is a horizontal line or coincident - no crossing
    if (scanline_y < int_min(v0->y, v1->y) || scanline_y > int_max(v0->y, v1->y))
        return false; // Ensure that the current scanline is between the two edge vertices
    int dx = v1->x - v0->x;
    float t = (float)(scanline_y - v0->y) / (float)dy; // Progress along y: 0 at v0->y, 1 at v1->y
    *x = (int)(v0->x + t * dx + 0.5f); // Interpolated x at this y, rounded
    return true;
}

void drawTriangle(const Triangle *tri, uint32_t color) {
    const Vec2 *a = &tri->v0, *b = &tri->v1, *c = &tri->v2;

    int top_y = int_min(a->y, int_min(b->y, c->y));
    int bot_y = int_max(a->y, int_max(b->y, c->y));

    for (int scanline_y = top_y; scanline_y <= bot_y; scanline_y++) {
        int left_x = INT_MAX, right_x = INT_MIN, crossing_x;

        if (crossesScanlineAt(a, b, scanline_y, &crossing_x)) {
            if (crossing_x < left_x) left_x = crossing_x;
            if (crossing_x > right_x) right_x = crossing_x;
        }
        if (crossesScanlineAt(b, c, scanline_y, &crossing_x)) {
            if (crossing_x < left_x) left_x = crossing_x;
            if (crossing_x > right_x) right_x = crossing_x;
        }
        if (crossesScanlineAt(c, a, scanline_y, &crossing_x)) {
            if (crossing_x < left_x) left_x = crossing_x;
            if (crossing_x > right_x) right_x = crossing_x;
        }

        if (left_x == INT_MAX || right_x == INT_MIN) {
            printf("drawTriangle: skipping triangle, left_x = %d, right_x = %d\n", left_x, right_x);
        }
        for (int scanline_x = left_x; scanline_x <= right_x; scanline_x++) {
            PIXEL(scanline_x, scanline_y) = color;
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
    
    // x-dominant (wide/flat)   — top-left cell
    Triangle tri0 = { { .x =  20, .y = 170 },
                      { .x = 180, .y = 170 },
                      { .x = 100, .y =  30 } };
    // y-dominant (tall/narrow) — top-middle cell
    Triangle tri1 = { { .x = 270, .y = 180 },
                      { .x = 330, .y = 180 },
                      { .x = 300, .y =  20 } };
    // one vertical edge        — top-right cell
    Triangle tri2 = { { .x = 410, .y =  20 },
                      { .x = 410, .y = 180 },
                      { .x = 580, .y = 100 } };
    // one horizontal edge      — bottom-left cell
    Triangle tri3 = { { .x =  20, .y = 210 },
                      { .x = 180, .y = 210 },
                      { .x = 100, .y = 380 } };
    // right triangle           — bottom-middle cell
    Triangle tri4 = { { .x = 220, .y = 220 },
                      { .x = 220, .y = 380 },
                      { .x = 380, .y = 380 } };
    // two coincident vertices  — bottom-right cell
    Triangle tri5 = { { .x = 500, .y = 220 },
                      { .x = 500, .y = 220 },
                      { .x = 420, .y = 370 } };
    // single point — tucked in corner of bottom-right cell
    Triangle tri6 = { { .x = 570, .y = 370 },
                      { .x = 570, .y = 370 },
                      { .x = 570, .y = 370 } };
    // near-horizontal sliver
    Triangle tri7 = { { .x =  20, .y = 195 },
                      { .x = 180, .y = 195 },
                      { .x = 100, .y = 194 } };
    // near-vertical sliver
    Triangle tri8 = { { .x = 200, .y = 210 },
                      { .x = 200, .y = 380 },
                      { .x = 201, .y = 295 } };
    // horizontal line degenerate
    Triangle tri9 = { { .x = 240, .y = 200 },
                      { .x = 310, .y = 200 },
                      { .x = 380, .y = 201 } };
    // vertical line degenerate
    Triangle tri10 = { { .x = 400, .y = 220 },
                       { .x = 400, .y = 380 },
                       { .x = 400, .y = 300 } };
    drawTriangle(&tri0, colors[RED]);
    drawTriangle(&tri1, colors[GREEN]);
    drawTriangle(&tri2, colors[BLUE]);
    drawTriangle(&tri3, colors[YELLOW]);
    drawTriangle(&tri4, colors[CYAN]);
    drawTriangle(&tri5, colors[MAGENTA]);
    drawTriangle(&tri6, colors[ORANGE]);
    // drawTriangle(&tri7, colors[PURPLE]);
    // drawTriangle(&tri8, colors[WHITE]);
    // drawTriangle(&tri9, colors[GRAY]);
    // drawTriangle(&tri10, colors[SILVER]);

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
