# Building MangroveIPlug Xcode Project

## Overview

MangroveIPlug can be built using Xcode (macOS) with the IPlug2 framework. The project produces VST3, VST2, AU, AAX, and standalone app formats.

## Prerequisites

- Xcode 13.0+
- CMake 3.15+ (for helper builds)
- IPlug2 dependencies (included as git submodule in `external/iplug2/`)

## Building with Xcode

### Release Build

```bash
xcodebuild build -project projects/MangroveIPlug-macOS.xcodeproj -scheme "macOS-VST3" -configuration Release
```

Built plugin location:
```
~/Library/Audio/Plug-Ins/VST3/MangroveIPlug.vst3
```

### Debug Build

```bash
xcodebuild build -project projects/MangroveIPlug-macOS.xcodeproj -scheme "macOS-VST3" -configuration Debug
```

### All Formats

```bash
xcodebuild build -project projects/MangroveIPlug-macOS.xcodeproj -scheme "All macOS" -configuration Release
```

## Build Cache Issues

**Important:** After modifying `scripts/prepare_resources-mac.py` (which generates Info.plist files), you must clear Xcode's derived data cache:

```bash
rm -rf ~/Library/Developer/Xcode/DerivedData/*
xcodebuild clean -project projects/MangroveIPlug-macOS.xcodeproj -scheme "macOS-VST3" -configuration Release
```

Without this, Xcode may use cached plist data instead of the newly generated files, causing bundle identifier mismatches.

## Known Issues & Fixes

### Issue: CFBundleExecutable Mismatch

**Problem:** Plugin bundle fails to load because `CFBundleExecutable` in Info.plist doesn't match the actual binary name.

**Root Cause:** The `prepare_resources-mac.py` script was setting `CFBundleExecutable` to `config['BUNDLE_NAME']` ("Mangrove") instead of the actual binary name ("MangroveIPlug").

**Fix Applied:**
- Modified `scripts/prepare_resources-mac.py` line 99:
  ```python
  vst3['CFBundleExecutable'] = 'MangroveIPlug'  # Use actual binary name, not BUNDLE_NAME
  ```
- Applied same fix for VST2, AU, AAX, and macOS App targets

### Issue: CFBundleIdentifier Mismatch

**Problem:** Xcode warns that user-supplied CFBundleIdentifier doesn't match PRODUCT_BUNDLE_IDENTIFIER build setting.

**Root Cause:** The `prepare_resources-mac.py` script was constructing identifiers using `config['BUNDLE_NAME']` ("Mangrove"), but Xcode build settings use "MangroveIPlug" in the identifier.

**Fix Applied:**
- Modified all CFBundleIdentifier generation in `prepare_resources-mac.py`:
  ```python
  # VST3
  vst3['CFBundleIdentifier'] = "com.Nassau.vst3.MangroveIPlug"
  # VST2
  vst2['CFBundleIdentifier'] = "com.Nassau.vst.MangroveIPlug"
  # AU
  auv2['CFBundleIdentifier'] = "com.Nassau.audiounit.MangroveIPlug"
  # AUv3
  auv3['CFBundleIdentifier'] = "com.Nassau.app.MangroveIPlug.AUv3"
  # AAX
  aax['CFBundleIdentifier'] = "com.Nassau.aax.MangroveIPlug"
  # macOS App
  macOSapp['CFBundleIdentifier'] = "com.Nassau.app.MangroveIPlug"
  ```
- Also fixed AudioComponentBundle reference for AUv3 Framework

## Plugin Discovery in DAWs

### Studio One

After building, clear Studio One's VST3 cache before testing:

```bash
rm -rf ~/Library/Preferences/PreSonus\ Studio\ One/Cache/VST3
```

Then open Studio One and let it rescan for plugins. The plugin should now appear in the instrument list.

## UI Status

- ✅ Plugin builds successfully
- ✅ Plugin is discoverable in DAWs (VST3 numClasses > 0)
- ✅ UI window displays and shows controls layout
- ⏳ Control bindings to DSP parameters (in progress)
- ⏳ Real-time meter display (planned)

## Architecture Notes

### Universal Binary

The built binary is a universal Mach-O binary supporting both:
- x86_64 (Intel)
- arm64 (Apple Silicon / M1-M4)

Verify with:
```bash
file ~/Library/Audio/Plug-Ins/VST3/MangroveIPlug.vst3/Contents/MacOS/MangroveIPlug
```

### Linking

VST3 SDK libraries are statically linked during the build. Required libraries:
- `libsdk.a`
- `libpluginterfaces.a`
- `libbase.a`
- `libsdk_common.a`

These are configured in the Xcode project build settings under `OTHER_LDFLAGS`.

## Debugging

### Check Bundle Contents

```bash
# View Info.plist
plutil -p ~/Library/Audio/Plug-Ins/VST3/MangroveIPlug.vst3/Contents/Info.plist

# List all files in bundle
ls -la ~/Library/Audio/Plug-Ins/VST3/MangroveIPlug.vst3/Contents/
```

### View Build Settings

```bash
# List all schemes
xcodebuild -project projects/MangroveIPlug-macOS.xcodeproj -list

# Check a specific build setting
xcodebuild -project projects/MangroveIPlug-macOS.xcodeproj -scheme "macOS-VST3" -configuration Release -showBuildSettings | grep BUNDLE
```

### CMake Alternative

For debugging DSP without GUI overhead, the project also supports CMake builds:

```bash
cd ../..  # Go to nassau-mangrove2 root
mkdir build_phase5
cd build_phase5
cmake ..
cmake --build . --config Release
```

This produces a command-line test binary without the GUI framework dependencies.
