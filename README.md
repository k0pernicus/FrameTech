# FrameTech

An example of a game and a custom rendering engine, using Vulkan.

**This project is not compatible with Vulkan SDK > 1.3.211.**

This repository includes:

1. a simple application,
2. the rendering engine (`FrameTech`),
3. a custom standard library (`ftstd`),

The custom standard library, `ftstd` (for "FrameTech STanDard library"), is enough to run and debug the engine only, and has no ambition to be
pushed in production.

`FrameTech` has no ambition to be pushed in production either.

## About externs

This repository includes extern repositories / dependencies, like `imgui` or `glm`.
Do not forget to run a `git submodule update` after cloning the _FrameTech_ repository.

Thanks to the ImGui and GLM teams for providing such great tools for free.

## Screenshots

![State at December 15 of 2022](docs/images/state_12152022.png "State of 15th of December, 2022")

## TODO

1. Custom resources allocator,
2. Multiple buffers allocation using **one** _VkBuffer_ only,
3. Improve the shaders bindings,
4. Make a todo statement about what to move from pipeline / render / swapchain / ... to ...,
5. Make the configuration external to the build (like FPS limit),
6. [OPTIONAL] Multi-platforms.
