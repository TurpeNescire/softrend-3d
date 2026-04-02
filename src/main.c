/* **** main.c ************************************************************** *
 * SoftRend3D                                                                 *
 * https://github.com/TurpeNescire/softrend-3d                                *
 * https://turpenescire.github.io/softrend-3d                                 *
 *                                                                            *
 * v0.1.3                                                                     *
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
#include <stdint.h>  // uint32_t, int64_t types
#include <stdbool.h> // bool
#include <limits.h>  // INT_MIN/MAX — sentinel values for span initialization in drawTriangle
#include <stdlib.h>  // abs()
#include <stdio.h>   // printf()

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

typedef struct Vec2     { int x; int y; }             Vec2;
typedef struct Triangle { Vec2 v0; Vec2 v1; Vec2 v2; } Triangle;

// Animation fill cursor: drawTriangle skips pixels beyond this point.
// Pixels at scan_y > g_anim_fill_y are not drawn.
// On scan_y == g_anim_fill_y, pixels at scan_x > g_anim_fill_x are not drawn.
static int g_anim_fill_y = -1;
static int g_anim_fill_x = WIDTH - 1;

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

// Takes a triangle's edge as two Vec2 values, tests if the edge crosses the current scanline.
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

static inline void updateSpan(int x, int *left_x, int *right_x) {
    if (x < *left_x)  *left_x  = x;
    if (x > *right_x) *right_x = x;
}

// Draws the Triangle filled with the ARGB format value in color. Triangles with
// 0 area (degenerate) are skipped. Calls crossesScanlineAt for each triangle
// edge to find if and where it crosses the current scanline, then fills
// between those edges if they exist.
// Fill is clipped to the global animation cursor (g_anim_fill_y, g_anim_fill_x).
// TODO: Calculate slope for each edge once and pass to crossesScanlineAt()
void drawTriangle(const Triangle *tri, uint32_t color) {
    const Vec2 *a = &tri->v0, *b = &tri->v1, *c = &tri->v2;

    // The cross product for 2D vectors has only a z-component: (0, 0, (b-a)×(c-a)).
    // That z value equals the signed parallelogram area spanned by u and v.
    // A value of zero means no area — no triangle to draw.
    // 2D cross product formula: u × v = (0, 0, u.x*v.y - u.y*v.x)
    int crossProductZ = ((b->x - a->x) * (c->y - a->y)) - ((b->y - a->y) * (c->x - a->x));
    if (crossProductZ == 0) return;

    // Find the vertical span of the triangle, clamp to vertical screen bounds
    int top_y = int_max(int_min(a->y, int_min(b->y, c->y)), 0);
    int bot_y = int_min(int_max(a->y, int_max(b->y, c->y)), HEIGHT - 1);

    // Loop over each scanline in the vertical span, find then fill its horizontal span
    for (int scan_y = top_y; scan_y <= bot_y; scan_y++) {
        if (scan_y > g_anim_fill_y) break; // animation cursor: skip everything below

        int left_x = INT_MAX, right_x = INT_MIN, x;

        if (crossesScanlineAt(a, b, scan_y, &x)) updateSpan(x, &left_x, &right_x);
        if (crossesScanlineAt(b, c, scan_y, &x)) updateSpan(x, &left_x, &right_x);
        if (crossesScanlineAt(c, a, scan_y, &x)) updateSpan(x, &left_x, &right_x);

        // On the cursor scanline, fill only up to g_anim_fill_x; above it, fill the whole span
        int x_limit = (scan_y == g_anim_fill_y) ? g_anim_fill_x : WIDTH - 1;
        for (int scan_x = int_max(left_x, 0); scan_x <= int_min(right_x, int_min(WIDTH - 1, x_limit)); scan_x++) {
            PIXEL(scan_x, scan_y) = color;
        }
    }
}

// Draws the three edges of a triangle as lines. Since the framebuffer is redrawn
// from scratch each frame, calling this after drawTriangle keeps the wireframe
// visible on top of any fill that has been drawn so far.
static void drawWireframe(const Triangle *tri, uint32_t color) {
    drawLine(&tri->v0, &tri->v1, color);
    drawLine(&tri->v1, &tri->v2, color);
    drawLine(&tri->v2, &tri->v0, color);
}

// Draws a line clipped to y <= max_y. Used to overwrite the upper portion of a
// white wireframe with the triangle's own color as the fill cursor descends.
static void drawLineYClipped(const Vec2 *v0, const Vec2 *v1, uint32_t color, int max_y) {
    int x0 = v0->x, y0 = v0->y;
    int x1 = v1->x, y1 = v1->y;

    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    if (dx == 0 && dy == 0) {
        if (y0 <= max_y) PIXEL(x0, y0) = color;
        return;
    }

    int xDominant = (dx >= dy);
    int leftX = x0, rightX  = x1;
    int topY  = y0, bottomY = y1;
    if (xDominant ? leftX > rightX : topY > bottomY) {
        leftX = x1; rightX  = x0;
        topY  = y1; bottomY = y0;
    }

    if (xDominant) {
        dy = bottomY - topY;
        for (int x = leftX; x <= rightX; x++) {
            float t = (float)(x - leftX) / dx;
            int y = (int)(topY + t * dy + 0.5f);
            if (y <= max_y) PIXEL(x, y) = color;
        }
    } else {
        dx = rightX - leftX;
        for (int y = topY; y <= int_min(bottomY, max_y); y++) {
            float t = (float)(y - topY) / dy;
            int x = (int)(leftX + t * dx + 0.5f);
            PIXEL(x, y) = color;
        }
    }
}

// Draws the wireframe edges clipped to y <= max_y in the given color.
// Called after drawWireframe (white) to overwrite only the scanned portion.
static void drawWireframeYClipped(const Triangle *tri, uint32_t color, int max_y) {
    drawLineYClipped(&tri->v0, &tri->v1, color, max_y);
    drawLineYClipped(&tri->v1, &tri->v2, color, max_y);
    drawLineYClipped(&tri->v2, &tri->v0, color, max_y);
}

// Returns 1 if the application should quit, 0 otherwise
int handleInput(struct fenster *f) {
    if (f->keys[27]) return 1;                  // Escape
    if (f->keys['Q'] && (f->mod & 8)) return 1; // Cmd+Q  (macOS)
    if (f->keys['Q'] && (f->mod & 1)) return 1; // Ctrl+Q (Windows/Linux)

    return 0;
}

// Animation phases:
//   WIREFRAME — hold showing only triangle outlines
//   FAST      — advance the fill cursor ~60 scanlines/sec
//   SLOW      — hold the cursor on one scanline and advance it 3px/frame (~3s per line)
//   DONE      — hold the fully filled image, then restart
typedef enum { ANIM_WIREFRAME, ANIM_FAST, ANIM_SLOW, ANIM_DONE } AnimPhase;

int main()
{
    struct fenster window = {
        .title  = "Hello, World!",
        .width  = WIDTH,
        .height = HEIGHT,
        .buf    = buffer
    };

    // x-dominant (wide/flat)      — top-left cell
    Triangle tri0  = { { .x =  20, .y = 170 },   // bottom-left
                       { .x = 180, .y = 170 },   // bottom-right
                       { .x = 100, .y =  30 } }; // top-middle
    // y-dominant (tall/narrow)    — top-middle cell
    Triangle tri1  = { { .x = 270, .y = 180 },   // bottom-left
                       { .x = 330, .y = 180 },   // bottom-right
                       { .x = 300, .y =  20 } }; // top-middle
    // one vertical edge           — top-right cell
    Triangle tri2  = { { .x = 410, .y =  20 },   // top-left
                       { .x = 410, .y = 180 },   // bottom-left
                       { .x = 580, .y = 100 } }; // right-middle
    // one horizontal edge         — bottom-left cell
    Triangle tri3  = { { .x =  20, .y = 210 },   // bottom-left
                       { .x = 100, .y = 380 },   // top-middle
                       { .x = 180, .y = 210 } }; // bottom-right
    // right triangle              — bottom-middle cell
    Triangle tri4  = { { .x = 220, .y = 220 },   // top-left
                       { .x = 220, .y = 380 },   // bottom-left
                       { .x = 380, .y = 380 } }; // bottom-right
    // degenerate: two coincident vertices  — bottom-right cell
    Triangle tri5  = { { .x = 500, .y = 220 },
                       { .x = 500, .y = 220 },
                       { .x = 420, .y = 370 } };
    // degenerate: single point    — tucked in corner of bottom-right cell
    Triangle tri6  = { { .x = 570, .y = 370 },
                       { .x = 570, .y = 370 },
                       { .x = 570, .y = 370 } };
    // near-horizontal sliver      — left-side middle
    Triangle tri7  = { { .x =  20, .y = 195 },   // left
                       { .x = 180, .y = 195 },   // right
                       { .x = 180, .y = 194 } }; // 1 pixel above right
    // near-vertical sliver        — between yellow and cyan tris
    Triangle tri8  = { { .x = 200, .y = 210 },   // top
                       { .x = 200, .y = 380 },   // bottom
                       { .x = 201, .y = 295 } }; // middle, 1 pixel right
    // degenerate: horizontal line
    Triangle tri9  = { { .x = 240, .y = 200 },
                       { .x = 310, .y = 200 },
                       { .x = 380, .y = 200 } };
    // degenerate: vertical line
    Triangle tri10 = { { .x = 400, .y = 220 },
                       { .x = 400, .y = 380 },
                       { .x = 400, .y = 300 } };

    // Scanlines at which to pause for the slow fill. Each stop fills 4 consecutive
    // scanlines at 3px/frame (~3.3s each, ~13s per stop):
    //   y=100: mid-way through the top triangle group
    //   y=300: mid-way through the bottom triangle group
    static const int SLOW_AT[]           = {100, 300};
    static const int SLOW_COUNT          = 2;
    static const int SLOW_LINES_PER_STOP = 4; // scanlines filled slowly per stop

    AnimPhase phase           = ANIM_WIREFRAME;
    int       slow_idx        = 0;    // which slow scanline we're currently on
    int       slow_lines_done = 0;    // how many scanlines filled at the current stop
    float     line_accum      = 0.0f; // fractional line accumulator for smooth fast fill speed
    int       hold_frames     = 0;    // frame counter used in WIREFRAME and DONE phases

    if (fenster_open(&window) < 0) return 1;

    int64_t secondStartMS = fenster_time();
    double  nextFrameTime = (double)secondStartMS;
    int     frameCount    = 0;

    while (fenster_loop(&window) == 0) {
        frameCount++;

        int64_t elapsedMS = fenster_time() - secondStartMS;
        if (elapsedMS >= 1000) {
            printf("fps: %.1f\n", frameCount * 1000.0 / elapsedMS);
            frameCount    = 0;
            secondStartMS = fenster_time();
        }

        if (handleInput(&window)) break;

        // ── Advance animation state ──────────────────────────────────────────
        switch (phase) {

            case ANIM_WIREFRAME:
                // Hold wireframe-only for ~2.5s, then begin filling
                hold_frames++;
                if (hold_frames >= 150) {
                    phase         = ANIM_FAST;
                    hold_frames   = 0;
                    g_anim_fill_y = 0;
                    g_anim_fill_x = WIDTH - 1;
                }
                break;

            case ANIM_FAST: {
                // Fill ~60 scanlines/sec (1 line per frame at 60fps).
                // Stop at the next slow-line target, or HEIGHT-1 if all slow lines are done.
                int target = (slow_idx < SLOW_COUNT) ? SLOW_AT[slow_idx] : HEIGHT - 1;
                line_accum += 1.0f;
                while (line_accum >= 1.0f && g_anim_fill_y < target) {
                    g_anim_fill_y++;
                    line_accum -= 1.0f;
                }
                if (g_anim_fill_y >= target) {
                    g_anim_fill_y = target;
                    if (slow_idx < SLOW_COUNT) {
                        phase           = ANIM_SLOW;
                        g_anim_fill_x   = 0;
                        slow_lines_done = 0;
                        line_accum      = 0.0f;
                    } else {
                        phase       = ANIM_DONE;
                        hold_frames = 0;
                    }
                }
                break;
            }

            case ANIM_SLOW:
                // Fill the current scanline 3 pixels per frame — 600px / 3px = ~200 frames = ~3.3s per line
                g_anim_fill_x += 3;
                if (g_anim_fill_x >= WIDTH) {
                    g_anim_fill_x = WIDTH - 1;
                    slow_lines_done++;
                    g_anim_fill_y++;
                    if (g_anim_fill_y > HEIGHT - 1) g_anim_fill_y = HEIGHT - 1;
                    if (slow_lines_done >= SLOW_LINES_PER_STOP) {
                        // All lines for this stop done: return to fast fill
                        slow_idx++;
                        phase           = ANIM_FAST;
                        slow_lines_done = 0;
                        line_accum      = 0.0f;
                    } else {
                        g_anim_fill_x = 0; // start next slow scanline
                    }
                }
                break;

            case ANIM_DONE:
                // Hold the fully filled image for ~3s, then loop
                hold_frames++;
                if (hold_frames >= 180) {
                    phase           = ANIM_WIREFRAME;
                    slow_idx        = 0;
                    slow_lines_done = 0;
                    line_accum      = 0.0f;
                    hold_frames     = 0;
                    g_anim_fill_y   = -1;
                    g_anim_fill_x   = WIDTH - 1;
                }
                break;
        }

        // ── Draw ─────────────────────────────────────────────────────────────

        // Clear framebuffer to black each frame — fill and wireframe are redrawn from scratch
        for (int i = 0; i < WIDTH * HEIGHT; i++) buffer[i] = colors[BLACK];

        // Draw filled triangles clipped to the animation cursor
        drawTriangle(&tri0,  colors[RED]);
        drawTriangle(&tri1,  colors[GREEN]);
        drawTriangle(&tri2,  colors[BLUE]);
        drawTriangle(&tri3,  colors[YELLOW]);
        drawTriangle(&tri4,  colors[CYAN]);
        drawTriangle(&tri5,  colors[MAGENTA]);
        drawTriangle(&tri6,  colors[ORANGE]);
        drawTriangle(&tri7,  colors[PURPLE]);
        drawTriangle(&tri8,  colors[WHITE]);
        drawTriangle(&tri9,  colors[GRAY]);
        drawTriangle(&tri10, colors[SILVER]);

        // Draw wireframes in two passes. First pass: full outline in white, visible
        // against both the black background and the fill colors. Second pass: overwrite
        // only the portion above the fill cursor (y <= g_anim_fill_y) in the triangle's
        // own color, so the outline progressively transitions from white to colored as
        // the scanline sweeps through each triangle.
        drawWireframe(&tri0,  colors[WHITE]);
        drawWireframe(&tri1,  colors[WHITE]);
        drawWireframe(&tri2,  colors[WHITE]);
        drawWireframe(&tri3,  colors[WHITE]);
        drawWireframe(&tri4,  colors[WHITE]);
        drawWireframe(&tri5,  colors[WHITE]);
        drawWireframe(&tri6,  colors[WHITE]);
        drawWireframe(&tri7,  colors[WHITE]);
        drawWireframe(&tri8,  colors[WHITE]);
        drawWireframe(&tri9,  colors[WHITE]);
        drawWireframe(&tri10, colors[WHITE]);

        drawWireframeYClipped(&tri0,  colors[RED],     g_anim_fill_y);
        drawWireframeYClipped(&tri1,  colors[GREEN],   g_anim_fill_y);
        drawWireframeYClipped(&tri2,  colors[BLUE],    g_anim_fill_y);
        drawWireframeYClipped(&tri3,  colors[YELLOW],  g_anim_fill_y);
        drawWireframeYClipped(&tri4,  colors[CYAN],    g_anim_fill_y);
        drawWireframeYClipped(&tri5,  colors[MAGENTA], g_anim_fill_y);
        drawWireframeYClipped(&tri6,  colors[ORANGE],  g_anim_fill_y);
        drawWireframeYClipped(&tri7,  colors[PURPLE],  g_anim_fill_y);
        drawWireframeYClipped(&tri8,  colors[WHITE],   g_anim_fill_y);
        drawWireframeYClipped(&tri9,  colors[GRAY],    g_anim_fill_y);
        drawWireframeYClipped(&tri10, colors[SILVER],  g_anim_fill_y);

        // Sleep until we reach desired frame time
        // TODO: if nextFrameTime falls far behind due to program stall, clamp it
        //       forward instead of spinning no-sleep frames to catch up
        nextFrameTime += 1000.0 / FPS;
        double remainingMS = nextFrameTime - (double)fenster_time();
        if (remainingMS > 0) fenster_sleep((int64_t)remainingMS);
    }

    fenster_close(&window);
    return 0;
}
