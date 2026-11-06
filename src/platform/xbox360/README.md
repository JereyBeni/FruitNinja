# Xbox 360 platform backend

Target: modified Xbox 360 consoles running RGH or JTAG. Retail consoles are
outside this port's scope.

## Current stage: display and controller bring-up

This directory is currently a standalone XSDK title, not yet the complete
Fruit Ninja game port. It checks the first pieces of the platform layer:

- XEX creation through `xsdk_add_xex`.
- XSDK GPU ownership and 720p presentation.
- Frame submission and a solid-colour clear.
- Controller polling through `XamInputGetState`; Back exits.

The title uses XSDK's native Xenos GPU interface. It does not include
Windows `d3d9.h`, call `IDirect3DDevice9`, or depend on Microsoft's
Xbox 360 XDK. XSDK exposes its own API in `xsdk/gpu.h` and supports
Shader Model 3 HLSL and GLSL shader compilation.

## Build

Build XSDK first, following its `docs/GETTING_STARTED.md`, so
`build/libxsdk.a`, the PowerPC toolchain and shader tools are available.

Then configure this directory as its own CMake project:

```sh
cmake -S src/platform/xbox360 -B build/xbox360 -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/xsdk/cmake/xsdk.cmake" \
  -DXSDK_TOOLCHAIN="$HOME/xsdk-gcc"
cmake --build build/xbox360
```

The expected output is
`build/xbox360/fruit-ninja-xbox360.xex`.

This stage has not yet been built or tested on a console. Use an XSDK-supported
RGH/JTAG development setup for hardware validation.

## Next integration stages

1. Add the Xbox 360 platform option to the main Fruit Ninja CMake build.
2. Implement the game entry point, fixed-step loop and XAM controller input.
3. Port file access and save handling to XSDK paths and XAM saves.
4. Port audio to `xsdk_audio_*` or an XSDK-compatible audio layer.
5. Implement the renderer against XSDK's Xenos GPU API and adapt the game's
   GLSL shaders. The existing Wii GX renderer is not reusable on Xenos.
6. Validate asset byte order, texture formats, memory use and frame pacing
   on actual hardware.

The current title is a platform smoke test only. It deliberately does not
pretend that the game's renderer or engine is already running.
