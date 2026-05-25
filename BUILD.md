# Mangrove Plugin Build Guide

## Quick Start (DSP-Only, Recommended)

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

**Result:** Plugin with host-generated UI, full DSP processing.  
**Time:** ~2 minutes  
**Complexity:** Low

## Building with Skia Graphics (Custom UI)

Custom UI requires Skia library. This is a complex, one-time setup.

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
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

CMake will automatically detect Skia libraries and enable graphics.

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

**Build:** 2 minutes

### With Custom UI (Developers)
- ✅ Custom-rendered MangroveUI layout
- ✅ Full DSP processing
- ❌ Requires Skia build setup (~1 hour)
- ❌ Larger plugin binary
- ❌ Not recommended for distribution (use DSP-only)

**Build:** 60+ minutes (first time only)

## File Output

Plugin locations after successful build:

**CMake (Recommended):**
```
build_xcode/Source/Plugin/Release/MangroveIPlug.vst3
```

**Install to macOS:**
```bash
cp -r build_xcode/Source/Plugin/Release/MangroveIPlug.vst3 \
  ~/Library/Audio/Plug-Ins/VST3/
```

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
