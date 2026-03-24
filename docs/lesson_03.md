---
layout: lesson
title: "Lesson 03: Triangle rasterizer"
lesson_label: "Lesson 03"
prev_lesson: /lesson_02
next_lesson: /lesson_04
---

<figure>
  <img src="{{ '/images/lesson_02_animated_triangle.gif' | relative_url }}" alt="Lesson 02 Animated Triangle" style="width:100%">
  <figcaption>Temporary placeholder for Lesson 3 banner</figcaption>
</figure>

# [Lesson 03: Initial main.c](https://github.com/TurpeNescire/softrend-3d/tree/acf002b72695acdcf68a3dc822e6e2e747339ab3/src/main.c)

# A brief historical note on software vs. hardware 3D rendering
Software 3D rendering had a relatively brief window of popularity. Early examples of wireframe 3D rendering included games like Battlezone (1980) but it wasn't until the early 1990s that the first 2.5D texture mapped 3D games were created. Examples include Ultima Underworld and Wolfenstein 3D, both in 1992. The first consumer hardware accelerators like the [3dfx Voodoo](https://en.wikipedia.org/wiki/3dfx#First_chips) arrived in the mid-90s, and Quake was the most notable early example of a 3D game to take advantage of 3D acceleration with its GLQuake release in 1997. Add-in 3D cards rapidly gained in popularity after that, and through the late 90s most games offered support for software rendering either as the default, or as a fallback when the user didn't have a 3D card. By the early 2000s very few games bothered to include a software render, and the [Pixomatic software renderer](https://web.archive.org/web/20130425032829/http://www.radgametools.com/pixomain.htm) developed by [Michael Abrash](https://en.wikipedia.org/wiki/Michael_Abrash) at RAD Game tools was a drop-in DX7 (and later DX-9) software render that companies could license as a fall-back renderer for users on old hardware. The most famous game to make use of this licensed software render was [Unreal Tournament 2004](https://en.wikipedia.org/wiki/Unreal_Tournament_2004).

## Why software rendering was phased out
The reason CPU based software rendering fell out of favor is not as obvious as it might seem at first. Software rendering is not calculation-bound, but memory-bandwidth-bound. When reading and writing pixel data to a framebuffer that is too large to fit in the CPU cache, the CPU has to switch to reading and writing to main memory (RAM). This is far slower than hitting the L1-L3 cache - RAM latency is orders of magnitude higher, a compounding issue for every framebuffer access that misses cache. As screen resolutions (and framebuffer sizes) continued to grow, the bandwidth problem scaled with them and CPU memory bandwidth could not keep pace with GPU memory bandwidth.

This situation is still the same today. Even with L3 caches now reaching 8MB+, renderers require multiple buffers (a framebuffer and depth buffer at minimum) and these exceed cache capacity at any resolution beyond those of the mid-90s. Texture mapping adds another wrinkle with hundreds of megabytes of texture data needing to be read per frame in modern AAA games. Most modern GPUs range from 8GB to 24GB of VRAM, and while they do have small on-die L1 and L2 caches and higher memory access latency than the CPU, they can move hundreds of GB/s from memory compared to under 100 GB/s for CPUs, and thousands of parallel execution units hide the latency cost while any individual access resolves.

# What is rasterization?
**Rasterization** comes from "raster", derived from the Latin *rastrum* meaning rake. A ["raster image"](https://en.wikipedia.org/wiki/Raster_graphics) is a two-dimensional matrix of pixel color information. [Raster scanning](https://en.wikipedia.org/wiki/Raster_scan) is the process used in old [CRT display](https://en.wikipedia.org/wiki/Cathode_ray_tube) technology to display a raster image where an electron beam is traced horizontally across the phosphor face of the display from left to right, top to bottom. In modern graphics programming, rasterization is the process of converting a geometric description of polygons into discrete pixels on the screen. [Scanline rasterization](https://en.wikipedia.org/wiki/Scanline_rendering) was the natural approach to rasterize polygon data into pixels for CRT screens, rendering pixels line by line starting from the top left of the screen in the same order that the beam traced the image across the screen, and is still the process employed by modern GPU rasterizers.

<figure>
  <img src="{{ '/images/lesson_03_Scan-line_algorithm.png' | relative_url }}" alt="Scanline algorithm" style="width:100%">
  <figcaption>Scanline algorithm</figcaption>
</figure>

## Why use triangles 
Triangles have several advantages over more complex polygons. Three points always lie on a single plane while polygons with more vertices do not have this advantage and aren't guaranteed to be flat. Each point on a triangle has the same surface normal, which is a highly useful property in 3D rendering as we'll see. Triangles are also always convex, which means a horizontal scanline can intersect it at most at two edges which makes finding and filling spans between surface edges far more simple. For our purposes, this is the most essential property of triangles. An n-gon is just a polygon with more than 3 sides. A scanline can intersect n-gons at more than two edges, resulting in multiple disconnected spans on the same row. To draw pixels on each disconnected span for that polygon would require sorting them and deciding which pairs of intersection points to fill, greatly increasing the complexity of the algorithm. Instead, any n-gon can be converted into triangles on export from a 3D modeling program, or even by us at runtime when loading from a file, and modern GPUs still work exclusively using triangle rasterization. We'll revisit each of these properties as we build the rasterizer.


