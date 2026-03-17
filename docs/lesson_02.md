---
layout: lesson
title: "Lesson 02: Drawing to the screen"
lesson_label: "Lesson 02"
prev_lesson: /lesson_01
next_lesson: /lesson_03
---

![Lesson 02 Animated Triangle]({{ '/images/lesson_02_animated_triangle.gif' | relative_url }}){: width="100%"}

## Clean up old code
We're not going to need to print the channel values for a buffer pixel again, so we can remove that. I changed the timing code to be more accurate. We cap the renderer to the target frame rate by initializing nextFrameTime to fenster_time() and advancing it by the desired frame time (1000ms / FPS) each iteration, sleeping until it's reached. If the OS wakes us late, the next sleep is shortened by exactly that amount.

[Lesson 02: Initial main.c](https://github.com/TurpeNescire/softrend-3d/tree/acf002b72695acdcf68a3dc822e6e2e747339ab3/src/main.c)

## Drawing our first triangle

Drawing anything to the screen at this point involves setting the color value of our `buffer` pixels. Because we declared `buffer` in global scope, it is automatically 0 initialized along with all other global scoped variables, which as we saw means the color black when all color channels are set to 0 in RGB. We have created a window WIDTH pixels wide and HEIGHT pixels tall, so there are 600x400 or 240,000 pixels in our window. We could have declared `buffer` as a row-major multi-dimensional array, `uint32_t buffer[HEIGHT][WIDTH]` which is of type `uint32_t **`, and assigned `window.buf = (uint32_t *)buffer;`. This simplifies the notation for element access, but hides the flat nature of the memory array. We can create a convenience macro for getting and setting pixel information and add a color constant for later use:

```c
#define FPS 60 // Targeted frame rate (frequency in hz)

// Framebuffer is row-major
#define PIXEL(x, y) buffer[(y) * WIDTH + (x)]

#define WHITE 0xFFFFFFFF

uint32_t buffer[WIDTH * HEIGHT];

```

This removes any concern over row or column major ordering or having to write the expression for each index. The first step to drawing a triangle is to determine the vertices. If we want a 300 pixel wide, 200 pixel tall isosceles triangle that fits nicely in our screen, we can use the following declarations followed by calling `PIXEL` for each point:

```c
        .buf    = buffer
    };

    // define's the vertices of a 300 pixel wide, 200 pixel high screen centered triangle
    int x0 = 150, y0 = 300; // bottom-left
    int x1 = 450, y1 = 300; // bottom-right
    int x2 = 300, y2 = 100; // apex
    PIXEL(x0, y0) = WHITE;
    PIXEL(x1, y1) = WHITE;
    PIXEL(x2, y2) = WHITE;

    // Open a system window using the given window specifications
    if (fenster_open(&window) < 0) return 1;
```

You should now see 3 single pixels outlining the vertices of a triangle. How do we connect the dots? There are several common methods, Bresenham's

A 3D object is typically described as a collection of meshes that contain geometric information about the triangle or quad faces of the object. Triangles are the easiest to work with because all points on the triangle are coplanar, and the face has only one surface normal. We'll come back to all of this later when we learn to import a geometry from an object file.


