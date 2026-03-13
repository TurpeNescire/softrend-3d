---
layout: lesson
title: "Lesson 01: Setup"
lesson_label: "Lesson 01"
next_lesson: /lesson_02
---
![Lesson 01 banner]({{ '/images/lesson_01_banner.jpg' | relative_url }}){: width="100%"}

# Lesson 01: Purpose and Setup

## What's the goal?
This is a toy project to teach myself software 3D rendering techniques by building a triangle rasterizer entirely from scratch. Here are some of the current features that these docs will cover implementing:

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

Aside from the [C standard library](https://en.wikipedia.org/wiki/C_standard_library) we should only need Fenster and [STB Image](https://github.com/nothings/stb/blob/master/stb_image.h), both of which are single source files with no external dependencies that we can include directly in our project directories without having to link to any external system libraries. I'll include a Makefile in the project source that should compile on Windows, Linux and Mac, but on Windows this might require some extra work. If you have Windows 10 you can install [Windows Subsystem for Linux (WSL)](https://en.wikipedia.org/wiki/Windows_Subsystem_for_Linux) which comes installed on Windows 11 by default and run `sudo apt install make`. Because we not linking to external system installed libraries there should be no added complexity for compiling with a visual IDE like Visual Studio.

## Download and compiling the Fenster library
The simplest method is to just copy [fenster.h](https://github.com/zserge/fenster/blob/main/fenster.h) to your project directory. I like to store headers in the `/include` directory of my project, which is what I will assume going forward. The most basic Fenster program is listed in the article listed above by the creator:

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

To compile, you simply run the command `cc -i include src/main.c -o softrend -framework cocoa` on mac, `cc -i include src/main.c -o softrend -framework -lX11` on Linux and `cc -i include src/main.c -o softrend -lgdi32` on Windows. Going forward, I'll be using a cross-platform Makefile that you can download from the project source at the end of the lesson, and compile simply by typing `make`, and run the source with `./softrend`.

You should see a 600x400 console window pop up with all black contents and a window title of "Hello, World!":

![Lesson 01 Fenster Window]({{ '/images/lesson_01_fenster_window.png' | relative_url }}){: width="100%"}

## What's a framebuffer?
This code should be mostly self-explanatory save for the `fenster.buf` field that takes an array of type `uint32_t`. Fenster passes this framebuffer array to the underlying operating system to write to the display.

![Lesson 01 Buffer Array]({{ '/images/lesson_01_buffer_array.png' | relative_url }}){: width="100%"}

A [framebuffer](https://en.wikipedia.org/wiki/Framebuffer) is a contiguous section of RAM that represents all of the pixels on the screen. Most modern applications use 32-bits for each pixel in either ARGB or RGBA order, with 1 byte per channel (R=red, G=green, B=blue, A=alpha or transparency). Often they will use structs with separate fields for each channel, but Fenster packs all color channels into a single 4 byte word. When converting between different libraries or formats, you need to be specific about the order of color channels. We'll come back to this in the following lesson when we actually write pixels into the buffer.


## Source
[Github Source](https://github.com/TurpeNescire/softrend-3d/tree/2120947b2bfac14a45bfb445d76b93efb96af1eb)
