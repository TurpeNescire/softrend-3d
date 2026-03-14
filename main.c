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

#define B_END_ALPHA  3
#define B_END_RED    2
#define B_END_GREEN  1
#define B_END_BLUE   0

uint32_t pixelColor = 0x0;

uint32_t buffer[WIDTH * HEIGHT];

int main()
{
    struct fenster window = {
        .title = "Hello, World!",
        .width = WIDTH,
        .height = HEIGHT,
        .buf    = buffer
    };

    for (int i = 0; i < WIDTH * HEIGHT; i++)
        buffer[i] = 0;

    uint8_t *bytes = (uint8_t *)&pixelColor;
    bytes[B_END_RED] = 0xFF;
    printf("B:%02X G:%02X R:%02X A:%02X\n", bytes[B_END_BLUE], bytes[B_END_GREEN], bytes[B_END_RED], bytes[B_END_ALPHA]);

    // Open a system window using the given window specifications
    if (fenster_open(&window) < 0) return 1;

    while (fenster_loop(&window) == 0) {
        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            buffer[i] = pixelColor;
        }
        pixelColor += 1;
        if (pixelColor % 256 == 0)
            printf("%u\n", pixelColor); 
    }

    fenster_close(&window);
}

