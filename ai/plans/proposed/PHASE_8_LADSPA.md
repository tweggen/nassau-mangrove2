# Phase 8: LADSPA (Linux)

**Objective:** Create LADSPA plugin wrapper for Linux DAWs (Ardour, JACK, Mixxx).

**Duration:** 1–2 weeks  
**Dependencies:** Phase 1–3 (CompressorChain DSP)  
**Produces:** `libmangrove.so` (shared library)  
**Platforms:** Linux (x86_64, ARM64)  

---

## Overview

LADSPA (Linux Audio Developer and Plugin API) is a simple, lightweight plugin standard for Linux.

**Key Characteristics:**
- No GUI (host-drawn sliders)
- Simple C API
- Process block-based
- No real-time parameter smoothing
- No preset save/load (host-dependent)

---

## Implementation

### Task 8.1: Create LADSPA Descriptor

**File:** `Source/LADSPA/MangroveLADSPA.h`

```cpp
#pragma once

#include <ladspa.h>
#include "compressor_chain.h"

// Port indices
enum MangrovePorts {
  // Audio I/O (4 ports)
  kAudioInLeft = 0,
  kAudioInRight = 1,
  kAudioOutLeft = 2,
  kAudioOutRight = 3,
  
  // Parameters (14 ports)
  kInputGain = 4,
  kInputLoCut = 5,
  kInputSaturate = 6,
  kLevelThreshold = 7,
  kLevelRatio = 8,
  kLevelAttack = 9,
  kLevelRelease = 10,
  kLevelLoCut = 11,
  kLevelTubeGain = 12,
  kLevelFeedback = 13,
  kDensityThreshold = 14,
  kDensityRatio = 15,
  kDensityAttack = 16,
  kDensityRelease = 17,
  
  kPortCount = 18,
};

typedef struct {
  CompressorChain dsp;
  LADSPA_Data* ports[kPortCount];
} MangrovePlugin;
```

### Task 8.2: Implement LADSPA Callbacks

**File:** `Source/LADSPA/MangroveLADSPA.cpp`

```cpp
#include "MangroveLADSPA.h"
#include <cstring>
#include <cmath>

static LADSPA_Descriptor g_mangroveDescriptor = {
  .UniqueID = 42,
  .Label = "mangrove_compressor",
  .Name = "Mangrove Compressor Chain",
  .Maker = "Nassau",
  .Copyright = "Copyright 2026",
  .PortCount = kPortCount,
  .PortDescriptors = nullptr,  // Filled in init
  .PortNames = nullptr,
  .PortRangeHints = nullptr,
  .ImplementationData = nullptr,
  .instantiate = instantiate,
  .activate = activate,
  .run = run,
  .run_adding = nullptr,
  .set_run_adding_gain = nullptr,
  .deactivate = deactivate,
  .cleanup = cleanup,
};

LADSPA_Handle instantiate(const LADSPA_Descriptor* descriptor,
                          unsigned long sampleRate) {
  MangrovePlugin* plugin = new MangrovePlugin();
  plugin->dsp.init((float)sampleRate);
  return (LADSPA_Handle)plugin;
}

void activate(LADSPA_Handle instance) {
  // No special activation needed
}

void run(LADSPA_Handle instance, unsigned long sampleCount) {
  MangrovePlugin* plugin = (MangrovePlugin*)instance;
  
  // Read parameters
  plugin->dsp.setInputGain(*plugin->ports[kInputGain]);
  plugin->dsp.setInputLoCut(*plugin->ports[kInputLoCut]);
  plugin->dsp.setInputSaturate(*plugin->ports[kInputSaturate]);
  plugin->dsp.setLevelThreshold(*plugin->ports[kLevelThreshold]);
  plugin->dsp.setLevelRatio(*plugin->ports[kLevelRatio]);
  plugin->dsp.setLevelAttack(*plugin->ports[kLevelAttack]);
  plugin->dsp.setLevelRelease(*plugin->ports[kLevelRelease]);
  plugin->dsp.setLevelLoCut(*plugin->ports[kLevelLoCut] >= 0.5f);
  plugin->dsp.setLevelTubeGain(*plugin->ports[kLevelTubeGain] >= 0.5f);
  plugin->dsp.setLevelFeedback(*plugin->ports[kLevelFeedback] >= 0.5f);
  plugin->dsp.setDensityThreshold(*plugin->ports[kDensityThreshold]);
  plugin->dsp.setDensityRatio(*plugin->ports[kDensityRatio]);
  plugin->dsp.setDensityAttack(*plugin->ports[kDensityAttack]);
  plugin->dsp.setDensityRelease(*plugin->ports[kDensityRelease]);
  
  // Process audio
  plugin->dsp.process(
    plugin->ports[kAudioInLeft],
    plugin->ports[kAudioInRight],
    plugin->ports[kAudioOutLeft],
    plugin->ports[kAudioOutRight],
    (int)sampleCount);
}

void deactivate(LADSPA_Handle instance) {
  // No cleanup needed
}

void cleanup(LADSPA_Handle instance) {
  delete (MangrovePlugin*)instance;
}

// Library entry point
const LADSPA_Descriptor* ladspa_descriptor(unsigned long index) {
  if (index == 0) {
    return &g_mangroveDescriptor;
  }
  return nullptr;
}
```

### Task 8.3: Create CMake Build

**File:** `Source/LADSPA/CMakeLists.txt`

```cmake
add_library(mangrove_ladspa SHARED
  MangroveLADSPA.cpp
)

target_include_directories(mangrove_ladspa PRIVATE
  ${CMAKE_SOURCE_DIR}/Source/DSP
)

target_link_libraries(mangrove_ladspa PRIVATE
  MangroveCompressorDSP
)

# Set output name to standard LADSPA convention
set_target_properties(mangrove_ladspa PROPERTIES
  PREFIX "lib"
  SUFFIX ".so"
)

# Install to standard LADSPA location
install(TARGETS mangrove_ladspa
  LIBRARY DESTINATION ~/.ladspa
)
```

### Task 8.4: Build & Install

```bash
mkdir build_ladspa && cd build_ladspa
cmake ..
cmake --build . --config Release

# Install
mkdir -p ~/.ladspa
cp libmangrove_ladspa.so ~/.ladspa/
```

### Task 8.5: Test in Ardour/JACK

1. **Launch Ardour:**
   ```bash
   ardour6
   ```

2. **Scan plugins:**
   - Ardour → Preferences → Plugins → Plugin Manager
   - Rescan LADSPA

3. **Load plugin:**
   - Create audio track
   - Insert → LADSPA Plugins → Mangrove

4. **Test:**
   - Adjust parameter sliders (host-drawn)
   - Verify audio processing

---

## Deliverables

- [ ] `Source/LADSPA/MangroveLADSPA.h` (descriptor)
- [ ] `Source/LADSPA/MangroveLADSPA.cpp` (implementation)
- [ ] `Source/LADSPA/CMakeLists.txt` (build config)
- [ ] `libmangrove_ladspa.so` (compiled library)
- [ ] Tested in Ardour

---

## Success Criteria

✅ Compiles to .so library  
✅ Loads in Ardour without crash  
✅ Audio processes correctly  
✅ Parameters appear as host-drawn sliders  
✅ Works on Linux x86_64 and ARM64  

---

## Timeline

Days 1–2: Tasks 8.1–8.2  
Day 3: Task 8.3–8.4  
Day 4: Task 8.5  

**Total: 1–2 weeks**

---

## Notes

- **No GUI:** LADSPA doesn't support custom UI; host draws sliders
- **No presets:** Host-dependent preset management
- **No parameter smoothing:** Changes apply instantly (not sample-accurate)
- **Niche format:** Primarily used in Linux (Ardour, JACK)
