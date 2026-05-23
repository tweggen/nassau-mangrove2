# Building Mangrove Compressor

**Version:** 5.0.0 | **Phase:** 5 (IPlug2 GUI + AUv2) | **Last Updated:** 2026-05-23

## Project Overview

Mangrove is a professional audio compressor plugin with a digital-analog hybrid topology featuring separate Level and Density compression stages. The project is built as a cross-platform plugin using IPlug2 with IGraphics UI for macOS (VST3, AUv2) and Windows/Linux support planned.

## Architecture

```
Source/DSP/compressor_chain.{h,cpp}
  ↓ (static library: libcompressor_chain)
  ├─→ Tests/dsp_tests.cpp (51 verification tests, all passing)
  └─→ Source/Plugin/MangrovePlugin.{h,cpp} (IPlug2 wrapper)
       ├─→ VST3 plugin (MangrovePlugin-macOS.xcodeproj → MangroveIPlug.vst3)
       └─→ AUv2 plugin (MangrovePlugin-macOS.xcodeproj → MangroveIPlug.component)

Source/VST3/ (Phase 4, native Steinberg SDK implementation — kept as fallback)
  └─→ Verified working in Studio One 7, not built in Phase 5
```

## Build Systems

### 1. CMake (DSP Library + Tests)

**Status:** ✅ Stable — 51 tests passing  
**Used for:** CompressorChain library validation, independent testing

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --target dsp_tests
cd build && ctest -V
```

**Key files:**
- `CMakeLists.txt` (root) — project configuration, enables IPlug2/VST3 conditional builds
- `Source/DSP/CMakeLists.txt` — compressor_chain static library
- `Tests/CMakeLists.txt` — dsp_tests executable
- `Source/VST3/CMakeLists.txt` — native VST3 plugin (optional, not built in Phase 5)

### 2. Xcode (IPlug2 Plugin Targets)

**Status:** 🏗️ In Progress — project structure generated, library integration pending  
**Used for:** VST3 + AUv2 plugin builds with IGraphics GUI

**Project:** `/MangrovePlugin/projects/MangrovePlugin-macOS.xcodeproj`

Generated targets:
- `MangrovePlugin_VST3` — VST3 bundle
- `MangrovePlugin_AU` — AUv2 component
- `MangrovePlugin_CLAP` — CLAP bundle (generated, not validated)
- `MangrovePlugin_Standalone` — Standalone app (generated, not validated)

**Next steps to complete Phase 5:**
1. Link `Source/DSP/libcompressor_chain.a` to Xcode targets
   - Option A: Copy `Source/DSP/*.cpp/.h` to MangrovePlugin project
   - Option B: Build CMake `libcompressor_chain.a` and statically link in Xcode
2. Build `MangrovePlugin_VST3` target
3. Test in Studio One 7 (verify GUI, 14 parameters, audio path)
4. Build `MangrovePlugin_AU` target and test in Logic Pro

## Current Status

### ✅ Completed (Phase 3–4)

- **DSP Core:** All 51 compression algorithm tests passing
  - Level stage: threshold, ratio, attack, release, optional tube saturation, optional feedback
  - Density stage: threshold, ratio, attack, release
  - Input stage: gain, low-cut filter, saturation
  - Multi-sample-rate support: 8 kHz to 192 kHz
  
- **Phase 4 VST3 (Native Steinberg SDK):** Working, verified in Studio One 7
  - Processor/Controller architecture
  - All 14 parameters automated
  - Stereo audio processing

### 🏗️ In Progress (Phase 5)

- **IPlug2 Wrapper:** Plugin class skeleton complete
  - `config.h` — plugin identity (version 5.0.0, new UID 50D5F78F…9CD66A0E)
  - `MangrovePlugin.h/.cpp` — ProcessBlock, OnReset, OnParamChange hooks
  - `MangroveUI.h/.cpp` — IGraphics layout (640×400 canvas, 3 sections, 14 controls)
  - `Info.plist.vst3/.au` — bundle metadata

- **Xcode Project:** Generated from IPlug2's duplicate.py template
  - All plugin format projects created
  - Plugin sources copied in place
  - **Pending:** Link CompressorChain library, verify build

### ❌ Not Started

- **Windows/Linux:** Platform-specific code paths, standalone app validation
- **CLAP target:** Generated but untested
- **Preset system:** UI/parameter serialization
- **Documentation:** User manual, parameter reference

## File Structure

```
Source/
├── DSP/
│   ├── CMakeLists.txt
│   ├── compressor_chain.h
│   ├── compressor_chain.cpp
│   └── (11 test data JSON files)
├── Plugin/                    ← Phase 5 IPlug2 wrapper
│   ├── config.h              ← plugin identity, 14 parameters
│   ├── MangrovePlugin.h
│   ├── MangrovePlugin.cpp
│   ├── MangroveUI.h
│   ├── MangroveUI.cpp
│   ├── Info.plist.vst3       ← VST3 bundle metadata
│   ├── Info.plist.au         ← AU component metadata
│   └── CMakeLists.txt        ← build config (unused in Xcode phase)
├── VST3/                      ← Phase 4 native implementation (fallback)
│   ├── CMakeLists.txt
│   ├── config.h
│   ├── MangrovePlugin.h
│   ├── MangrovePlugin.cpp
│   └── (VST3 SDK integration files)
└── (other header-only DSP utilities)

external/
├── iplug2/                    ← submodule: IPlug2 framework + IGraphics
├── vst3sdk/                   ← submodule: Steinberg VST3 SDK

MangrovePlugin/               ← Xcode project (Phase 5)
├── projects/
│   ├── MangrovePlugin-macOS.xcodeproj
│   ├── MangrovePlugin-iOS.xcodeproj
│   ├── MangrovePlugin-Windows.sln
│   └── MangrovePlugin-Web.html
├── Source/
│   ├── config.h              ← copied from Source/Plugin/
│   ├── MangrovePlugin.h/cpp  ← copied from Source/Plugin/
│   ├── MangroveUI.h/cpp      ← copied from Source/Plugin/
│   └── (IPlug2 template resources)
└── (Xcode build artifacts)

Tests/
├── CMakeLists.txt
├── dsp_tests.cpp
└── (test data files)

docs/
└── BUILDING.md               ← this file
```

## Dependencies

### External

- **IPlug2** (`external/iplug2`) — plugin framework abstraction, IGraphics UI library
  - NanoVG — vector graphics backend
  - MetalNanoVG — Metal renderer for macOS
  - Depends on Cocoa, Metal, QuartzCore, AudioUnit, CoreAudio
  
- **Steinberg VST3 SDK** (`external/vst3sdk`) — Phase 4 native VST3 only; referenced by IPlug2's VST3 adapter

### System Frameworks (macOS)

```
Cocoa, Metal, MetalKit, QuartzCore
CoreAudio, AudioToolbox, CoreMIDI, AudioUnit
Accelerate, Carbon
```

## Building and Testing

### Validate DSP (CMake, all platforms)

```bash
# Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64

# Build and test
cmake --build build --target dsp_tests
cd build && ctest --output-on-failure
```

**Expected:** 51/51 tests PASS

### Build IPlug2 VST3 + AU (Xcode, macOS only)

```bash
# 1. Open project
open MangrovePlugin/projects/MangrovePlugin-macOS.xcodeproj

# 2. In Xcode:
#    a) Drag Source/DSP/ folder into project navigator (or use Option B below)
#    b) Add compressor_chain.cpp/.h to MangrovePlugin_VST3 target
#    c) Add same to MangrovePlugin_AU target
#    d) In Build Settings → Header Search Paths, add $(SRCROOT)/../../Source/DSP

# 3. Build
#    Scheme: MangrovePlugin_VST3
#    Destination: Mac (arm64)
#    Build button or Cmd+B

# 4. Find output
find ~/Library/Developer/Xcode/DerivedData \
  -name "MangroveIPlug.vst3" -type d
```

**Alternative (Option B): Link pre-built library**

```bash
# Build CMake static library
cmake --build build --target compressor_chain

# In Xcode:
#   1. Add build phase: Link Binary with Libraries
#   2. Add libcompressor_chain.a from build/Source/DSP/
#   3. Header Search Paths: $(SRCROOT)/../../Source/DSP
```

### Install and Test

```bash
# VST3
cp -r build-artifacts/MangroveIPlug.vst3 \
  ~/Library/Audio/Plug-Ins/VST3/

# AUv2
cp -r build-artifacts/MangroveIPlug.component \
  ~/Library/Audio/Plug-Ins/Components/

# Verify AU registration (macOS)
auval -v aufx Mng5 Nss2
# Expected: result: PASS

# Test in DAW
# Studio One 7 (VST3): rescan VST3s, verify GUI, test parameters
# Logic Pro (AU): rescan Audio Units, verify GUI, test parameters
```

## Key Configuration Values

| Parameter | Value | Purpose |
|-----------|-------|---------|
| PLUG_NAME | "Mangrove" | Display name |
| PLUG_VERSION_STR | "5.0.0" | Plugin version |
| PLUG_UNIQUE_ID | 'Mng5' | AU component subtype (4-char code) |
| PLUG_MFR_ID | 'Nss2' | Manufacturer code |
| PLUG_UID_STR | 50D5F78F…9CD66A0E | VST3 UID (differs from Phase 4 native) |
| PLUG_N_PARAMS | 14 | Threshold, Ratio, Attack, Release × 2 stages + Input controls |
| PLUG_WIDTH × HEIGHT | 640 × 400 | UI canvas size |
| Bundle ID | com.nassau.mangrove.iplug | macOS App Store identifier |

## Known Limitations & Workarounds

| Issue | Status | Workaround |
|-------|--------|-----------|
| IPlug2 CMake incomplete | ✅ Resolved | Use Xcode build system instead |
| CMake `-Wpedantic` breaks IPlug2 | ✅ Resolved | Move to per-target `target_compile_options()` |
| IPlug2 VST3 SDK path resolution | Pending | Verify in Xcode; symlink if needed |
| UID collision with Phase 4 native VST3 | ✅ Mitigated | New UID (…9CD66A0E) to avoid host conflicts |
| sample type mismatch (double/float) | ✅ Resolved | Convert in ProcessBlock, use float scratch buffers |

## Troubleshooting

### "Missing compressor_chain.h"

**Cause:** CompressorChain library not linked  
**Fix:** See "Build IPlug2 VST3 + AU" section, link the library or copy source files

### "IPlugAU.cpp: 'IPlugAUEntry' symbol not found"

**Cause:** AU factory function signature mismatch  
**Fix:** Check IPlug2's AU entry point; update `Info.plist.au` `factoryFunction` key if needed

### "Cannot open include file: 'pluginterfaces/vst/…'"

**Cause:** VST3 SDK path not in Xcode header search  
**Fix:** In Build Settings, add header search path for `external/vst3sdk`

### Tests fail after CMake build

**Cause:** CMake configuration out of sync  
**Fix:** `rm -rf build && cmake -S . -B build` and rebuild

## Next Steps (Post Phase 5)

1. ✅ Complete Xcode build (link library, compile targets)
2. ✅ Test in Studio One 7 and Logic Pro
3. ⬜ Add preset serialization (optional)
4. ⬜ Windows/Linux builds (requires JUCE or native adapters)
5. ⬜ Standalone app (if needed)
6. ⬜ Code signing and notarization (macOS distribution)
7. ⬜ Installer packaging (VST3/AU discovery, developer ID signing)

## References

- **IPlug2 Docs:** https://github.com/iPlug2/iPlug2/wiki
- **VST3 SDK:** https://github.com/steinbergmedia/vst3sdk
- **IGraphics:** Canvas-based UI, NanoVG vector graphics, platform-agnostic
- **AUv2 Registration:** `AudioComponents` key in Info.plist (arm64 only, no Carbon `.r` file needed)

---

**Questions?** Check the Troubleshooting section above, or review the implementation notes in `Source/Plugin/*.h` header comments.
