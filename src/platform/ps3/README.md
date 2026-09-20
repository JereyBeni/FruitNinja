# PS3 port (PSL1GHT / RSX)

Status: **scaffolding + DualShock 3 input**. Compiles/links not yet validated
on hardware/RPCS3. `-DFRUIT_PLATFORM_PS3=ON` + `cmake/ps3.toolchain.cmake`.
Every other build is unaffected when this option is OFF.

## Input (live in this scaffold)

| Control | Action |
|---|---|
| Left stick | Aim blade / pointer |
| Cross (X) or R2 | Cut / click |
| Motion mode ON (default) | Blade follows stick continuously; Cross is menu-click only (speed-gated slice) |
| Motion mode OFF | Hold Cross + move stick to cut |

Up to 4 pads. Same two-role channel model as Wii/SDL (`Mortar::Touch`).
Sixaxis sensors are enabled; tilt-aim can be wired later.

Implementation: `InputTranslatorPS3.{h,cpp}` via PSL1GHT `ioPad`.

## Goal

Faithful C++11 port of Fruit Ninja Bada 1.6.1 as PSL1GHT homebrew on real PS3
and RPCS3.

## Approach (planned)

| Concern | Location | State |
|---|---|---|
| Entry + loop | `mainPS3.cpp` | stub loop + pad poll |
| Input | `InputTranslatorPS3` | **real** DualShock 3 |
| Video / RSX | TBD | TODO |
| Audio | TBD | TODO |
| Filesystem | TBD | TODO |
| Endian | `FN_BIG_ENDIAN` | Cell PPU is big-endian |

## Build

```sh
export PS3DEV=/usr/local/ps3dev
export PSL1GHT=$PS3DEV/psl1ght
export PATH=$PS3DEV/bin:$PS3DEV/ppu/bin:$PATH

bash tools/ps3/build.sh
```

## Remaining

- [x] CMake option + toolchain file
- [x] Platform directory + DualShock 3 input
- [ ] Wire `FRUIT_PLATFORM_PS3` fully in root `CMakeLists.txt` (executable + defs)
- [ ] RSX / display path
- [ ] Audio backend
- [ ] Asset staging + `.self` / `.pkg`
- [ ] RPCS3 + real hardware validation
