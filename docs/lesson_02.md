---
layout: lesson
title: "Lesson 02: Drawing to the screen"
lesson_label: "Lesson 02"
prev_lesson: /lesson_01
next_lesson: /lesson_03
---

![Lesson 02 Animated Triangle]({{ '/images/lesson_02_animated_triangle.gif' | relative_url }}){: width="100%"}

## Clean up old code
We're not going to need to print the channel values for a buffer pixel again, so we can remove that. I changed the timing code to be more accurate. We cap the renderer to the target frame rate by initializing `nextFrameTime` to `fenster_time()` and advancing it by the desired frame time (1000ms / FPS) each frame loop, sleeping until it's reached. If the OS wakes us late, the next sleep is shortened by exactly the amount of oversleep, and if `remainingMS > 0` fails the check, we don't sleep and the next frame will also have a shorter sleep until it catches up.

[Lesson 02: Initial main.c](https://github.com/TurpeNescire/softrend-3d/tree/acf002b72695acdcf68a3dc822e6e2e747339ab3/src/main.c)

## Drawing our first triangle

Drawing anything to the screen at this point involves setting the color value of our `buffer` pixels. Because we declared `buffer` in global scope, it is automatically 0 initialized along with all other global scoped variables, which as we saw means the color black when all color channels are set to 0 in RGB. We have created a window `WIDTH` pixels wide and `HEIGHT` pixels tall, so there are 600x400 or 240,000 pixels in our window. We could have declared `buffer` as a row-major multi-dimensional array, `uint32_t buffer[HEIGHT][WIDTH]` which is of type `uint32_t **`, and assigned `window.buf = (uint32_t *)buffer;`. This simplifies the notation for element access, allowing us to access elements with `buffer[y][x]`, but hides the flat nature of the memory array and requires us to remember the array is row major. I think both forms compile to the same machine code, so it's mostly a matter of preference - I like using a flat array. For the flat buffer array we can create a convenience macro for getting and setting pixel information, and add a new color constant `WHITE` for later use:

```c
#define FPS 60 // Targeted frame rate (frequency in hz)

// Framebuffer is row-major
#define PIXEL(x, y) buffer[(y) * WIDTH + (x)]

#define WHITE 0xFFFFFFFF

uint32_t buffer[WIDTH * HEIGHT];

```

This removes any concern over remembering whether we're using row or column major ordering, or remembering how to write the indexing expression for each element.

The first step to drawing a triangle is to determine its vertices. If we want a 300 pixel wide, 200 pixel tall isosceles triangle that fits nicely in our screen, we can use the following declarations followed by calling `PIXEL` for each point:

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

If you compile and run you should now see 3 single pixels outlining the vertices of a triangle. 

[Updated main.c](https://github.com/TurpeNescire/softrend-3d/tree/f27ea7862ed20db5045cd7be59ae0d14e5abc243/src/main.c)

![Lesson 02 Triangle Vertices]({{ '/images/lesson_02_triangle_verts.png' | relative_url }}){: width="100%"}

How do we connect the dots between two vertices to form an edge? There are several common methods. We need to test each pixel to determine if it's on the line. [Bresenham's line algorithm](https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm) is an oldy that avoids using floating-point division, but is a bit of a black magic algorithm at first glance. For me, [linear interpolation](https://en.wikipedia.org/wiki/Linear_interpolation#Linear_interpolation_between_two_known_points) is the most understandable method for finding a given point along an edge with two known endpoints. Modern FPUs can easily handle the simple division we'll need to use.

### Linear interpolation

<video width="100%" controls autoplay loop muted>
  <source src="{{ '/images/lesson_02_LerpDerivation.mp4' | relative_url }}" type="video/mp4">
</video>

We have two endpoints on an edge, `(x0, y0)` and `(x1, y1)`, and for any `x` value along that line we want to find the corresponding `y`.

The differences between the `x` and `y` values of the two points are shown in the video as `dx` and `dy`, the legs of the right triangle formed by the two endpoints. For any point `x` along the edge, a smaller similar triangle is formed with legs `(x - x0)` and `(y - y0)`. Because the triangles are similar, their corresponding sides are proportional:

`dy / dx = (y - y0) / (x - x0)`

We want to get to a simple equation to solve for `y`. This is typically given as `y = y0 + t * dy`. By rearranging the above equation with cross multiplication:

`(x - x0)/dx = (y - y0)/dy`

We define `t = (x - x0) / dx` as the *interpolation parameter* — the ratio of how far `x` has traveled across `dx`, ranging from `0` at `x0` to `1` at `x1`. Substituting `t` into `(x - x0)/dx = (y - y0)/dy`:

`t = (y - y0)/dy`

Multiplying `t` by `dy` scales the ratio `t` to the full height of the triangle, giving the displacement (distance) of `y` from `y0`:

`t * dy = y - y0`

Adding `y0` gives us the final linear interpolation formula for an unknown coordinate `y`:

`y = y0 + t * dy`

We derived the interpolation formula starting from a geometric proportion of triangle sides. Typically the formula is derived algebraically from the *point-slope form* of a line, `y = y0 + m(x - x0)`, by substituting `m = dy/dx` and letting `t = (x - x0)/dx`. Hopefully having a visual grounding is helpful in understanding what the `t` ratio is and how it is used to scale `dy` at various points along the line, and why we add `y0` back to anchor `y` to an actual position and not as a relative displacement.

### Drawing edges with linear interpolation
Linear interpolation is one of the most useful algorithms in graphics programming and you'll see and use it often. It's worth spending the effort to understand it now. Before moving on, I would recommend pausing and trying to implement the line drawing algorithm to draw a triangle by yourself.

My first naieve attempt looked like this:

```c
void drawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = x1 - x0, dy = y1 - y0;

    for (int x = x0; x <= x1; x++) {
        float t = (float)(x - x0) / dx; // progress along x: 0 at leftX, 1 at rightX
        int y = (int)(y0 + t * dy);     // interpolated y at this x
        PIXEL(x, y) = color;
    }
}
```

I put the `drawLine` definition just before `main`, and in `main` at the top I defined a few vertices and attempted to draw a triangle by calling `drawLine` three times to draw the three edges:

```c
        .buf    = buffer
    };

    // vertices of a 100 pixel wide, 300 pixel tall screen centered triangle
    int x0 = 250, y0 = 350; // bottom-left
    int x1 = 350, y1 = 350; // bottom-right
    int x2 = 300, y2 =  50; // apex

    drawLine(x0, y0, x1, y1, WHITE);
    drawLine(x1, y1, x2, y2, WHITE);
    drawLine(x2, y2, x0, y0, WHITE);
```

This just draws a straight horizontal line. Thinking it over, what happens when it draws the second edge `drawLine(x1, y1, x2, y2, WHITE)`?

`dx` becomes -50, `dy` becomes -300, and in the first iteration of the `for` loop `x` is set to 350 and immediately fails the `x <= x1` test because `x1` is 300, so that edge never gets drawn. Similarly, the third edge `drawLine(x2, y2, x0, y0, WHITE)` never draws because `x` is set to 300 and fails `x <= x1` because `x1` is 250. So the only edge that gets drawn is `drawLine(x0, y0, x1, y1, WHITE)` as a 100 pixel wide horizontal line.

The fix is easy, we just need to make sure that we start iterating from the leftmost `x` to the rightmost inside `drawLine`:

```c
    int dx = x1 - x0, dy = y1 - y0;

    int leftX = x0, rightX  = x1;

    if (x0 > x1) { leftX = x1; rightX  = x0; }

    for (int x = leftX; x <= rightX; x++) {
        float t = (float)(x - leftX) / dx; // progress along x: 0 at leftX, 1 at rightX
        int y = (int)(y0 + t * dy);        // interpolated y at this x
        PIXEL(x, y) = color;
    }
```

Now I get a [bus error](https://en.wikipedia.org/wiki/Bus_error) on my machine. Looks like we tried to access an out of bounds pixel. Adding a debuf print to the loop we can find where:

```c
        int y = (int)(y0 + t * dy);
        printf("PIXEL(%d, %d)\n", x, y);
        PIXEL(x, y) = color;
```

The first edge draws correctly from `(250, 350)` to `(350, 350)`, the second edge is where we crash:

```console
PIXEL(300, 350)
PIXEL(301, 356)
PIXEL(302, 362)
PIXEL(303, 368)
PIXEL(304, 374)
PIXEL(305, 381)
PIXEL(306, 387)
PIXEL(307, 393)
PIXEL(308, 399)
PIXEL(309, 405)
[1]    46141 bus error  ./softrend
```

Our screen is 400 pixels tall and we tried to write to y = 405, which would be below the screen. But we want to be drawing from an apex `y` position at 50 down to the base horizontal line `y` position at 350. When `x0 > x1` we failed to swap the `y` values also and we lost the relative positioning of `y` with respect to `x`. In other words, we changed the line's endpoint positions from `(350, 350)` and `(300, 50)` to `(300, 350)` and `(350, 50)`. Easy fix, we'll add a similar swap for y.

```c
void drawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = x1 - x0, dy = y1 - y0;

    int leftX = x0, rightX  = x1;
    int topY  = y0, bottomY = y1;

    // Swap both x and y together to preserve the endpoint pairing —
    // topY must be the y value at leftX, not rightX, or t interpolates
    // in the wrong direction.
    if (x0 > x1) { 
        leftX = x1; rightX  = x0;
        topY  = y1; bottomY = y0;
    }

    for (int x = leftX; x <= rightX; x++) {
        float t = (float)(x - leftX) / dx; // progress along x: 0 at leftX, 1 at rightX
        int y = (int)(topY + t * dy);      // interpolated y at this x
        PIXEL(x, y) = color;
    }
}
```

![Lesson 02 Triangle With Height Gaps]({{ '/images/lesson_02_triangle_height_gaps.png' | relative_url }}){: width="100%"}

That's odd. It looks like a triangle now, but there are a bunch of gaps in the vertical line. If you noticed the debug pixel position values, `x` was incremening by 1 but `y` was skipping 6 or 7 pixels each iteration. That's where the vertical gaps are coming from. How do we fix this? The difference in `x` values is 100 and the difference in `y` values is 300. If we iterated over the range of `y` values (300 times) we should see the interpolated `x` values increase after 6 or 7 iterations. We want to loop over the dominant axis with the greatest difference (length) and interpolate the positions of the other minor axis.

```c
void drawLine(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);

    // Sort by the dominant axis
    // x-dominant: ensure leftX < rightX
    // y-dominant: ensure topY < bottomY
    // x and y must swap together — a partial swap mixes coordinates between the endpoints
    int xDominant = (dx >= dy);
    int leftX = x0, rightX  = x1;
    int topY  = y0, bottomY = y1;
    if (xDominant ? leftX > rightX : topY > bottomY) { 
        leftX = x1; rightX  = x0;
        topY  = y1; bottomY = y0;
    }

    if (xDominant) {
        for (int x = leftX; x <= rightX; x++) {
            float t = (float)(x - leftX) / dx; // progress along x: 0 at leftX, 1 at rightX
            int y = (int)(topY + t * dy);      // interpolated y at this x
            printf("PIXEL(%d, %d)\n", x, y);
            PIXEL(x, y) = color;
        }
    } else {
        for (int y = topY; y <= bottomY; y++) {
            float t = (float)(y - topY) / dy; // progress along y: 0 at topY, 1 at bottomY
            int x = (int)(leftX + t * dx);    // interpolated x at this y
            printf("PIXEL(%d, %d)\n", x, y);
            PIXEL(x, y) = color;
        }
    }
    printf("\n");
}
```

This got rid of the gaps on the second edge from `(300, 50)` to `(350, 350)` for the `drawLine(x1, y1, x2, y2, WHITE)` call, but it printed that side twice. If you look at the third edge called with `drawLine(x2, y2, x0, y0, WHITE)`, the debug printing shows its `x` values also increasing from `(300, 50)` to `(350, 350)`, just like the second edge. We computed the `dx` value from the absolute difference of `x1 - x0`, but doing so lost the sign of `dx`. If we're drawing the edge starting from `(300, 50)` and ending at `(250, 350)` then the difference between `leftX` and `rightX` is -50, not 50 in the `!xDominant` branch. When we swap x and y positions, the sign of the opposite axis's difference becomes stale and needs to be recalculated. We can add a line to each branch doing this:

```c
    if (xDominant) {
        for (int x = leftX; x <= rightX; x++) {
            dy = bottomY - topY; // recompute minor-axis delta from the sorted endpoints
            float t = (float)(x - leftX) / dx; // progress along x: 0 at leftX, 1 at rightX
            int y = (int)(topY + t * dy);      // interpolated y at this x
            printf("PIXEL(%d, %d)\n", x, y);
            PIXEL(x, y) = color;
        }
    } else {
        for (int y = topY; y <= bottomY; y++) {
            dx = rightX - leftX; // recompute minor-axis delta from the sorted endpoints
            float t = (float)(y - topY) / dy; // progress along y: 0 at topY, 1 at bottomY
            int x = (int)(leftX + t * dx);    // interpolated x at this y
            printf("PIXEL(%d, %d)\n", x, y);
            PIXEL(x, y) = color;
        }
    }
```

![Lesson 02 Triangle With Height Gaps Fixed]({{ '/images/lesson_02_triangle_height_fixed.png' | relative_url }}){: width="100%"}

Now the third edge properly draws down and to the left from the apex to the bottom left vertex, and the pixel gaps on the minor axis edges are gone. There are a few edge case concerns to think about. What if our `t` calculations divide by a `dx` or `dy` that is 0? Can this happen? It turns out only in one case. Can you spot it?

When both `dx` and `dy` are 0, we have a degenerate edge where both vertices are the same. In this case, we end up on the `xDominant` branch and divide by `dx` that is equal to 0. We want to handle this edge case by just printing a single pixel at `(x0, y0)` or `(x1, y1)` - it doesn't matter which. Add this to the top of `drawLine`:

```c
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    // Degenerate case: both endpoints identical, draw a single pixel and bail
    if (dx == 0 && dy == 0) { PIXEL(x0, y0) = color; return; }
```

[Click here](https://github.com/TurpeNescire/softrend-3d/tree/3e64b7a657379f022f4c1e89039be450f2365c4e/src/main.c) for the current `main.c`

`drawLine` should now handle all edges cleanly, but it's best to check. If we divide the screen up into a 3x2 grid and put a triangle in each, we can fit six example triangles on the screen at once. Replace the `x0`/`x1`/`x2` etc. assignments and the three single `drawLine` calls with:

```c
    // x-dominant (wide/flat)        — top-left cell
    drawLine( 20, 170, 180, 170, WHITE);
    drawLine(180, 170, 100,  30, WHITE);
    drawLine(100,  30,  20, 170, WHITE);

    // y-dominant (tall/narrow)      — top-middle cell
    drawLine(270, 180, 330, 180, WHITE);
    drawLine(330, 180, 300,  20, WHITE);
    drawLine(300,  20, 270, 180, WHITE);

    // one vertical edge              — top-right cell
    drawLine(410,  20, 410, 180, WHITE);
    drawLine(410, 180, 580, 100, WHITE);
    drawLine(580, 100, 410,  20, WHITE);

    // one horizontal edge            — bottom-left cell
    drawLine( 20, 210, 180, 210, WHITE);
    drawLine(180, 210, 100, 380, WHITE);
    drawLine(100, 380,  20, 210, WHITE);

    // right triangle                 — bottom-middle cell
    drawLine(220, 220, 220, 380, WHITE);
    drawLine(220, 380, 380, 380, WHITE);
    drawLine(380, 380, 220, 220, WHITE);

    // two coincident vertices        — bottom-right cell
    drawLine(500, 220, 500, 220, WHITE);
    drawLine(500, 220, 420, 370, WHITE);
    drawLine(420, 370, 500, 220, WHITE);

    // single point — tucked in corner of bottom-right cell
    drawLine(570, 370, 570, 370, WHITE);
    drawLine(570, 370, 570, 370, WHITE);
    drawLine(570, 370, 570, 370, WHITE);
```

Using the interpolation formula, we can iterage over all of the x values between two points to find their corresponding y values.  But in the case where the difference in `y` values is greater than the different in `x` values, we'll end up with gaps in our Just as we solved for the `y` coordinate of a point on the line with a given `x` coordinate, we can derive the formula for the reciprocal as `x = x0 + t * dx`. If you imagine two types of lines, one set where the `dx` is greater than or equal to `dy` and the other where `dy` is greater than `dx`, we can 

![Lesson 02 Triangle Edge Cases]({{ '/images/lesson_02_triangle_edge_cases.png' | relative_url }}){: width="100%"}

## in progress
A 3D object is typically described as a collection of meshes that contain geometric information about the triangle or quad faces of the object. Triangles are the easiest to work with because all points on the triangle are coplanar, and the face has only one surface normal. We'll come back to all of this later when we learn to import a geometry from an object file.



