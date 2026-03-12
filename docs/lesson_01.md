---
layout: lesson
title: "Lesson 01: Setup"
lesson_label: "Lesson 01"
next_lesson: /lesson02
---

![Lesson 01 banner]({{ '/images/lesson_01_banner.jpg' | relative_url }}){: width="100%"}

# Lesson 01: Purpose and Setup

## Why a software 3D renderer?
Since the late 1990s and early 2000s, most applications that employ 3D rendering have moved from handling rendering calculations on the CPU and RAM to handling them on dedicated GPU hardware.  Quake was one of the first major game releases to support GPU rendering with id softwares 1997 release of GLQuake that could only realistically be run on expensive workstations that supported OpenGL. OpenGL existed before Quake (created around 1992) but was targeted mainly for expensive graphics rendering workstation hardware like SGI. In 1996 a small company called 3dfx released one of the first consumer graphics cards with their own proprietary API called Glide.  After GLQuake's release, 3dfx released another API called MiniGL that was a wrapper for the subset of OpenGL 1.1 that was used in GLQuake, allowing home users to use 3D acceleration for the first time and kicking off the consumer video card market race of the late 1990s. Over the next few years, most games still defaulted to using software rendering but also added optional support for 3D hardware based rendering via Glide, but increasingly also OpenGL or Microsoft's DirectX API. Games like Half-life (1998) and Unreal (1998) defaulted to software rendering but allowed for hardware acceleration via Glide, OpenGL and DirectX. Id software is also credited with being one of the first companies to release a commercial game that required a dedicated 3D video card with their 1999 release of Quake III Arena.  Only a few years later, most AAA game releases no longer included fallback software renderers.  In 2003, RAD Tools released an API called [Pixomatic](https://www.gamedeveloper.com/game-platforms/rad-launches-pixomatic----new-software-renderer) that allowed companies to write code that would run on any available API including software only, and Unreal Tournament 2004 appears to be one of the last AAA games to allow software fallback for users without a dedicated video card by utilizing Pixomatic. [World of Warcraft (2004)](https://www.pcgamingwiki.com/wiki/World_of_Warcraft#API) also appears to have included a software renderer, but I can't confirm this.

