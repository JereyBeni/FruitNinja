# PS3 port (PSL1GHT / RSX)

Status: **scaffolding only**. Compiles/links are not yet validated.
`-DFRUIT_PLATFORM_PS3=ON` + `cmake/ps3.toolchain.cmake`. Every other build
(host/web/webOS/Wii/asm-verify) is unaffected when this option is OFF.

## Goal

Faithful C++11 port of Fruit Ninja Bada 1.6.1 running as PSL1GHT homebrew on
real PS3 hardware and RPCS3. Same philosophy as the Wii port: keep the engine
mostly untouched and put platform differences behind thin seams.

## Approach (planned)

| Concern | Planned location | Notes |
|---|---|---|
| Entry + loop | `mainPS3.cpp` | PSL1GHT init, fixed-step loop |
| Video / RSX | TBD (`DisplayManagerPS3` or GL-on-RSX shim) | RSX via `rsx` / `gcm` |
| Input | `InputTranslatorPS3.{h,cpp}` | Pad (Sixaxis) + optional Move |
| Audio | `SoundManagerPS3` | libaudio / spu or simple mix |
| Filesystem | `FileSystemPS3` | cellFs / hostfs for RPCS3 |
| Endian | already handled via `FN_BIG_ENDIAN` | Cell PPU is big-endian |

## Build

```sh
# Env (once)
export PS3DEV=/usr/local/ps3dev          # or your prefix
export PSL1GHT=$PS3DEV/psl1ght
export PATH=$PS3DEV/bin:$PS3DEV/ppu/bin:$PATH

# Configure + build
cmake -B build/ps3 -G "Unix Makefiles" \
  -DCMAKE_TOOLCHAIN_FILE=cmake/ps3.toolchain.cmake \
  -DFRUIT_PLATFORM_PS3=ON
cmake --build build/ps3 -j$(nproc)
```

Or use the helper:

```sh
bash tools/ps3/build.sh
```

Output will eventually be a `.self` / `.pkg` suitable for RPCS3 or a CFW PS3.

## State / remaining

- [x] CMake option + toolchain file
- [x] Platform directory + stubs
- [ ] `mainPS3.cpp` real entry + fixed-step loop
- [ ] RSX / display path
- [ ] Pad input translator
- [ ] Audio backend
- [ ] Asset staging for PS3 (raw Tex1 like Wii, or compressed)
- [ ] `.self` / package generation
- [ ] RPCS3 + real hardware validation

See also the Wii README (`src/platform/wii/README.md`) for patterns we will
reuse (endian gates, block-preload, GL shim idea, etc.).
