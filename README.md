# FrameTech

An example of a game and the custom `FrameTech` engine.

**This project is not compatible with a Vulkan SDK > 1.3.211.**

This repository includes:

1. a simple application (game),
2. a custom standard library for FrameTech, called `ftstd`,
3. a custom engine called `FrameTech`.

The custom standard library, `ftstd` (for "FrameTech STanDard library"), is enough to run and debug the engine only, and has no purpose to run in PRODUCTION mode for the moment.

This engine is divided in several namespaces, or parts, like `Graphics`.

## TODO

1. DONE: Get the monitor via GLFW, and set the refresh rate based on the monitor device output,
2. Improve the shaders bindings,
3. Make a todo statement about what to move from pipeline / render / swapchain / ... to ...,
4. Make a proper FPS count + display it on screen,
5. Make the configuration external to the build (like FPS limit),
6. Multi-platforms.
