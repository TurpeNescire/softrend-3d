---
layout: lesson
title: "Lesson 02: Drawing to the screen"
lesson_label: "Lesson 02"
prev_lesson: /lesson_01
next_lesson: /lesson_03
---

![Lesson 02 Animated Triangle]({{ '/images/lesson_02_animated_triangle.gif' | relative_url }}){: width="100%"}

[Lesson 02 Initial Source](https://github.com/TurpeNescire/softrend-3d/tree/3138f214f36acb6b3cee617c93566a7757034f60

# Drawing our first triangle

[Bresenham's line algorithm](https://en.wikipedia.org/wiki/Bresenham's_line_algorithm)
[triangle rasterization](https://en.wikipedia.org/wiki/Rasterisation#3D_images).
A 3D object is typically described as a collection of meshes that contain geometric information about the triangle or quad faces of the object. Triangles are the easiest to work with because all points on the triangle are coplanar, and the face has only one surface normal. We'll come back to all of this later when we learn to import a geometry from an object file.

## Clean up old code
We're not going to need to print the channel values for a buffer pixel again, so we can remove that. In the FPS print loop in `main` return to the original code:

```c
    while (fenster_loop(&window) == 0) {
        // print FPS once a second
        frameCount++; 
        if (fenster_time() - secondStart >= 1000) { // is elapsed time over 1000ms (1s)?
            printf("fps: %d\n", frameCount);
            frameCount  = 0;
            secondStart = fenster_time();
        }
```

and at the top remove the ARGB array index defines ALPHA, RED, etc.:

```c
#define FRAME_TIME (1000 / FPS) // Targeted frame duration (period in ms)

uint32_t buffer[WIDTH * HEIGHT];
```
