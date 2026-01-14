# Contributing art

Inside the IGR is a Raspberry Pi 4B running OpenGL shaders. The shader's output is sent to a CRT screen over composite video at a 720x576 resolution.

You don't need to compile IGR's software yourself to contribute, but you are welcome to! In either case it may be useful to read the page on IGR's [software](software.md) and then this page, to get an idea of what's happening under the hood.

If you just want to focus on shader graphics, I will magick them into IGR's source code for you. You can share a shader with me that you run in [Shadertoy](https://www.shadertoy.com), or in a tool like Patricio's [glslViewer](https://github.com/patriciogonzalezvivo/glslViewer). You can even share a [Hydra](https://hydra.ojack.xyz) sketch and most likely we can make it work.

All the source code I refer to on this page is inside the repo's `code-raspi` subdirectory.

### How to declare a sketch

Every sketch lives as a few files in a directory under [src/sketches](/code-raspi/src/sketches/). Sketches must implement a class derived from `SketchBase`. By convention this goes into a pair of files with the sketch's name, e.g. [star_sketch.h](/code-raspi/src/sketches/star/star_sketch.h) and [star_sketch.cpp](/code-raspi/src/sketches/star/star_sketch.cpp) for the "star" sketch's `StarSketch` class.

There is no magical auto-discovery of sketches. Every animation that is available as a station is included and instantiated explicitly in [main_igr.cpp](/code-raspi/src/main_igr.cpp). The includes are at the top, and every sketch is instantiated once in `init_stations()`.

### Sketch lifecycle

This section refers to virtual methods declared on the abstract base class `SketchBase`. These methods correspond directly to the lifecycle events described here. Implementing them is how every sketch gets to handle its lifecycle events.

#### Instantiation

When the `igr` application starts, a single new instance of the sketch class is created. The sketch is not meant to do any real work here, just remember the parameters it is provided:

- The viewport with and height in pixels
- The framebuffer the sketch must render into in each frame

#### Initialization

Immeditaly after instantiation, `igr` calls the sketch's `init()` method. This is where the sketch is expected to compile its program(s), allocate array buffers for attributes like _position_, allocate textures if needed etc.

#### Rendering frames

`igr` calls the sketch's `frame()` function every time a new frame is needed.

- `igr` targets a frame rate of 50, which is what matches the PAL video format.
- `frame()` gets a single argument, which is the time elapsed since the last frame, in seconds. This is more useful than current time if you want to smoothly speed up or slow down animations based on user input, like the position of one of the knobs on the Receiver.
- `frame()` must render to the framebuffer the sketch received in the constructor.

#### Unloading and reloading

If the viewer tunes away from the sketch, `igr` will stop calling its `frame()` method to render frames.

All sketches are instantiated for the whole time the host program is running. Eventually there will be many sketches in IGR, and some of these will use a lot of resources such as input or output textures. It's not smart to keep the sketch's resources in GPU memory when the sketch is not even running.

Therefore, in `unload()` the sketch is expected to free all the GPU resources it has allocated. This includes attribute buffers, shaders, programs, and textures.

If you use a static image in a texture, it's OK to hold on to that image in main memory; `igr` is not really constrained by classic system resources. Also, loading an image from disk would be unnecessarily slow.

When the viewer tunes into the station again, `igr` calls the sketch's `reload()` method so it can allocate its GPU resources again. It's best to call `init()` from here and not do any meaningful work.

`reload()` receives the current time as an argument. The elapsed time passed to the next `frame()` call is counted from this moment.

### The Render Blender

When the viewer is tuning the Receiver (turning the tuning knob continuously), it goes through these states:

* Showing static. In the background, the frames of Sketch X are rendered, but they are never shown.
* Still showing static. As the viewer tunes closer to the frequency of Sketch Y, X is unloaded and Y is reloaded. Now the frames of Y are rendered and discarded.
* Showing a dimmed version of Sketch Y's output, with an overlay containing info about the sketch and the artist.
* Showing the normal output of Sketch Y.
* At the other side of the station, a dimmed version again with the info overlay.
* Static, with the sketch's frames discarded.

The blending of the info overlay over a dimmed view of the sketch's output is why sketches need to render to a framebuffer and not the screen directly.

The sketch's output is blended with the overlay, or discarded in favor of generated static, in `RenderBlender`. It lives in [src/render_blender.cpp](/code-raspi/src/render_blender.cpp).

### How shader code is embedded

So far this page has dealt with the mechanics of the C++ class representing a sketch, but what we really want to write is GLSL shader code.

`igr`'s build scripts provide some machinery so you can edit shader code in separate files with a `.frag` or `.vert` extension. This way, in an IDE, you get GLSL syntax highlighting and editing comforts, instead of "assistance" for editing a generic C string literal.

* Create a file called `shader_template.h` in the sketch's directory. It could look like this:
    ```
    #ifndef STAR_SHADERS_H
    #define STAR_SHADERS_H
  
    constexpr const char *star_frag = R"(
    SRC star.frag
    )";
  
    #endif
    ```
  
* When you invoke [src/sketches/make_shader.sh](/code-raspi/src/sketches/make_shaders.sh) with the name of the sketch's directory, it will read `shader_template.h` and generate `shaders.h` by putting the contents of the shader files into the multi-line string literals. In your sketch, you can now include `shaders.h` and you'll get your shaders as nice C string literals.
* [src/sketches/Makefile](/code-raspi/src/sketches/make_shader.sh) calls the embedding script `make_shader.sh` for every subdirectory under `sketches`.
* The root [build.sh](/code-raspi/build.sh) enters `/src/sketches` and calls `make` to do this before compiling the final program.

### OpenGL version

The Raspbery Pi 4B inside IGR supports OpenGL ES 3.1. This is also known as GLSL ES 3.10, and is indicated by putting `#version 310 es` at the top of the shader file.

This OpenGL version uses `in`/`out` instead of `attribute` and `varying`. It supports the `for` loops needed for ray tracing (although the GPU is not really fast enough for it).

### Simple fragment shaders

[WIP] the SketchFrag class

### Creature comforts

[WIP]
* full-screen quad
* load PNG
* create texture

### Helpful examples

[WIP]

- Using a "real" vertex shader
- Using an image via a texture
- Feedback buffer

### Stress testing

[WIP] will be done! to verify unload/reload behavior
