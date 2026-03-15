![Softrend Banner](docs/images/softrend_banner.png)

# Softrend: 3D Software Rendering from Scratch

Softrend is a minimal 3D software rasterizer in C using the multiplatform [Fenster](https://zserge.com/posts/fenster/) single header library for windowing and input and Sean Barret's [STB Image](https://github.com/nothings/stb/blob/master/stb_image.h) for loading PNG texture files. This project began as an educational attempt to learn how to impelement the entire 3D rendering pipeline from scratch, without using any outside libraries or APIs. Where it goes from here I haven't decided yet!

[Click here](https://turpenescire.github.io/softrend-3d/lesson_01) for the first tutorial lesson if you'd like to follow along with me. I have a nearly complete renderer, and I'm currently writing these lesson tutorials detailing how I implemented each of the following features:

- OBJ + MTL model loading with multi-mesh support and UV texture mapping
- MVP transform pipeline: local → world → camera → screen
- Near-plane clipping via Sutherland-Hodgman
- Depth buffering
- Backface culling and screen-space bounding box culling
- Perspective-correct UV interpolation
- Programmable vertex and fragment shaders
- Lambert, Gouraud, Phong and textured shaders
- CPU-generated mipmapping
- Bilinear filtering
- Shadow mapping
- Multi-threaded tile grid based parallelism
- Particle system

The clip below is taken from a current build of the engine using a scene from [JoeTheBox on itch.io](https://joethejunkbox.itch.io/psx-horror-graveyard-overlooking-sea). You can see most of the above features. I'm working on implementing a skybox, transparency, reflective water, fog, and additional lighting improvements.
<video src="https://github.com/user-attachments/assets/c035b0a7-17c7-4377-ae0a-8c2cd311142a" controls width="100%"></video>

