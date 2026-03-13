#include "../include/fenster.h"

#define WIDTH  600
#define HEIGHT 400

uint32_t buffer[WIDTH * HEIGHT];

int main()
{
    struct fenster window = {
        .title = "Hello, World!",
        .width = WIDTH,
        .height = HEIGHT,
        .buf    = buffer
    };

    // Open a system window using the given window specifications
    if (fenster_open(&window) < 0) return 1;

    while (fenster_loop(&window) == 0) {
        ;
    }
    fenster_close(&window);
}

