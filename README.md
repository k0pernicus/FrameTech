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

This repository includes extern repositories / dependencies, like `imgui`, `vma`, or `glm`.
Do not forget to run a `git submodule update` after cloning the _FrameTech_ repository.

The Vulkan engine uses `VMA` as memory allocator dependency. 

Thanks to the ImGui and GLM teams for providing such great tools for free.

## Screenshots

![State at December 15 of 2022](docs/images/state_12152022.png "State of 15th of December, 2022")

## TODO

1. Multiple buffers allocation using **one** _VkBuffer_ only,
2. Improve the shaders bindings,
3. Make a todo statement about what to move from pipeline / render / swapchain / ... to ...,
4. Make the configuration external to the build (like FPS limit),
5. [OPTIONAL] Multi-platforms.
