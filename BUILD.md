# Mangrove Plugin Build Guide (macOS)

> ## 🪟 Building on Windows? Use [`docs/BUILDING_WIN11.md`](docs/BUILDING_WIN11.md) instead.
>
> **This guide does not work on Windows.** The root CMake project described here is
> macOS-only and cannot produce a Windows plugin:
>
> - it applies `-Wall -Wextra` to every target, which MSVC rejects
>   (`error D8021: invalid numeric argument '/Wextra'`);
> - it does not define `NOMINMAX`, so IPlug2 headers fail with `error C2589`;
> - its graphics and entry-point sources are Objective-C++ (`IGraphicsMac.mm`,
>   `IGraphicsMac_view.mm`, `IGraphicsCoreText.mm`, `macmain.cpp`);
> - it looks for Skia only under `Dependencies/Build/mac/lib`, so on Windows it silently
>   configures with `IPLUG_EDITOR=0 NO_IGRAPHICS=1` — **no custom UI** — and then tells you
>   to run `build-skia-mac.sh`, which is a dead end there.
>
> On Windows the plugin is built from `MangrovePlugin\MangrovePlugin.sln`, needs no Skia,
> and gets its UI from the NanoVG/OpenGL 2 backend.

## Which build should I use?

This repo contains three plugin trees. For a command-line build on macOS, use the first one:

- **Root CMake project (recommended, this guide).** Driven by the top-level `CMakeLists.txt`,
  built from the repo root into `build/`. This is the blessed command-line path **on macOS**.
- **`MangroveIPlug/` and `MangrovePlugin/`** — standalone IPlug2 project scaffolds with their
  own build files (`.sln`, `.xcworkspace`, `build-mac/`). These are for the IDE / Reaper
  IPlug2 workflow, and `MangrovePlugin/` is also the Windows build. **You can ignore them for
  a macOS CLI build.**

Everything below refers to the root CMake project, run from the repository root on macOS.

## Quick Start (DSP-Only, Recommended)

### Prerequisites
- macOS with Xcode command-line tools: `xcode-select --install`
- CMake ≥ 3.15 (`cmake --version`)
- The `external/iplug2` and `external/vst3sdk` submodules present
  (`git submodule update --init --recursive` if `external/` is empty)
- No Skia setup needed for this path.

### Build
```bash
# from the repository root
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

**Result:** Plugin with host-generated UI, full DSP processing.
**Time:** ~2 minutes
**Complexity:** Low

This is the **DSP-only** variant: the host/DAW draws the parameter UI. CMake auto-detects
that Skia is absent and disables the custom UI for you (the plugin target compiles with
`NO_IGRAPHICS=1` in `Source/Plugin/CMakeLists.txt`).

## File Output

The Quick Start uses the default **Unix Makefiles** generator, so the build type is chosen
at configure time (`-DCMAKE_BUILD_TYPE`) and there is **no `Release/` subfolder**. Two
plugin bundles are produced under `build/`:

```
build/Source/Plugin/MangroveIPlug.vst3   # IPlug2-wrapped plugin (host UI)
build/Source/VST3/Mangrove.vst3          # raw VST3-SDK plugin
```

**Install to macOS:**
```bash
cp -r build/Source/Plugin/MangroveIPlug.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

> Optional: if you configure with the Xcode generator instead (`cmake .. -G Xcode`), the
> output moves under a per-config folder, e.g.
> `build/Source/Plugin/Release/MangroveIPlug.vst3`.

## Building with Skia Graphics (Custom UI) — Advanced / Optional

> **Not needed for the default build.** Skip this entire section unless you specifically want
> the custom-rendered `MangroveUI` instead of the host-generated UI.

Custom UI requires the Skia library. This is a complex, one-time setup. When Skia libraries
are present, CMake auto-detects them and enables graphics
(`IGRAPHICS_SKIA=1` / `IGRAPHICS_METAL=1`); otherwise it falls back to the DSP-only UI above.

### Prerequisites

- Xcode with command-line tools
- Python 3
- Git
- ~5-10GB free disk space
- 30-60 minutes

### Step 1: Build Skia (One-Time Setup)

```bash
cd external/iplug2/Dependencies

# Clone Skia source
mkdir -p Build/src
cd Build/src
git clone https://chromium.googlesource.com/skia.git

# Clone depot_tools (Chromium's build tools)
cd ../
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git

# Return to IGraphics folder
cd ../IGraphics

# Run the build script (30-60 minutes)
bash build-skia-mac.sh
```

This creates pre-built Skia libraries at:
```
external/iplug2/Dependencies/Build/mac/lib/
```

### Step 2: Build Plugin with Graphics

```bash
# from the repository root
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

CMake will automatically detect the Skia libraries and enable graphics.

### Troubleshooting Skia Build

If the Skia build fails:

1. **Missing `ninja`**
   ```bash
   brew install ninja
   ```

2. **Missing `gn`**
   This should be downloaded by depot_tools. Ensure depot_tools is in PATH:
   ```bash
   export PATH="$(pwd)/Build/tmp/depot_tools:$PATH"
   ```

3. **Python issues**
   Ensure Python 3 is available:
   ```bash
   python3 --version
   ```

## Plugin Variants

### DSP-Only (Recommended for most users)
- ✅ Loads instantly
- ✅ Full DSP processing
- ✅ Host-generated parameter UI
- ✅ No complex dependencies
- ❌ No custom visualization

**Build:** ~2 minutes

### With Custom UI (Developers)
- ✅ Custom-rendered MangroveUI layout
- ✅ Full DSP processing
- ❌ Requires Skia build setup (~1 hour)
- ❌ Larger plugin binary
- ❌ Not recommended for distribution (use DSP-only)

**Build:** 60+ minutes (first time only)

## Parameters (15 total)

**Input Stage:**
- Input Gain
- Input Lo Cut
- Input Saturate

**Level Compressor:**
- Threshold, Ratio, Attack, Release
- Lo Cut (toggle), Tube Gain (toggle), Feedback (toggle), Fast (toggle)

**Density Compressor:**
- Threshold, Ratio, Attack, Release

## Future Work

- Audio metering display
- Visual compression reduction indicators
- Custom preset management
- Preset browser integration
