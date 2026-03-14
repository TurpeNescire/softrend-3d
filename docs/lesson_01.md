---
layout: lesson
title: "Lesson 01: Setup"
lesson_label: "Lesson 01"
next_lesson: /lesson_02
---
![Lesson 01 banner]({{ '/images/lesson_01_banner.png' | relative_url }}){: width="100%"}

# Lesson 01: Purpose and Setup

## What's the goal?
This is a project to teach myself software 3D rendering techniques by building a triangle rasterizer entirely from scratch. Here are some of the current features that these docs will cover implementing:

- OBJ + MTL model loading with multi-mesh support and UV texture mapping
- MVP transform pipeline: local → world → camera → screen
- Near-plane clipping via Sutherland-Hodgman
- Depth buffering
- Backface culling and screen-space bounding box culling
- Perspective-correct UV interpolation
- Programmable vertex and fragment shaders
- Lambert, Gouraud, Phong and textured shaders
- CPU-generated mipmapping (12 levels)
- Bilinear filtering
- Shadow mapping
- Multi-threaded tile grid based parallelism
- Particle system

To begin with, I wanted to have a minimal windowing system. I'm on a Mac at the moment, but don't know Objective-C which Apple uses for its system libraries, so finding out how to write a framebuffer to the screen was beyond me. I found [Fenster](https://zserge.com/posts/fenster/) to be the most minimal framebuffer library, offering only a cross-platform window, framebuffer, mouse and keyboard handling. At around 300 lines it's very readable and mirrors my desire to avoid the abstractions of 3D APIs by doing the same thing for drawing pixels to the screen.

## What you should know
This tutorial series assumes you have a working knowledge of the [C programming language](https://en.wikipedia.org/wiki/C_%28programming_language%29) and a passing familiarity with basic math concepts like the [Cartesian coordinate system](https://en.wikipedia.org/wiki/Cartesian_coordinate_system), the [unit circle](https://en.wikipedia.org/wiki/Unit_circle) and the three [basic trigonometric functions](https://en.wikipedia.org/wiki/Trigonometric_functions#Right-angled_triangle_definitions). I'll go over the little bit of extra math needed such as vectors and matrices.

Aside from the [C standard library](https://en.wikipedia.org/wiki/C_standard_library) we should only need Fenster and [STB Image](https://github.com/nothings/stb/blob/master/stb_image.h), both of which are single source files with no external dependencies that we can include directly in our project directories without having to link to any external system libraries. I'll include a Makefile in the project source that should compile on Windows, Linux and Mac, but on Windows this might require some extra work. If you have Windows 10 you can install [Windows Subsystem for Linux (WSL)](https://en.wikipedia.org/wiki/Windows_Subsystem_for_Linux) which comes installed on Windows 11 by default and run `sudo apt install make`. Because we're not linking to any external system installed libraries there should be no added complexity for compiling with a visual IDE like Visual Studio.

## Downloading and compiling the Fenster library
The simplest method is to just copy [fenster.h](https://github.com/zserge/fenster/blob/main/fenster.h) to your project directory. I like to store headers in the `/include` directory of my project and source files in `/src` which is what I will assume going forward. An example of a basic Fenster program is contained in the article listed above by the creator:

```c
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
```

To compile, you simply run the command `cc -I include src/main.c -o softrend -framework cocoa` on mac, `cc -I include src/main.c -o softrend -lX11` on Linux and `cc -I include src/main.c -o softrend -lgdi32` on Windows. Going forward, I'll be using a cross-platform Makefile that you can download from the project source at the end of the lesson, and compile simply by typing `make`, and run the source with `./softrend`.

You should see a 600x400 console window pop up with all black contents and a window title of "Hello, World!":

![Lesson 01 Fenster Window]({{ '/images/lesson_01_fenster_window.png' | relative_url }}){: width="100%"}

## What's a framebuffer?
This code should be mostly self-explanatory save for the `fenster.buf` field that takes an array of type `uint32_t`. Fenster passes this framebuffer array to the underlying operating system to write to the display.

![Lesson 01 Buffer Array]({{ '/images/lesson_01_buffer_array.png' | relative_url }}){: width="100%"}

A [framebuffer](https://en.wikipedia.org/wiki/Framebuffer) is a contiguous section of RAM that represents all of the pixels on the screen. Most modern applications use 32-bits for each pixel in either ARGB or RGBA order, with 1 byte per channel (R=red, G=green, B=blue, A=alpha or transparency). Often they will use structs with separate fields for each channel, but Fenster packs all color channels into a single 4 byte word. When converting between different libraries or formats, you need to be specific about the order of color channels.

The image above shows a 32-bit ARGB pixel in [little-endian](https://en.wikipedia.org/wiki/Endianness) notation.  The high order byte represents the alpha transparency mask in bits 24-31.  When all of the bits are set to 0xFF in hexadecimal (or 255 in decimal, 11111111 in binary) then the image is fully opaque. When the bits are set to 0 (0x00 in hex) the image is fully transparent. Bits 16-23 represent the red channel, bits 8-15 the green channel, and bits 0-7 the blue channel. It'll be a while before we need to deal with transparency, and in fact Fenster ignores the alpha byte when sending the buffer to the system, so the only transparency blending on our buffer will have to be taken care of by ourselves. For the time being, the alpha channel can be set to 0, or 0xFF, or anything in between, both our own code and Fenster (and the underlying window system) will ignore it.

If we want to represent fully opaque red we'd use 0xFFFF0000, with bits 24-31 (the high order, most significant byte) representing the alpha value and the second byte the red value (fully red). We can define a constant at the top of the file next to our #defines:

### Filling the buffer

```c
#define HEIGHT 400

const uint32_t RED = 0xFFFF0000;

uint32_t buffer[WIDTH * HEIGHT];
```

and then inside `main` we can fill the buffer once before the Fenster window loop:

```c
        .buf    = buffer
    };

    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        buffer[i] = RED;
    }

    // Open a system window using the given window specifications
```

## The Fenster event loop
Like most windowing libraries, Fenster continuously loops over the system event queue, though in as simple a manner as possible, only presenting the window dimensions and title and framebuffer to the system and grabbing mouse and keyboard input values during each loop. If we only call `fenster_open(&window)` without continuously calling `fenster_loop(&window)` the application window will instantly open and close. The loop continuously draws the framebuffer and keeps the window open. If you put a timer in and track the loop speed you should see that it's capped around your monitor's refresh rate by the underlying window system compositor. We can use fenster_time, which returns the number of milliseconds since the Unix epoch (time since 00:00:00 UTC on 1 January 1970). For example:

```c
    // Open a system window using the given window specifications
    if (fenster_open(&window) < 0) return 1;

    int64_t secondStart = fenster_time();
    int     frameCount  = 0;
    while (fenster_loop(&window) == 0) {
        // print FPS once a second
        frameCount++;
        if (fenster_time() - secondStart >= 1000) { // is elapsed time over 1000ms (1s)?
            printf("fps: %d\n", frameCount);
            frameCount  = 0;
            secondStart = fenster_time();
        }
    }

    fenster_close(&window);
```

On my system it starts out printing a very high number (~700ms) then settles around 120 fps, the refresh rate of my display. We filled the framebuffer `window.buf` with red pixels before calling the render loop. We can also change the buffer's pixels inside the loop. Simply moving the `for` loop inside the `fenster_loop` doesn't change what appears on the screen, it just means the buffer is getting overwritten with the same pixels each frame. We could change it so that pixel value changes each loop:

```c
            secondStart = fenster_time();
        }

        for (int i = 0; i < WIDTH * HEIGHT; i++) {
            buffer[i] += 1;
        }
    }

    fenster_close(&window);
```

Because we declared the `buffer` array globally it is 0 initialized and this code will add 1 to each pixel value.  At 0, each channel starts out black and ramps up to full intensity blue at 255 (each channel has 256 possible values, 0-255). After hitting 255, the pixel adds 1 to the next channel (green) and due to unsigned integer wraparound the blue byte starts over at 0 again ramping across the blue colors, but now with a tiny 1 bit shift towards green. If we let the program run at 120 fps it would take approximately 39 hours to cycle through all 16,777,216 possible RGB values — one per frame. 

### Capping FPS
If we want to limit the frame time to a specific value, we can add a few lines of code. We add a few \#defines at the top of the file:

```c
#define HEIGHT 400

#define FPS    60               // Targeted frame rate (frequency in hz)
#define FRAME_TIME (1000 / FPS) // Targeted frame duration (period in ms)

const uint32_t RED = 0xFFFF0000;
```

To the declarations before the window loop we add `frameStart`:

```c
    int64_t secondStart = fenster_time();
    int64_t frameStart  = secondStart;
    int     frameCount  = 0;
```

and at the end of the loop:

```c
            buffer[i] += 1;
        }

        // sleep until we reach desired FRAME_TIME 
        int64_t remainingMS = frameStart + FRAME_TIME - fenster_time();
        if (remainingMS > 0) fenster_sleep(remainingMS);
        frameStart = fenster_time();
    }
```

As a point of curiosity, if we were taking 39 hours before to draw all possible RBG values we'd now take around 78 hours to iterate over all the colors values. Worth noting that the alpha value is ignored, so it actually would take us around 2.27 years before the alpha channel wrapped back to 0 for each pixel.

## Source
[Project Source: Lesson 01](https://github.com/TurpeNescire/softrend-3d/tree/07b1e965b231eb930a9f89b56a5b6f81371fde2d)

