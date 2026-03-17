/* **** main.c ************************************************************** *
 * SoftRend3D                                                                 *
 * https://github.com/TurpeNescire/softrend-3d                                *
 * https://turpenescire.github.io/softrend-3d                                 *
 *                                                                            *
 * v0.1.0                                                                     *
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

#define FPS        60           // Targeted frame rate (frequency in hz)
#define FRAME_TIME (1000 / FPS) // Targeted frame duration (period in ms)

#define ALPHA 3
#define RED   2
#define GREEN 1
#define BLUE  0

uint32_t buffer[WIDTH * HEIGHT];

int main()
{
    struct fenster window = {
        .title  = "Hello, World!",
        .width  = WIDTH,
        .height = HEIGHT,
        .buf    = buffer
    };

    // Open a system window using the given window specifications
    if (fenster_open(&window) < 0) return 1;

    int64_t secondStart = fenster_time();
    int64_t frameStart  = secondStart;
    int     frameCount  = 0;
    while (fenster_loop(&window) == 0) {
        // print FPS once a second
        frameCount++; 
        if (fenster_time() - secondStart >= 1000) { // is elapsed time over 1000ms (1s)?
            // bufferByteArray points to the first byte (the blue channel) of the first element of buffer
            const uint8_t *bufferByteArray = (const uint8_t *)buffer;
            printf("fps: %d, A:%02x R:%02x G:%02x B:%02x\n",
                    frameCount,
                    bufferByteArray[ALPHA],
                    bufferByteArray[RED],
                    bufferByteArray[GREEN],
                    bufferByteArray[BLUE]);
            frameCount  = 0;
            secondStart = fenster_time();
        }

        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            buffer[i] += 1;
        }

        // sleep until we reach desired FRAME_TIME 
        int64_t remainingMS = frameStart + FRAME_TIME - fenster_time();
        if (remainingMS > 0) fenster_sleep(remainingMS);
        frameStart = fenster_time();
    }

    fenster_close(&window);
}


