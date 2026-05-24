# Phase 4: VST 3 Wrapper (IPlug2)

**Objective:** Wrap `CompressorChain` DSP library in IPlug2 VST 3 framework for cross-platform distribution.

**Duration:** 3–4 weeks  
**Dependencies:** Phase 1–3 (Complete CompressorChain DSP)  
**Produces:** Loadable `.vst3` plugin (Windows/macOS/Linux)  
**Tests Required:** Plugin load, parameter automation, audio passthrough  

---

## Overview

This phase uses **IPlug2** (open-source plugin framework) to wrap the `CompressorChain` DSP into a VST 3 plugin. IPlug2 handles:

- ✅ VST 3 SDK boilerplate
- ✅ Parameter mapping & automation
- ✅ Audio I/O & sample-accurate processing
- ✅ Cross-platform build (CMake)
- ✅ GUI framework (IGraphics, covered in Phase 5)

### Why IPlug2?

- Eliminates 2000+ lines of VST 3 boilerplate
- One codebase → multiple formats (VST 3, AU, LADSPA)
- Active community, well-documented
- Free and open-source (AGPL)

### What We Build

```
MangroveVST3Plugin
├── Audio Processing
│   └── CompressorChain (from Phase 1–3)
├── Parameter Mapping
│   └── 14 JUCE parameters → 14 VST 3 parameters
├── State Serialization
│   └── Binary format for save/load
└── GUI Placeholder
    └── Minimal implementation (full GUI in Phase 5)
```

---

## Acceptance Criteria

Phase 4 is **complete** when:

- [ ] IPlug2 project created and builds without warnings
- [ ] Plugin loads in DAW without crashes
- [ ] All 14 parameters appear in DAW parameter list
- [ ] Parameters are automatable (respond to host automation)
- [ ] Audio processes correctly (input → DSP → output)
- [ ] State serialization works (save preset → reload → values match)
- [ ] Plugin works on Windows, macOS, and Linux
- [ ] CPU usage reasonable (<10% per instance on test signal)
- [ ] No latency introduced (0 samples)
- [ ] Parameter names and ranges correct
- [ ] Compiles without warnings on all platforms

---

## Detailed Tasks

### Task 4.1: Download & Set Up IPlug2

**What:** Clone and configure IPlug2 in project  
**Duration:** 1–2 hours  
**Files to Create/Modify:** `CMakeLists.txt`, `.gitmodules`

**Steps:**

1. **Add IPlug2 as Git submodule:**
   ```bash
   cd /path/to/mangrove-refactored
   git submodule add https://github.com/iPlug2/iPlug2.git external/iplug2
   git submodule update --init --recursive
   ```

2. **Verify IPlug2 structure:**
   ```bash
   ls -la external/iplug2/
   # Should contain: IPlug, IGraphics, Dependencies, etc.
   ```

3. **Update root `CMakeLists.txt` to include IPlug2:**

   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(MangrovePlugin VERSION 2.0.0 LANGUAGES CXX C)
   
   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   
   # ===== IPlug2 Configuration =====
   set(IPLUG_PLUGINFORMATS "VST3" CACHE STRING "Plugin formats to build")
   set(IPLUG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/iplug2")
   
   # Include IPlug2 CMake utilities
   list(APPEND CMAKE_MODULE_PATH "${IPLUG_ROOT}/cmake")
   include(IPlugPlatforms)
   include(IPlugTargets)
   
   # ===== Mangrove DSP Library =====
   add_subdirectory(Source/DSP)
   
   # ===== VST3 Plugin =====
   add_subdirectory(Source/VST3)
   
   # ===== Tests =====
   if(BUILD_TESTS)
     enable_testing()
     add_subdirectory(Tests)
   endif()
   ```

4. **Create `Source/VST3/CMakeLists.txt`:**

   ```cmake
   # Mangrove VST3 Plugin
   iplug_add_vst3(MangroveVST3
     SOURCES
       MangroveVST3.cpp
     
     INCLUDE_DIRS
       ${CMAKE_CURRENT_SOURCE_DIR}
       ${CMAKE_SOURCE_DIR}/Source/DSP
     
     LINK_LIBS
       MangroveCompressorDSP
     
     BUNDLE_ID "com.nassau.mangrove.vst3"
     BUNDLE_NAME "Mangrove"
     COMPANY_NAME "Nassau"
   )
   ```

5. **Build and test:**
   ```bash
   mkdir build && cd build
   cmake .. -DIPLUG_PLUGINFORMATS="VST3"
   cmake --build . --config Release
   
   # Verify plugin was created
   find . -name "*.vst3"
   ```

**Acceptance:**
- [ ] IPlug2 submodule added
- [ ] CMake files created
- [ ] Project builds (even if plugin is minimal)
- [ ] VST3 bundle created (`Mangrove.vst3/`)

---

### Task 4.2: Create IPlug2 Plugin Class

**What:** Implement main plugin class inheriting from `IPlug`  
**Duration:** 3–4 hours  
**Files to Create:**
- `Source/VST3/MangroveVST3.h`
- `Source/VST3/MangroveVST3.cpp`

**Header File:**

```cpp
// MangroveVST3.h
#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "compressor_chain.h"

const int kNumParams = 14;

enum EParams {
  kInputGain = 0,
  kInputLoCut,
  kInputSaturate,
  
  kLevelThreshold,
  kLevelRatio,
  kLevelAttack,
  kLevelRelease,
  kLevelLoCut,
  kLevelTubeGain,
  kLevelFeedback,
  
  kDensityThreshold,
  kDensityRatio,
  kDensityAttack,
  kDensityRelease,
};

class MangroveVST3 : public Plugin {
public:
  MangroveVST3(const InstanceInfo& info);
  
  // Plugin interface
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnParamChange(int paramIdx) override;
  void OnReset() override;
  
private:
  CompressorChain mDSP;
  
  // Prevent copy
  MangroveVST3(const MangroveVST3&) = delete;
  MangroveVST3& operator=(const MangroveVST3&) = delete;
};
```

**Implementation File:**

```cpp
// MangroveVST3.cpp
#include "MangroveVST3.h"

MangroveVST3::MangroveVST3(const InstanceInfo& info)
  : Plugin(info, MakeConfig(kNumParams, 1)) {
  
  // ===== Parameter Definitions =====
  
  // Input Stage
  GetParam(kInputGain)->InitDouble(
    "Input Gain", 0.0, -24.0, 24.0, 0.1, "dB");
  
  GetParam(kInputLoCut)->InitDouble(
    "Input LoC ut", 80.0, 20.0, 300.0, 1.0, "Hz");
  
  GetParam(kInputSaturate)->InitDouble(
    "Input Saturate", 0.0, 0.0, 5.0, 0.1);
  
  // Level Compressor
  GetParam(kLevelThreshold)->InitDouble(
    "Level Threshold", -10.0, -60.0, 0.0, 0.5, "dB");
  
  GetParam(kLevelRatio)->InitDouble(
    "Level Ratio", 2.5, 1.0, 10.0, 0.1);
  
  GetParam(kLevelAttack)->InitDouble(
    "Level Attack", 10.0, 0.0, 100.0, 1.0, "ms");
  
  GetParam(kLevelRelease)->InitDouble(
    "Level Release", 300.0, 10.0, 500.0, 10.0, "ms");
  
  GetParam(kLevelLoCut)->InitBool(
    "Level LoC ut", false);
  
  GetParam(kLevelTubeGain)->InitBool(
    "Level TubeGain", false);
  
  GetParam(kLevelFeedback)->InitBool(
    "Level Feedback", true);
  
  // Density Compressor
  GetParam(kDensityThreshold)->InitDouble(
    "Density Threshold", -10.0, -36.0, 0.0, 0.5, "dB");
  
  GetParam(kDensityRatio)->InitDouble(
    "Density Ratio", 1.0, 1.0, 10.0, 0.1);
  
  GetParam(kDensityAttack)->InitDouble(
    "Density Attack", 10.0, 0.001, 100.0, 0.5, "ms");
  
  GetParam(kDensityRelease)->InitDouble(
    "Density Release", 300.0, 10.0, 2000.0, 10.0, "ms");
}

void MangroveVST3::OnReset() {
  // Initialize DSP when sample rate is known
  double sampleRate = GetSampleRate();
  if (sampleRate > 8000.0 && sampleRate < 192000.0) {
    mDSP.init((float)sampleRate);
  }
}

void MangroveVST3::OnParamChange(int paramIdx) {
  // Handle parameter changes from host/UI
  double value = GetParam(paramIdx)->Value();
  
  switch (paramIdx) {
    case kInputGain:
      mDSP.setInputGain((float)value);
      break;
    case kInputLoCut:
      mDSP.setInputLoCut((float)value);
      break;
    case kInputSaturate:
      mDSP.setInputSaturate((float)value);
      break;
    
    case kLevelThreshold:
      mDSP.setLevelThreshold((float)value);
      break;
    case kLevelRatio:
      mDSP.setLevelRatio((float)value);
      break;
    case kLevelAttack:
      mDSP.setLevelAttack((float)value);
      break;
    case kLevelRelease:
      mDSP.setLevelRelease((float)value);
      break;
    case kLevelLoCut:
      mDSP.setLevelLoCut(value >= 0.5);
      break;
    case kLevelTubeGain:
      mDSP.setLevelTubeGain(value >= 0.5);
      break;
    case kLevelFeedback:
      mDSP.setLevelFeedback(value >= 0.5);
      break;
    
    case kDensityThreshold:
      mDSP.setDensityThreshold((float)value);
      break;
    case kDensityRatio:
      mDSP.setDensityRatio((float)value);
      break;
    case kDensityAttack:
      mDSP.setDensityAttack((float)value);
      break;
    case kDensityRelease:
      mDSP.setDensityRelease((float)value);
      break;
  }
}

void MangroveVST3::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
  // Stereo in/out processing
  const sample* inL = inputs[0];
  const sample* inR = inputs[1];
  
  sample* outL = outputs[0];
  sample* outR = outputs[1];
  
  // Process through DSP
  mDSP.process(inL, inR, outL, outR, nFrames);
}

// Entry point for VST3 SDK
using PLUG_CLASS_NAME = MangroveVST3;

#include "IPlug_include_in_plug_src.cpp"
```

**Acceptance:**
- [ ] Header compiles without errors
- [ ] Implementation compiles without warnings
- [ ] All 14 parameters defined
- [ ] Process block method signature correct
- [ ] DSP integration compiles

---

### Task 4.3: Parameter Serialization

**What:** Implement save/load of plugin state  
**Duration:** 2–3 hours  
**Files to Modify:** `Source/VST3/MangroveVST3.h`, `.cpp`

**Overview:**

IPlug2 handles serialization automatically, but we need to ensure parameters round-trip correctly.

**In MangroveVST3.h**, add state methods:

```cpp
class MangroveVST3 : public Plugin {
public:
  // ... existing methods ...
  
  void SaveState(Serialize::IByteChunk& chunk) override;
  int LoadState(Serialize::IByteChunk& chunk) override;
};
```

**In MangroveVST3.cpp**, implement:

```cpp
void MangroveVST3::SaveState(Serialize::IByteChunk& chunk) {
  // Save all parameters
  for (int i = 0; i < kNumParams; ++i) {
    double value = GetParam(i)->Value();
    // Let IPlug2 handle the serialization
    chunk.Put(&value);
  }
}

int MangroveVST3::LoadState(Serialize::IByteChunk& chunk) {
  // Load and restore all parameters
  int pos = 0;
  
  for (int i = 0; i < kNumParams; ++i) {
    double value = 0.0;
    
    // Note: IPlug2 usually handles this automatically
    // This is for custom parameter handling if needed
    pos += sizeof(value);
  }
  
  return pos;
}
```

**Note:** IPlug2 provides automatic parameter serialization. These methods can be minimal or omitted if IPlug2 handles it.

**Test round-trip:**

```cpp
TEST_CASE("VST3 Plugin: state save/load") {
  MangroveVST3 plugin(...);
  
  // Set parameters
  plugin.GetParam(kInputGain)->Set(6.0);
  plugin.GetParam(kLevelThreshold)->Set(-15.0);
  
  // Save state
  Serialize::IByteChunk chunk;
  plugin.SaveState(chunk);
  
  // Reset parameters
  plugin.GetParam(kInputGain)->Set(0.0);
  plugin.GetParam(kLevelThreshold)->Set(-10.0);
  
  // Load state
  chunk.SetReadPos(0);
  plugin.LoadState(chunk);
  
  // Verify values restored
  REQUIRE(plugin.GetParam(kInputGain)->Value() == Catch::Approx(6.0));
  REQUIRE(plugin.GetParam(kLevelThreshold)->Value() == Catch::Approx(-15.0));
}
```

**Acceptance:**
- [ ] Serialization methods implemented
- [ ] Round-trip test passes
- [ ] Preset file format (binary) works
- [ ] No data loss

---

### Task 4.4: Audio I/O Validation

**What:** Test plugin audio processing in DAW  
**Duration:** 2–3 hours  
**Tools Needed:** Reaper, Studio One, or other VST 3 host

**Procedure:**

1. **Build plugin:**
   ```bash
   cmake --build build --config Release
   ```

2. **Copy to VST3 folder:**
   ```bash
   # Windows
   copy build\Mangrove.vst3 "C:\Program Files\Common Files\VST3\"
   
   # macOS
   cp -r build/Mangrove.vst3 "~/Library/Audio/Plug-Ins/VST3/"
   
   # Linux
   cp -r build/Mangrove.vst3 ~/.vst3/
   ```

3. **Launch DAW and scan plugins:**
   - Open Reaper (or Studio One)
   - Scan VST3 folder
   - Verify "Mangrove" appears in plugin list

4. **Load plugin on audio track:**
   - Create stereo audio track
   - Load Mangrove plugin
   - Verify no crash on load

5. **Test parameter automation:**
   - Move slider in plugin GUI
   - Verify DAW parameter list reflects changes
   - Create automation track for each parameter
   - Verify automation plays back correctly

6. **Test audio I/O:**
   - Generate test tone (1 kHz sine, -20 dB)
   - Route through plugin
   - Verify output matches input (with plugin at 0 dB gain)
   - Adjust "Input Gain" to +6 dB
   - Verify output is 2× amplitude
   - Use Level Compressor to reduce peaks
   - Verify compression works as expected

7. **CPU profiling:**
   - Monitor CPU usage while playing
   - Target: <10% per instance on typical system
   - Use DAW's performance monitor

8. **Take screenshots:**
   - Plugin loaded (empty GUI from Phase 4)
   - Parameter list visible
   - Automation working

**Acceptance:**
- [ ] Plugin loads without crash
- [ ] Parameters appear in DAW
- [ ] Audio processing works (input passes through)
- [ ] Parameter automation functional
- [ ] CPU usage reasonable
- [ ] Works on Windows, macOS, Linux (if possible)

---

### Task 4.5: Cross-Platform Build Verification

**What:** Build and test on Windows, macOS, and Linux  
**Duration:** 3–4 hours (can be parallelized)

**Windows:**
```bash
# Visual Studio 2019+
cmake -G "Visual Studio 16 2019" -A x64 build_win
cmake --build build_win --config Release

# Verify
dir build_win\Release\Mangrove.vst3
```

**macOS:**
```bash
# Xcode
cmake -G Xcode build_mac
cmake --build build_mac --config Release

# Verify
ls -la build_mac/Release/Mangrove.vst3
```

**Linux:**
```bash
# GCC/Clang
cmake -G "Unix Makefiles" build_linux
cmake --build build_linux --config Release

# Verify
find build_linux -name "Mangrove.vst3"
```

**Acceptance:**
- [ ] Builds on Windows without errors
- [ ] Builds on macOS without errors
- [ ] Builds on Linux without errors
- [ ] VST3 bundle created on each platform
- [ ] Plugin loads in DAW on each platform

---

### Task 4.6: Fix Compilation Warnings

**What:** Address all compiler warnings  
**Duration:** 2–3 hours

**Common Warnings to Fix:**

1. **Unused variables:**
   ```cpp
   // Wrong
   int foo = 5;  // Unused
   
   // Right
   (void)foo;  // Explicitly unused
   // OR
   [[maybe_unused]] int foo = 5;
   ```

2. **Type mismatches:**
   ```cpp
   // Wrong
   double x = GetParam(i)->Value();
   mDSP.setInputGain(x);  // Expects float
   
   // Right
   mDSP.setInputGain((float)GetParam(i)->Value());
   ```

3. **Missing includes:**
   ```cpp
   // Wrong
   // Missing #include <cmath>
   float val = pow(2.0f, 3.0f);
   
   // Right
   #include <cmath>
   float val = pow(2.0f, 3.0f);
   ```

**Check warnings:**
```bash
cmake --build build --config Release 2>&1 | grep warning
```

**Acceptance:**
- [ ] Zero compiler warnings on all platforms
- [ ] Consistent across GCC, Clang, MSVC

---

### Task 4.7: Write Integration Tests

**What:** Test plugin in simulated DAW environment  
**Duration:** 3–4 hours  
**File:** `Tests/vst3_tests.cpp`

**Test Cases:**

```cpp
#include "catch.hpp"
#include "MangroveVST3.h"
#include <vector>

TEST_CASE("MangroveVST3: parameter count") {
  MangroveVST3Plugin plugin;
  REQUIRE(plugin.GetNumParams() == 14);
}

TEST_CASE("MangroveVST3: parameter name and range") {
  MangroveVST3Plugin plugin;
  
  // Check first parameter
  auto* param = plugin.GetParam(0); // kInputGain
  REQUIRE(param->GetName() == "Input Gain");
  REQUIRE(param->GetRange().min == -24.0);
  REQUIRE(param->GetRange().max == 24.0);
}

TEST_CASE("MangroveVST3: audio processing") {
  MangroveVST3Plugin plugin;
  plugin.OnReset();  // Set sample rate, etc.
  
  // Create test buffers
  std::vector<float> inL(512, 0.0f);
  std::vector<float> inR(512, 0.0f);
  std::vector<float> outL(512, 0.0f);
  std::vector<float> outR(512, 0.0f);
  
  // Fill input with test signal
  for (int i = 0; i < 512; ++i) {
    inL[i] = sin(2.0f * 3.14159f * i / 512.0f);
    inR[i] = sin(2.0f * 3.14159f * i / 512.0f);
  }
  
  // Process
  float* inputs[2] = {inL.data(), inR.data()};
  float* outputs[2] = {outL.data(), outR.data()};
  plugin.ProcessBlock(inputs, outputs, 512);
  
  // Verify output is not all zeros
  float rms = 0.0f;
  for (int i = 0; i < 512; ++i) {
    rms += outL[i] * outL[i];
  }
  rms = sqrt(rms / 512.0f);
  
  REQUIRE(rms > 0.001f);  // Not silence
}

TEST_CASE("MangroveVST3: parameter automation") {
  MangroveVST3Plugin plugin;
  plugin.OnReset();
  
  // Change parameter
  plugin.GetParam(kInputGain)->Set(6.0);
  plugin.OnParamChange(kInputGain);
  
  // Process and verify amplitude doubled
  std::vector<float> inL(512, 0.5f);
  std::vector<float> inR(512, 0.5f);
  std::vector<float> outL(512);
  std::vector<float> outR(512);
  
  float* inputs[2] = {inL.data(), inR.data()};
  float* outputs[2] = {outL.data(), outR.data()};
  plugin.ProcessBlock(inputs, outputs, 512);
  
  // With +6 dB gain, amplitude should be ~2x
  float ratio = outL[0] / inL[0];
  REQUIRE(ratio == Catch::Approx(2.0f).margin(0.1f));
}

TEST_CASE("MangroveVST3: state round-trip") {
  MangroveVST3Plugin plugin1, plugin2;
  
  // Set parameters on first instance
  plugin1.GetParam(kInputGain)->Set(3.0);
  plugin1.GetParam(kLevelThreshold)->Set(-15.0);
  
  // Save state
  Serialize::IByteChunk chunk;
  plugin1.SaveState(chunk);
  
  // Load into second instance
  chunk.SetReadPos(0);
  plugin2.LoadState(chunk);
  
  // Verify parameters match
  REQUIRE(plugin2.GetParam(kInputGain)->Value() == Catch::Approx(3.0));
  REQUIRE(plugin2.GetParam(kLevelThreshold)->Value() == Catch::Approx(-15.0));
}
```

**Acceptance:**
- [ ] 4+ integration tests pass
- [ ] All parameter types covered
- [ ] Audio I/O correct
- [ ] State serialization verified

---

### Task 4.8: Code Review & Documentation

**What:** Final review and documentation  
**Duration:** 2–3 hours

**Checklist:**

- [ ] All code follows naming conventions
- [ ] Function headers documented
- [ ] No compiler warnings
- [ ] Integration tests pass
- [ ] Parameter ranges match original
- [ ] Audio output bit-exact (Phase 1 reference)
- [ ] Serialization round-trips correctly
- [ ] Works on Windows, macOS, Linux

**Documentation to Add:**

**Create `Source/VST3/README.md`:**

```markdown
# Mangrove VST3 Plugin

Cross-platform VST3 wrapper using IPlug2.

## Building

```bash
mkdir build && cd build
cmake .. -DIPLUG_PLUGINFORMATS="VST3"
cmake --build . --config Release
```

## Parameters

14 total parameters (see MangroveVST3.h for details):

### Input Stage (3)
- Input Gain: -24 to +24 dB
- Input LoC ut: 20 to 300 Hz
- Input Saturate: 0 to 5

### Level Compressor (7)
- Level Threshold: -60 to 0 dB
- Level Ratio: 1 to 10 (≥9.999 = Vari-Mu)
- Level Attack: 0 to 100 ms
- Level Release: 10 to 500 ms
- Level LoC ut: Boolean
- Level Tube Gain: Boolean
- Level Feedback: Boolean (true = feedback, false = feedforward)

### Density Compressor (4)
- Density Threshold: -36 to 0 dB
- Density Ratio: 1 to 10 (≥9.999 = Limiter)
- Density Attack: 0.001 to 100 ms
- Density Release: 10 to 2000 ms

## Testing

Run integration tests:
```bash
ctest
```

## GUI

Minimal placeholder in this phase. Full GUI (sliders, meters) in Phase 5.
```

**Acceptance:**
- [ ] All documentation complete
- [ ] README covers parameters
- [ ] Build instructions clear
- [ ] Examples provided

---

## Deliverables Checklist

- [ ] `Source/VST3/MangroveVST3.h` (plugin class)
- [ ] `Source/VST3/MangroveVST3.cpp` (implementation)
- [ ] `Source/VST3/CMakeLists.txt` (build config)
- [ ] Updated root `CMakeLists.txt`
- [ ] `Tests/vst3_tests.cpp` (integration tests)
- [ ] `Source/VST3/README.md` (documentation)
- [ ] Build artifacts: `Mangrove.vst3/` (all platforms)
- [ ] Git commit with Phase 4 summary

---

## Success Criteria

Phase 4 is **complete** when:

✅ IPlug2 integrated into project  
✅ All 14 parameters defined  
✅ Plugin loads in DAW without crash  
✅ Parameters appear in DAW parameter list  
✅ Audio processes correctly  
✅ Parameter automation works  
✅ State save/load functions  
✅ Builds on Windows, macOS, Linux  
✅ CPU usage <10% per instance  
✅ Audio output matches Phase 1 reference  
✅ Zero compiler warnings  
✅ Integration tests pass  
✅ Documentation complete  

---

## Next Steps (When Complete)

1. Merge Phase 4 branch to `main`
2. Update [IMPLEMENTATION_PLAN.md](./IMPLEMENTATION_PLAN.md): Phase 4 → ✅ Completed
3. Begin [PHASE_5_GUI_IMPLEMENTATION.md](./PHASE_5_GUI_IMPLEMENTATION.md) (parameter UI)

---

## Timeline

- Days 1–2: Task 4.1–4.2 (IPlug2 setup + plugin class)
- Days 3: Task 4.3–4.4 (serialization + audio I/O)
- Days 4–5: Task 4.5–4.6 (cross-platform + warnings)
- Days 6–7: Task 4.7–4.8 (tests + review)

**Total: 3–4 weeks**

---

## Common Pitfalls

| Issue | Cause | Solution |
|-------|-------|----------|
| IPlug2 not found | Submodule not initialized | Run `git submodule update --init --recursive` |
| Plugin doesn't load | Wrong VST3 path | Check plugin folder location in DAW preferences |
| No parameters visible | Parameter registration missing | Verify all 14 GetParam calls in constructor |
| Audio doesn't process | ProcessBlock not called | Check plugin actually instantiated in DAW |
| Serialization fails | Chunk size mismatch | Ensure SaveState/LoadState sizes match |
| Crashes on parameter change | Thread safety issue | Use atomic access if parameter changed mid-block |
| Build fails on Linux | Missing dependencies | Install libfreetype6-dev, libfontconfig1-dev, etc. |

---

## Resources

- **IPlug2 Docs:** https://github.com/iPlug2/iPlug2/wiki
- **VST 3 SDK:** https://github.com/steinbergmedia/vst3sdk
- **Building IPlug2:** https://github.com/iPlug2/iPlug2/blob/master/docs/BUILDING.md
