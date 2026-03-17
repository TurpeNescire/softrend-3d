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

[Lesson 02 Initial Source](https://github.com/TurpeNescire/softrend-3d/tree/3138f214f36acb6b3cee617c93566a7757034f60)

## Drawing our first triangle

[Bresenham's line algorithm](https://en.wikipedia.org/wiki/Bresenham's_line_algorithm)
[triangle rasterization](https://en.wikipedia.org/wiki/Rasterisation#3D_images).

Drawing anything to the screen at this point involves setting the color value of our `buffer` pixels. Because we declared `buffer` in global scope, it is automatically 0 initialized along with all other global scoped variables, which as we saw means the color black when all color channels are set to 0 in RGB. We have created a window WIDTH pixels wide and HEIGHT pixels tall, so there are 600x400 or 240,000 pixels in our window. We could have declared `buffer` as a row-major multi-dimensional array, `uint32_t buffer[HEIGHT][WIDTH]` which is of type `uint32_t **`, and assigned `window.buf = (uint32_t *)buffer;`. This simplifies the notation for element access, but hides

A 3D object is typically described as a collection of meshes that contain geometric information about the triangle or quad faces of the object. Triangles are the easiest to work with because all points on the triangle are coplanar, and the face has only one surface normal. We'll come back to all of this later when we learn to import a geometry from an object file.


