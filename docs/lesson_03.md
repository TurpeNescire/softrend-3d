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

# [Lesson 03: Initial main.c](https://github.com/TurpeNescire/softrend-3d/tree/777f2da82da26f25757cf12ac8a1aa00f71e0559/src/main.c)

# A brief historical note on software vs. hardware 3D rendering
Software 3D rendering had a relatively brief window of popularity. Early examples of wireframe 3D rendering included games like Battlezone (1980) but it wasn't until the early 1990s that the first 2.5D texture mapped 3D games were created. Examples include Ultima Underworld and Wolfenstein 3D, both in 1992. The first consumer hardware accelerators like the [3dfx Voodoo](https://en.wikipedia.org/wiki/3dfx#First_chips) arrived in the mid-90s, and Quake was the most notable early example of a 3D game to take advantage of 3D acceleration with its GLQuake release in 1997. Add-in 3D cards rapidly gained in popularity after that, and through the late 90s most games offered support for software rendering either as the default, or as a fallback when the user didn't have a 3D card. By the early 2000s very few games bothered to include a software renderer. The [Pixomatic software renderer](https://web.archive.org/web/20130425032829/http://www.radgametools.com/pixomain.htm) developed by [Michael Abrash](https://en.wikipedia.org/wiki/Michael_Abrash) at RAD Game tools in 2003 was a drop-in DX7 (and later DX-9) software render that companies could license as a fall-back renderer for users on old hardware, and the last commercial software renderer still in development at that time. As far as I can tell, the last large commercial game to include support for software rendering (via Pixomatic) was [Unreal Tournament 2004](https://en.wikipedia.org/wiki/Unreal_Tournament_2004).

## Why software rendering was phased out
The reason CPU based software rendering fell out of favor is not as obvious as it might seem at first. Software rendering is not calculation-bound, but memory bandwidth-bound. When reading and writing pixel data to a framebuffer that is too large to fit in the CPU cache, the CPU has to switch to reading and writing to main memory (RAM). This is far slower than hitting the L1-L3 cache - RAM latency is orders of magnitude higher, a compounding issue for every framebuffer access that misses cache. As screen resolutions (and framebuffer sizes) continued to grow, the bandwidth problem scaled with them and CPU memory bandwidth growth could not keep pace with GPU memory bandwidth advancements.

This situation is still the same today. Even with L3 caches now reaching 8MB+, renderers require multiple buffers (a framebuffer and depth buffer at minimum) and these exceed cache capacity at any resolution beyond those of the mid-90s. Texture mapping adds another wrinkle with large amounts of texture data needing to also be read per frame in modern AAA games. Most modern GPUs range from 8GB to 24GB of VRAM, and while they do have small on-die L1 and L2 caches and higher memory access latency than the CPU, they can move hundreds of GB/s from memory compared to under 100 GB/s for CPUs, and thousands of parallel execution units hide the latency cost while any individual access resolves.

# What is rasterization?
**Rasterization** comes from "raster", derived from the Latin *rastrum* meaning rake. A [raster image](https://en.wikipedia.org/wiki/Raster_graphics) is a two-dimensional matrix of pixel color information. Rasterization is the process of converting a geometric description of polygons into a raster image. [Raster scanning](https://en.wikipedia.org/wiki/Raster_scan) is the process used in old [CRT display](https://en.wikipedia.org/wiki/Cathode_ray_tube) technology to display a raster image where an electron beam is traced horizontally across the phosphor face of the display from left to right, top to bottom. [Scanline rasterization](https://en.wikipedia.org/wiki/Scanline_rendering) was the natural approach to rasterize image data into pixels for CRT screens. Pixels were rendered line by line starting from the top left of the screen in the same order that the beam traced the image across the screen. Display and computation hardware has changed significantly, and so have the rasterization algorithms used by modern GPUs, but the end result is still a grid of discrete pixels to be rendered to the screen.

<figure>
  <img src="{{ '/images/lesson_03_Scan-line_algorithm.png' | relative_url }}" alt="Scanline algorithm" style="width:100%">
  <figcaption>Scanline algorithm</figcaption>
</figure>

## Why rasterizers use triangles
Triangles have several advantages over more complex polygons. The three points of a triangle always lie on a single plane (coplanar) while polygons with more vertices do not have this advantage and aren't guaranteed to be flat. Each point on a triangle has the same surface normal, which is a highly useful property in 3D rendering as we'll see. Triangles are also always convex, which means a horizontal scanline can intersect it at most at two edges which makes finding and filling spans between surface edges far more simple. For our purposes, this is the most essential property of triangles. An n-gon is just a polygon with more than 3 sides. A scanline can intersect n-gons at more than two edges, resulting in multiple disconnected spans on the same row. To draw pixels on each disconnected span for that polygon would require sorting them and deciding which pairs of intersection points to fill, greatly increasing the complexity of the algorithm. Instead, any n-gon can be converted into triangles on export from a 3D modeling program, or even by us at runtime when loading from a file, and modern GPUs still work exclusively using triangle rasterization. We'll revisit each of these properties as we build the rasterizer.

# Triangle fill
We left off having drawn the outline of triangles to the screen using our `drawLine` function to interpolate the position of each pixel on the line between two points. We've also talked about the scanline rasterization process used to render an image to a CRT screen and how triangles have the advantage of allowing us to easily find and fill the span between the two edges crossing the scanline. The algorithm is that easy: we iterate from the top of a triangle to the bottom, finding where the edges of the triangle cross the current scanline, and filling the gap between the two edges - the span. A triangle is thus filled from top to bottom, filling each horizontal span from the left edge to the right.

## Adding Vec2 type
We already have our triangle line edges, but they're not very convenient to work with. We're passing 4 values representing two points in screen space with the origin in the top-left, where the first pixel of our framebuffer is. The traditional approach to storing and passing point coordinates is to use what is known as a [vector](https://www.3blue1brown.com/lessons/vectors#title). Vector might be one of the most overloaded words in the English language, but at its simplest we can just think of it as a Cartesian (x, y) coordinate. Later on we'll build on that notion, but for now we can define a vector as:

```c
uint32_t buffer[WIDTH * HEIGHT];

typedef struct Vec2 {
    int x;
    int y;
} Vec2;
```

## Adding the Triangle type and drawTriangle()
We can then change our `drawLine` function signature to take two `Vec2` objects by reference instead of four `int` objects by value. We won't need to change their values, so we can declare them `const`. We can either replace all uses of `x0`, `y0`, `x1` and `x2` inside `drawLine` with `v0->x`, `v0->y`, `v1->x` and `v1->y` or create aliases as I've done here. With the const hint to the compiler and with optimizations on, the compiler should optimize the code into the same thing aside from the signature differences.

```c
void drawLine(const Vec2 *v0, const Vec2 *v1, uint32_t color) {
    int x0 = v0->x, y0 = v0->y;
    int x1 = v1->x, y1 = v1->y;
```

Along with `Vec2` it will be helpful to have a `Triangle` data type that can bundle all three `Vec2` vertices together for convenience. After the `Vec2` definition I placed:

```c
typedef struct Triangle {
    Vec2 v0;
    Vec2 v1;
    Vec2 v2;
} Triangle;
```

and created a new function `drawTriangle` that I defined after `drawLine` and before `main`:

```c
void drawTriangle(const Triangle *tri, uint32_t color) {
    drawLine(&tri->v0, &tri->v1, color);
    drawLine(&tri->v1, &tri->v2, color);
    drawLine(&tri->v2, &tri->v0, color);
}
```

And inside `main` changed our `drawLine` calls into `Triangle` definitions and calls to `drawTriangle`:

```c
        .buf    = buffer
    };
    
    // x-dominant (wide/flat)      — top-left cell
    Triangle tri0 = { { .x =  20, .y = 170 },   // bottom-left
                      { .x = 180, .y = 170 },   // bottom-right
                      { .x = 100, .y =  30 } }; // top-middle
    // y-dominant (tall/narrow)    — top-middle cell
    Triangle tri1 = { { .x = 270, .y = 180 },   // bottom-left
                      { .x = 330, .y = 180 },   // bottom-right
                      { .x = 300, .y =  20 } }; // top-middle
    // one vertical edge           — top-right cell
    Triangle tri2 = { { .x = 410, .y =  20 },   // top-left
                      { .x = 410, .y = 180 },   // bottom-left
                      { .x = 580, .y = 100 } }; // right-middle
    // one horizontal edge         — bottom-left cell
    Triangle tri3 = { { .x =  20, .y = 210 },   // bottom-left
                      { .x = 180, .y = 210 },   // bottom-right
                      { .x = 100, .y = 380 } }; // top-middle
    // right triangle              — bottom-middle cell
    Triangle tri4 = { { .x = 220, .y = 220 },   // top-left
                      { .x = 220, .y = 380 },   // bottom-left
                      { .x = 380, .y = 380 } }; // bottom-right
    // degenerate: two coincident vertices  — bottom-right cell
    Triangle tri5 = { { .x = 500, .y = 220 },   // top-right
                      { .x = 500, .y = 220 },   // top-right
                      { .x = 420, .y = 370 } }; // bottom-left
    // degenerate: single point    — tucked in corner of bottom-right cell
    Triangle tri6 = { { .x = 570, .y = 370 },
                      { .x = 570, .y = 370 },
                      { .x = 570, .y = 370 } };
    drawTriangle(&tri0, colors[RED]);
    drawTriangle(&tri1, colors[GREEN]);
    drawTriangle(&tri2, colors[BLUE]);
    drawTriangle(&tri3, colors[YELLOW]);
    drawTriangle(&tri4, colors[CYAN]);
    drawTriangle(&tri5, colors[MAGENTA]);
    drawTriangle(&tri6, colors[ORANGE]);

    // Open a system window using the given window specifications
    if (fenster_open(&window) < 0) return 1;
```

We're back to drawing our example triangles exactly as we started the lesson. What have we gained? We can now pass vertex information around in a more convenient fashion, but also have a better abstraction layer, passing each `Triangle` to `drawTriangle`. Within `drawTriangle` we now need to find and fill all of the spans where the triangle crosses the spanline. At the least, we need to know the vertical screen space height of the triangle so that we can iterate over each scanline that touches the triangle. 

The easiest way to find the minimum and maximum of three integer values is to create a few helper functions, `int_min` and `int_max`, both declared to be `static inline`. `inline` tells the compiler to replace the function calls with their bodies at each occurence, avoiding the added overhead of calling a function. For small functions like this the compiler will carry out that optimization by itself, but it's good practice to provide those hints to the reader and the compiler. `static` tells the compiler that the function symbol is only accessible from this file.

Before `drawLine` and after the `Triangle` definition:

```c
// Utility math functions
static inline int int_max(int x, int y) {
    return (x > y) ? x : y;
}

static inline int int_min(int x, int y) {
    return (x > y) ? y : x;
}
```

and we replace `drawTriangle` with:

```c
void drawTriangle(const Triangle *tri, uint32_t color) {
    const Vec2 *v0 = &tri->v0, *v1 = &tri->v1, *v2 = &tri->v2;

    int top_y = int_min(v0->y, int_min(v1->y, v2->y));
    int bot_y = int_max(v0->y, int_max(v1->y, v2->y));
}
```

Now we need to iterate over every scanline between `top_y` and `bot_y` inclusive. We need to find a clean interface for finding the `x` values of any intersections of the triangle against the scanline. We need a helper function that takes the triangle vertices representing an edge, and tell us if that edge crosses the scanline, and returning the `x` value of that intersection if it exists. This helper function might be called hundreds of thousands of times per frame depending on the number of triangles being rendered, so we'll declare it `static inline` also:

```c
static inline bool crossesScanlineAt(const Vec2 *v0, const Vec2 *v1, int scanline_y, int *x) {
    ...
}
```

If the edge crosses `scanline_y` we set the `x` value and return `true` otherwise we return `false`. Given what we have learned while implementing `drawLine`, you should be able to see an easy solution for determing that `x` value. I would recommend taking the time now to implement your own solution before reading on.

You might have noticed that we can use the basic interpolation algorithm we developed in `drawLine` to find the `x` intersection. There are several edge cases to consider. When `dy` is 0, the edge is horizontal. Our scanline fill algorithm as we imagined it will fill between two x-values. For a horizontal line, crossesScanlineAt can't tell us anything valuable. The horizontal line will eventually be filled when crossesScanlineAt iterates over the other two edges of the triangle that do cross the scanline. The other edge case is when `scanline_y` does not lie between the `y` values of both vertices passed to `crossesScanlineAt`. In both cases we exit early returning `false`.

```c
static inline bool crossesScanlineAt(const Vec2 *v0, const Vec2 *v1, int *x, int scanline_y) {
    int dy = v1->y - v0->y;
    if (dy == 0) return false; // The edge is a horizontal line or coincident - no crossing

    if (scanline_y < int_min(v0->y, v1->y) || scanline_y > int_max(v0->y, v1->y))
        return false; // Ensure that the current scanline is between the two edge vertices

    int dx = v1->x - v0->x;
    float t = (float)(scanline_y - v0->y) / (float)dy; // Progress along y: 0 at v0->y, 1 at v1->y
    *x = (int)(v0->x + t * dx + 0.5f); // Interpolated x at this y, rounded

    return true;
}
```

This tells us if and where an edge crosses the current scanline y-axis. Inside `drawTriangle` we need to check each edge for intersection and decide what the left and right bounds of our horizontal span will be:

```c
static inline void updateSpan(int x, int *left_x, int *right_x) {
    if (x < *left_x) *left_x = x;
    if (x > *right_x) *right_x = x;
}

void drawTriangle(const Triangle *tri, uint32_t color) {
    const Vec2 *a = &tri->v0, *b = &tri->v1, *c = &tri->v2;

    int top_y = int_min(a->y, int_min(b->y, c->y));
    int bot_y = int_max(a->y, int_max(b->y, c->y));

    for (int scan_y = top_y; scan_y <= bot_y; scan_y++) {
        int left_x = INT_MAX, right_x = INT_MIN, x;

        if (crossesScanlineAt(a, b, scan_y, &x)) updateSpan(x, &left_x, &right_x);
        if (crossesScanlineAt(b, c, scan_y, &x)) updateSpan(x, &left_x, &right_x);
        if (crossesScanlineAt(c, a, scan_y, &x)) updateSpan(x, &left_x, &right_x);

        for (int scan_x = int_max(left_x, 0); scan_x <= int_min(right_x, WIDTH - 1); scan_x++) {
            PIXEL(scan_x, scan_y) = color;
        }
    } 
}
```

<figure>
  <img src="{{ '/images/lesson_03_filled_triangles.png' | relative_url }}" alt="Filled triangles" style="width:100%">
  <figcaption>Filled triangles</figcaption>
</figure>

## Degenerate triangles
We now have our filled triangles. You might notice the single pixel orange triangle with all three vertices coincident did not render. These types of triangles are called degenerate where two or all three of the vertices are coincident. You can generally assume that any triangle definition loaded from a 3D file format like [Wavefront OBJ](https://en.wikipedia.org/wiki/Wavefront_.obj_file) and [glTF](https://en.wikipedia.org/wiki/GlTF) will not contain degenerate triangles. However, as you transform the position of geometry by moving the camera in a 3D engine, it's possible to end up viewing triangles edge on and the renderer will occasionaly be passed such degenerate zero area triangles. It's up to the engine to decide whether to render degenerate lines with no area as a single pixel or a single edge, or to skip rendering them altogether. Here are four additional triangles, two valid ones that are either 2 pixels wide or tall, and two that are 1 pixel wide or tall degenerate triangles with no area:

```c
    // degenerate: single point    — tucked in corner of bottom-right cell
    Triangle tri6 = { { .x = 570, .y = 370 },
                      { .x = 570, .y = 370 },
                      { .x = 570, .y = 370 } };
    // near-horizontal sliver      - left-side middle
    Triangle tri7 = { { .x =  20, .y = 195 },   // left
                      { .x = 180, .y = 195 },   // right
                      { .x = 180, .y = 194 } }; // 1 pixel above right
    // near-vertical sliver        - bottom-left between yellow and cyan tris
    Triangle tri8 = { { .x = 200, .y = 210 },   // top
                      { .x = 200, .y = 380 },   // bottom
                      { .x = 201, .y = 295 } }; // middle, 1 pixel right
    // degenerate: horizontal line - in the middle between green and cyan tris
    Triangle tri9 = { { .x = 240, .y = 200 },   // left
                      { .x = 310, .y = 200 },   // middle
                      { .x = 380, .y = 200 } }; // right
    // degenerate: vertical line   - bottom-right between cyan and magenta tris
    Triangle tri10 = { { .x = 400, .y = 220 },   // bottom 
                       { .x = 400, .y = 380 },   // top 
                       { .x = 400, .y = 300 } }; // middle
...
    drawTriangle(&tri6, colors[ORANGE]);
    drawTriangle(&tri7, colors[PURPLE]);
    drawTriangle(&tri8, colors[WHITE]);
    drawTriangle(&tri9, colors[GRAY]);
    drawTriangle(&tri10, colors[SILVER]);
```

<figure>
  <img src="{{ '/images/lesson_03_extra_test_triangles.png' | relative_url }}" alt="Extra test triangles" style="width:100%">
  <figcaption>Extra test triangles</figcaption>
</figure>


All of the new triangles render as expected except the horizontal degenerate triangle `tri9`, along with the three coincident vertices triangle `tri6` that wasn't rendering previously. For both, in all three `crossesScanlineAt` checks the `dy` value is 0 and return `false` so that in `drawTriangle`, `left_x` is still `INT_MAX` and `right_x` is still `INT_MIN`, and `scan_x` in the fill loop never passes the conditional check `scan_x <= int_min(right_x, WIDTH - 1)`.

The process of choosing triangles to ignore drawing is called *culling* or removing. There are various ways to handle culling triangles such as degenerate triangles or triangles that are so far away they render with zero area, or triangles that are never visible on the screen. Culling can be used to maintain visual correctness, another reason is to reduce the number of computations carried out in the rendering pipeline. We'll revist culling later as it becomes more important in our rendering pipeline.

For now, a simple check that would handle rejecting drawing all forms of degenerate triangle cases would be to cull any triangle with no area. The naieve `area = 1/2 * base * height` formula is relatively expensive in terms of [CPU instruction cycles](https://en.wikipedia.org/wiki/Cycles_per_instruction), requiring several square root and divide instructions, which can take somewhere around 40-60 cycles on a modern processor. A much cheaper method that gives us the same information is to find the [cross product](https://www.3blue1brown.com/lessons/cross-products#two-dimensions) of two triangle vectors. "Clocking" in at 2 multiplies and 5 subtracts this method is far more efficient, and can take somewhere between 3-10 total instruction cycles. The cross product is primarily useful in graphics programming because it gives us the surface normal, which is a vector perpendicular to the surface. The normal is used for computing things like lighting intensity and whether hidden surfaces should be rendered or not (backface culling). For now we're only interested in using the cross product formula to derive an area of our triangle.

## Using cross product to determine area
When you take the cross product of two 2D vectors, the resulting vector is a "3D" vector that looks like (0, 0, z) where the z component value is twice the area of the triangle formed by edges of the two vectors. `(0, 0, z)` also is perpendicular to the plane containing the first two vectors at their shared origin. This is not the usual way the cross-product is introduced, but I think it's a more simple introduction.

<figure>
  <video width="100%" controls loop muted>
    <source src="{{ '/images/lesson_03_CrossProductAnimation.mp4' | relative_url }}" type="video/mp4">
  </video>
  <figcaption>Finding the triangle area via cross product</figcaption>
</figure>

In the following I'll work through the steps in the video, using the general 3D cross product formula to arrive at a simple cross product formula for two 2D vectors `u` and `v` as:

`u × v = (0, 0, u.x*v.y - u.y*v.x)`

As we'll see, the `z` value `u.x*v.y - u.y*v.x` is a scalar that represents twice the area of the triangle built on vectors `u` and `v`, which we derive from our existing position vectors from the origin `a`, `b` and `c`. We could try picking two of these original position vectors `a`, `b` and `c` and applying the cross product to them, but the resulting area derived from the cross product would be with respect to the triangle formed with `b` and `c` and the origin and not the triangle formed by `a` `b` and `c`. Looking at the video above, if we used the current vectors `b` and `c` from the origin at `(0, 0)`, they would describe a triangle from the origin with vector `a` in the middle, not the triangle `abc`. However, we can derive two displacement vectors from our three positional vectors if we choose one of the vectors `a`, `b`, `c` as a common origin and subtract it from the other two vectors. Looking at the video, we choose vector `a` as our origin and subtract vector `a` from vector `b` to form vector `u`, and subtract vector `a` from vector `c` to form vector `v`. These two new vectors from the same origin form the edges of the triangle we want.

`u = (b.x - a.x, b.y - a.y)`
`v = (c.x - a.x, c.y - a.y)`

and substituting these values of `u` and `v` and a `z` value of 0 for each vector into the general 3D cross product definition gives:

`u × v = (u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x)`

The `z` values of 0 cancel out the first two terms:

```
u × v = (u.y*0 - 0*v.y, 0*v.x - u.x*0, u.x*v.y - u.y*v.x)
u × v = (0, 0, u.x*v.y - u.y*v.x)
```

We can plug the original component values of `a`, `b` and `c` back in to the components of `u` and `v` to express the computation with our original three positional vectors:

`u × v = (0, 0, ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)))`

The z-component of the cross product is thus the parallelogram formed by the sides of the triangle `u` and `v`:

`z = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)`

which is what we wanted to prove. With these two multiplications and five subtractions we've computed twice the area of our triangle. We don't actually need to divide by 2 to get the area, we only care that the area is not zero, which means none of our vertices are coincident and we have a non-degenerate triangle.

## Removing degenerate triangles
We can use the cross product formula for 2D vectors that we just derived to skip drawing any degenerate triangles with 2 or more coincident vertices that will have an area of 0:

```c
void drawTriangle(const Triangle *tri, uint32_t color) {
    const Vec2 *a = &tri->v0, *b = &tri->v1, *c = &tri->v2;

    // The cross product for 2D vectors has only a z-component: (0, 0, b-a)×(c-a).
    // That z value equals the signed parallelogram area spanned by u and v.
    // Zero means the vertices are collinear — no triangle to draw.
    int crossProductZ = (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
    if (crossProductZ == 0) return;

    // Find the vertical span of the triangle
```

<figure>
  <img src="{{ '/images/lesson_03_triangle_areas_not_zero.png' | relative_url }}" alt="Lesson 03 Triangles with Non-zero Area" style="width:100%">
  <figcaption>Triangles with non-zero area</figcaption>
</figure>

This gives us all triangles that have no coincident vertices. You might wonder, what does it mean for that signed paralellogram area representing the `z` component of the cross product to be negative or positive? Let's check:

```c
    int crossProductZ = (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
    if (crossProductZ < 0) return; // negative signed paralleogram area of u × v
```

<figure>
  <img src="{{ '/images/lesson_03_triangle_areas_gte_zero.png' | relative_url }}" alt="Triangle Areas Greater Than or Equal to 0" style="width:100%">
  <figcaption>Triangles with area greater than or equal to 0</figcaption>
</figure>

Two of our zero area degenerate triangles draw: `tri5` and `tri10`. `tri6` and `tri9` pass the area check but have `dy == 0` as mentioned above and never fill their horizontal spans. But why is `tri3` showing up, and why aren't all of the other filled triangles? This means `tri3` has a positive area and the other filled triangles all have negative areas. If you noticed at the end of the video above, the cross product vector that is perpendicular to the triangle `abc` surface can either point up along positive-z or down negative-z, depending on the order of `u × v` or `v × u`. We converted those two vectors back into the original triangle `abc` component values, and the video mentions that when the points `a` `b` and `c` of a triangle are counter-clockwise, the cross product points along the positive-z axis, and when they're clockwise the cross product points along the negative-z axis.

What does it mean for these position vectors to be clockwise or counter-clockwise? The [OpenGL wiki](https://wikis.khronos.org/opengl/Face_Culling#Winding_order) describes this as the *winding order* of a triangle.

```
Given an ordering of the triangle's three vertices, a triangle can appear to have a clockwise winding or counter-clockwise winding. Clockwise means that the three vertices, in order, rotate clockwise around the triangle's center. Counter-clockwise means that the three vertices, in order, rotate counter-clockwise around the triangle's center.
```

Look at the definition for `tri3`:

```c
    Triangle tri3 = { { .x =  20, .y = 210 },   // top-left
                      { .x = 180, .y = 210 },   // top-right
                      { .x = 100, .y = 380 } }; // bottom
```

Following the points in order across their appearance on screen shows them to be in clockwise order. Looking at all of our other filled triangles, they are listed in counter-clockwise order:

```c
    Triangle tri0 = { { .x =  20, .y = 170 },   // bottom-left
                      { .x = 180, .y = 170 },   // bottom-right
                      { .x = 100, .y =  30 } }; // top-middle
```

This is what the video mentions at the end: when a cross product has a positive-z component value, the triangle has counter-clockwise winding, and clockwise when it is negative. We can change the winding order of `tri3`, or the winding order of all the other triangles. We'll talk more later about this cross-product winding check and how it relates to culling triangles, but you might already be able to see how it can be very useful for optimizing a 3D engine. To end this section, let's fix it so all of the filled triangles render. That's a two step process, see if you can do it yourself first.

In `drawTriangle` change the test back to rejecting triangles with areas greater than 0:

```c
    // Zero means two or more of the vertices are collinear — no triangle to draw.
    int crossProductZ = (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
    if (crossProductZ > 0) return;

    // Find the vertical span of the triangle, clamp to vertical screen bounds
    int top_y = int_max(
```

and in `main` change the winding of `tri3` so it now also has a negative cross product z-component:

```c
    // one horizontal edge         — bottom-left cell
    Triangle tri3 = { { .x =  20, .y = 210 },   // bottom-left
                      { .x = 100, .y = 380 },   // top-middle
                      { .x = 180, .y = 210 } }; // bottom-right
```

<figure>
  <img src="{{ '/images/lesson_03_extra_test_triangles.png' | relative_url }}" alt="Triangle Areas Less Than or Equal to 0" style="width:100%">
  <figcaption>Triangles with area less than or equal to 0</figcaption>
</figure>

At this point, we have the basic building blocks of a 3D renderer. We could manually describe all of the triangles in a static scene just as we've done with this handful of triangles, or load them from a 3D model file, and apply this kind of triangle shading to each triangle with a single manually specified RBG color and render millions of triangles to draw an image from your favorite modern video game. The complexity of a 3D engine comes from dynamically changing that view, moving the camera around the scene, applying positional transformations to those triangles as well as lighting and texturing them and a million other operations. Below is a comparison of the current version of my engine with unlit textures on a 10 million triangle scene to an [OpenGL version](https://3dworldgen.blogspot.com/2017/01/san-miguel-scene.html) of the San Miguel scene by Guillermo Llaguno.

<div style="display:flex; gap:1rem;">
  <figure style="flex:1; margin:0;">
    <img src="{{ '/images/lesson_03_san-miguel_unlit.png' | relative_url }}" style="width:100%">
    <figcaption>Current Softrend unlit San Miguel</figcaption>
  </figure>
  <figure style="flex:1; margin:0;">
    <img src="{{ '/images/lesson_03_san-miguel_textured.png' | relative_url }}" style="width:100%">
    <figcaption>3D World Textured San Miguel</figcaption>
  </figure>
</div>

# Note on linear algebra
Graphics programming is tightly linked with linear algebra. You can just memorize the general formulas and applications that fall from them without understanding the math. Linear interpolation (LERP) just finds a point along a straight edge given one of the point's components, and the cross product tells us the vector perpendicular to two vectors in a plane, and also can be used to get the triangle area formed by those two vector edges.

But if you've made it this far you clearly enjoy learning from [first principles](https://en.wikipedia.org/wiki/Posterior_Analytics), from simple things we know through sense observation to more abstract and generalized conclusions. I think geometry is the first movement from our sense observations of continuous quantities in the world to a [formal abstraction](https://www.math.purdue.edu/~goldberg/Math460/Euclid-BKI.pdf). We never actually "see" perfect points, straight or parallel lines, planes, right angles, triangles, squares, etc. But we intuit those concepts and they become the first principles of geometry, from which fall other branches like trigonometry and calculus. Branches of math that proceed from our perceptions of discrete quantities build from basic arithmetic, like number and graph theory. Linear algebra is a blend of discrete and continuous math - matrices are discrete arrays of numbers, but they're used to transform continuous geometric space.

Determinants are a geometric property of matrix transformations. They give the scale factor the transformation matrix will apply to an area. If you scale a matrix by 2, the determinant doubles, if you rotate it the determinant stays the same
As an aside, you might see this formula in other forms, like:

```
| a c |
| b d | = ad - bc
```

This is the same thing as the formula above, `a = b.x - a.x`, `b = c.y - a.y`, `c = b.y - a.y`, `d = c.x - a.x`, just factored more simply. Using the language of matrices, the signed area formula is the determinant of two vectors `u` and `v` as columns or rows depending on whether we're using a left-handed or right-handed coordinate system. You may also see this formula referred to as the "shoelace" formula, this is a special case of that formula that sums the determinant for all edge pairs in a polygon.

# Final notes
`drawLine` is now dead code that is not called anywhere, but we'll keep it around for now. You never know when we'll want to draw a line.

## Winding order and the cross product sign

The industry-standard front-face convention is **CCW winding**: when looking at
a visible face, its vertices are listed counterclockwise. This convention is
used by all major 3D tools (Blender, Maya) and model formats (OBJ, glTF, FBX),
regardless of the tool's native coordinate system.

The cross product z-component encodes this: for two edge vectors **u** and **v**
derived from a triangle's vertices, `z = u.x*v.y - u.y*v.x` gives the signed
area of the parallelogram they span. The sign tells you the winding:

- **y-up space** (model/camera space): CCW triangle → **positive** z
- **y-down space** (screen space): CCW triangle → **negative** z

The triangle hasn't changed — the y-flip when projecting to screen space inverts
the sign. The cull condition flips with it:

| Space         | Visible (CCW) | Cull condition |
|---------------|---------------|----------------|
| Camera (y-up) | z > 0         | z <= 0         |
| Screen (y-down)| z < 0        | z >= 0         |

Most rasterizers perform the cull in screen space since the projected 2D
vertices are already available there. The sign inversion is not a bug — it is
the expected consequence of the y-flip. You can also negate the formula once
(`v.y*u.x - v.x*u.y`) so that CCW gives positive in screen space too, trading
an unintuitive comparison for an unintuitive formula.

Backface culling — discarding triangles whose winding has flipped in screen space
because the camera is viewing them from behind — is the practical application of
this sign test. It is covered once the transform pipeline (model → world → camera
→ screen) is in place, since that is the context that makes the sign flip
meaningful.
