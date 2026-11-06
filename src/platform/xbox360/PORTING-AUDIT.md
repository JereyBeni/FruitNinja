# Xbox 360 port audit: Wii engine and platform path

This audit is based on the current source tree and the XSDK public headers and samples. It is the implementation map for the native Xbox 360 port. It separates verified source behaviour from platform work that is not yet implemented or hardware-tested.

## 1. Engine structure

Fruit Ninja is not a Wii-only engine. The project is split into a shared Mortar/game layer and platform translation units selected by CMake.

- `src/Game.cpp`: platform-independent game singleton, lifecycle and game-specific virtual methods.
- `src/GameWii.cpp`: Wii implementation of the platform-facing `Game` methods; it is the twin of `GameSDL.cpp`.
- `src/engine/`: Mortar core, matrices, render API, asset/resource loading, input abstraction, audio interface, collision, particles, fonts and XML.
- `src/game/`, `src/entities/`, `src/hud/`, `src/screens/`: game simulation, entities, menus, HUD and screens. These should remain shared.
- `src/platform/wii/`: Wii entry point and hardware adapters.
- CMake chooses platform twins for the Game methods, renderer, licensing and audio. Xbox 360 needs to become a third platform choice, not a copy of the Wii build flags.

## 2. Wii startup and frame loop

`src/platform/wii/mainWii.cpp` owns hardware startup and the main loop:

1. Initialise VIDEO and allocate two external framebuffers.
2. Initialise GX and its command FIFO.
3. Initialise the MEM2 texture allocator.
4. Draw the boot splash.
5. Mount SD/USB using libfat.
6. Initialise WPAD and register power/reset callbacks.
7. Call `Game::init()`.
8. Each frame: poll Wiimote IR/buttons, advance the fixed-step driver, dispatch input before each simulation tick, update realtime UI, pump streamed music, render, and present.
9. On exit, shut down the game and WPAD.

The Xbox equivalent should use XSDK's GPU/presentation lifecycle, XAM controller polling, and a monotonic timer. It should preserve the fixed-step simulation cadence and render interpolation rather than moving game updates into the presentation callback.

## 3. Game lifecycle

`src/GameWii.cpp` is a useful template, but contains Wii-specific display-aspect correction, WPAD pointer overlay, and Wii boot-splash handling. The shared game work is:

- Set asset and save roots before loading settings.
- Set the logical game window size (the game coordinate system is 480x320).
- Initialise the renderer.
- Call `GamePreInitialise()`, `SetHardware()`, and `GameInitialise()`.
- `stepUpdate()` runs `SystemManager::Update` and `GameTaskUpdate`.
- `renderFrame()` computes the active viewport, begins the frame, runs `GameTaskDraw`, draws diagnostics, and swaps.

Xbox must not inherit Wii's EFB/XFB assumptions or its IR pointer overlay.

## 4. Wii rendering path

The Wii renderer is more than the GL shim described in older scaffolding notes:

- `Renderer.cpp` is the portable draw API. It builds 2D quad/triangle vertices and funnels 2D and 3D calls into the backend.
- `RendererGX.cpp` is the active native Wii renderer. It reads the separate projection and modelview matrices from `MatrixManager`, draws 2D and 3D geometry with GX, applies blend/depth/cull state, and resolves texture/buffer IDs through the Wii shim.
- `gl_funcsWii.cpp` supplies the GL-shaped asset/state seam used by shared code. It owns texture and buffer registries, uploads/tiles texture pixels for GX, tracks viewport/depth/blend state, and makes shader-object calls no-ops because GX has no GLSL pipeline.
- `DisplayManagerWii.cpp` copies GX EFB to the current XFB and waits for VIDEO vblank.

Xbox cannot reuse GX calls or the fixed-function TEV path. Xenos is programmable. The Xbox renderer needs precompiled XSDK vertex/pixel shaders, Xenos vertex layouts, GPU-resident vertex/index buffers, XSDK textures/samplers, depth/blend/raster state, and a resolve into the address supplied by `xsdk_present_begin()`. The shared `Renderer.h`/portable `Renderer.cpp` API can remain the contract.

## 5. Asset and texture path

Shared asset loading goes through `Mortar::File`, `FileManager`, `IFileSystem`, `TextureManager`, `TextureLoader` and `TextureFileFormat`. The Wii GL shim is responsible for turning uploaded pixels into GX textures. Xbox needs a separate Xenos texture registry that owns `XsdkTexture` and `XsdkSamplerState` objects and translates the supported source formats to XSDK formats. GPU texture memory must remain alive until all draws using it have passed a GPU fence.

## 6. Filesystem finding that needs correction

The Wii README describes a libfat-backed filesystem adapter, but the checked-in `FileSystemWii.cpp` and `FileSystemWii.h` still label themselves scaffolding and contain TODOs. `FileWii.h` explicitly says the per-file wrapper is not needed. The shared `Game.cpp::CreateFileSystems` currently constructs `FileSystem_Direct` with an empty root, rather than using `FileSystemWii::InitialiseWii`.

For Xbox, do not clone that ambiguity. Mount the XAM save package first, then register explicit direct filesystem roots for read-only game content and writable save content. XSDK supports `game:`, HDD, USB, memory-unit and DVD paths; its `XsdkSave` package wrapper currently mounts saves on HDD. Keep the save mount alive while any save file is open, and flush/unmount on orderly shutdown.

## 7. Audio finding

`SoundManagerWii.cpp` implements the Mortar `SoundManager` interface with ASND: SFX are cached PCM, while music is streamed through two buffers and refilled by `AudioStreamPump()` on the main thread. Its source notes that audio still needs validation on Dolphin/real hardware.

XSDK audio is a different contract: 6 channels at 48 kHz, 256 samples per frame, big-endian float samples, queued through `xsdk_audio_next_frame()` / `xsdk_audio_queue_frame()`. The Xbox backend must mix/resample the engine's 16 kHz mono PCM assets into this output format. The ASND voice allocation and callback model must not be copied verbatim.

## 8. Input mapping

Wii's `InputTranslatorWii` converts each Wiimote IR point plus A-button state into Mortar touch slots, preserving both menu clicks and the optional motion-mode blade behaviour. XSDK exposes `INPUT_STATE` with buttons, triggers and both analogue sticks. Xbox needs a deliberate controller-to-touch mapping and edge detection; it should feed the same `Mortar::Touch` ring rather than inventing a second gameplay-input system.

## 9. Endianness and data layout

Both Wii Broadway and Xbox 360 Xenon are big-endian PowerPC-family CPUs. Xbox 360's CPU is 64-bit-capable, but the XSDK title ABI is a 32-bit PowerPC environment. Do not change pointer width or assume little-endian host loads. Keep the engine's little-endian asset readers and pixel/index conversions explicit, and validate packed colours, 16-bit indices, texture uploads and shader constant layout on the Xbox toolchain.

## 10. Build-system work still required

The existing root CMake only knows the Wii-vs-default platform split. It selects `GameWii.cpp` vs `GameSDL.cpp`, `RendererGX.cpp` vs `RendererGL.cpp`, Wii-vs-SDL licensing, and Wii-vs-SDL audio. The existing `src/platform/xbox360/CMakeLists.txt` is currently a standalone XSDK smoke-test target and is not wired into the root Mortar/game target.

The next implementation sequence is:

1. Add a real `FRUIT_PLATFORM_XBOX360` root build mode using `xenosworks/xsdk/cmake/xsdk.cmake`; select Xbox game, renderer, audio, logger and platform sources without pulling SDL/libogc.
2. Replace the standalone C smoke test with a C++ XSDK entry point that owns GPU, display, gamepad and save lifecycles.
3. Implement `RendererXbox360.cpp` and its Xenos texture/buffer registry. Start with 2D UI, then 3D meshes and texture formats.
4. Implement controller-to-touch translation and the XSDK audio mixer.
5. Register `game:` assets and an HDD XAM save package through Mortar's filesystem interface.
6. Build with the XSDK toolchain, fix compile/link errors, then validate on Xenia and an RGH/JTAG console. A successful compile is not a substitute for hardware rendering/audio validation.

## Status

This commit is an audit and implementation map only. It does not claim the Xbox 360 game port is complete or tested.
