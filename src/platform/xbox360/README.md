# Xbox 360 platform bring-up

This target is an early native Xenos rendering probe for the modified-console port. It is built as an unsigned XEX with the public xenosworks/xsdk toolchain and is intended for RGH/JTAG hardware.

## What this target currently does

- Opens the Xenos GPU and the 720p presentation path.
- Compiles the GLSL vertex and fragment shaders with `xsdk_add_shaders`.
- Uploads a three-vertex buffer to GPU memory.
- Draws a coloured triangle through `xsdk_gpu_*` and presents each frame.
- Reads the Back button through XAM and shuts down its GPU resources.

This is a renderer bring-up test, not the complete Fruit Ninja game renderer. It does not yet connect Mortar's texture, mesh, UI, audio, filesystem, or save interfaces.

## Build

Build XSDK and its host shader compiler first. Then configure this directory with XSDK's toolchain file:

```sh
cmake -S src/platform/xbox360 -B build/xbox360 -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/xsdk/cmake/xsdk.cmake
cmake --build build/xbox360
```

The output is `build/xbox360/fruit-ninja-xbox360.xex`. Run it only on an RGH/JTAG console that can launch unsigned XEX titles.
