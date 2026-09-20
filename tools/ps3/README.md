# tools/ps3 — PS3 build & package helpers

## Packages

| File | Script | What |
|---|---|---|
| `TestBuildFruitNinja1.0.pkg` | `package.sh` | Executable (EBOOT + SFO) |
| `FruitNinjaNECESARRYAssets.PKG` | `package-assets.sh` | Game data only |

## Build executable PKG

```bash
export PS3DEV=/usr/local/ps3dev
export PSL1GHT=$PS3DEV/psl1ght
export PATH=$PS3DEV/bin:$PS3DEV/ppu/bin:$PATH

bash tools/ps3/build.sh pkg
# -> build/ps3/TestBuildFruitNinja1.0.pkg
```

## Build assets PKG (Bada or Android dump)

The port expects **Bada-style** `Data/` (Tex1 textures, etc.).  
If you extracted an **Android** APK/OBB, put the files in a folder that matches that layout (or convert first with `tools/assets/`).

```bash
# Default: FruitNinjaBada/Data in the repo
bash tools/ps3/package-assets.sh

# Or Android / custom extract
bash tools/ps3/package-assets.sh /path/to/AndroidExtract/Data
# or
export FN_ANDROID_DATA=/path/to/AndroidExtract/Data
bash tools/ps3/package-assets.sh

# -> build/ps3/FruitNinjaNECESARRYAssets.PKG
```

## Install (CFW / RPCS3)

1. Install **FruitNinjaNECESARRYAssets.PKG** first (data).
2. Install **TestBuildFruitNinja1.0.pkg** (code).

Assets APPID: `FNASSET01` → data under that title’s `USRDIR/Data`.  
You can also copy `Data/` into the main app’s `USRDIR` after install.

## Legal

You need your **own** copy of the game data. This only packages files you provide; it does not ship Halfbrick assets.
