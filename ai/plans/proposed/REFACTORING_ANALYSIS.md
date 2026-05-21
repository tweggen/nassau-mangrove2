# Mangrove Plugin: JUCE-to-Native Refactoring Analysis

**Project:** Mangrove Compressor Chain  
**Current Framework:** JUCE (VST3)  
**Target Platforms:** macOS (AudioUnit + VST3), Windows/Linux (VST3, LADSPA)

---

## Executive Summary

The Mangrove plugin is a **dynamic range compressor** featuring dual compression stages (level + density) with sophisticated envelope following and feedback/feedforward control. The audio processing DSP is well-encapsulated and relatively JUCE-independent, making it a **strong candidate for refactoring to native plugin formats**. 

**Estimated complexity: MODERATE-HIGH**
- **Pros:** Clean DSP separation, stateless audio algorithm, no complex MIDI
- **Cons:** Heavy GUI, parameter serialization, multi-platform requirements, sample-rate hardcoding issues

---

## Current Architecture Analysis

### Plugin Structure

```
MangroveAudioProcessor (JUCE AudioProcessor)
├── Audio Processing
│   ├── Input Stage (gain, saturation, high-pass filter)
│   ├── Level Compressor (vari-mu capable)
│   │   ├── Feedback/Feedforward selection
│   │   ├── Attack/Release envelope
│   │   └── IIR high-pass sidechain filter
│   └── Density Compressor (density limiting)
├── Parameter Management (AudioProcessorValueTreeState)
│   └── 14 parameters across 3 stages
└── GUI (MangroveAudioProcessorEditor)
    ├── 3 custom Slider subclasses
    ├── 10 parameter sliders
    ├── 4 toggle buttons
    └── Real-time metering (20 Hz timer)

MangroveAudioProcessorEditor (JUCE AudioProcessorEditor)
├── Parameter Attachments (SliderAttachment, ButtonAttachment)
├── Custom Slider Rendering (text overrides)
└── Meter Display & Logo
```

### Parameter Inventory

**Input Stage (3 params):**
- `inputGain`: -24 to +24 dB (default 0)
- `inputLoCut`: 20–300 Hz (default 80), with discrete "Disabled" state
- `inputSaturate`: 0–5 (default 0), non-linear behavior for negative samples

**Level Compressor (7 params):**
- `levelThreshold`: -60 to 0 dB (default -10)
- `levelRatio`: 1–10 (special case ≥9.999 = "Vari-Mu" mode using atan curve)
- `levelAttack`: 0–100 ms (default 10)
- `levelRelease`: 10–500 ms (default 300)
- `levelLoCut`: boolean (applies HPF to sidechain)
- `levelTubeGain`: boolean (soft-clipping nonlinearity)
- `levelFeedback`: boolean (toggles feedback vs. feedforward control mode)

**Density Compressor (4 params):**
- `densityThreshold`: -36 to 0 dB (default -10)
- `densityRatio`: 1–10 (≥9.999 = "Limiter" mode)
- `densityAttack`: 0.001–100 ms (default 10)
- `densityRelease`: 10–2000 ms (default 300)

---

## DSP Core Analysis

### Strengths
✅ **Well-isolated audio algorithm:** `processBlock()` reads parameters atomically and updates state  
✅ **Minimal JUCE dependencies:** Only `IIRFilter` and `AudioBuffer` from JUCE  
✅ **Stateful but self-contained:** All state variables are private and managed in `prepareToPlay()`  
✅ **No dynamic allocation in hot path:** Fixed-size state, no vector resizing during processing  
✅ **Clear parameter semantics:** dB/Hz/ms values with well-defined ranges  

### Weaknesses
⚠️ **Sample-rate hardcoding:** Multiple TXWTODO comments hardcode 44.1 kHz:
  - Line 174: `pow(0.5, 1. / ((*_paramLevelRelease * relSoundGoodAdjustment / 1000.) * 44100.))`
  - Line 278: Attack/release calculations assume 44.1 kHz
  - Line 354–355: IIR filter setup hardcodes 44100

⚠️ **Magic numbers & tuning constants:**
  - `relSoundGoodAdjustment = 1.0` and `attSoundGoodAdjustment = 0.1` (ratio mismatch suggests legacy tuning)
  - Attack reason logic (lines 555–604) has hardcoded multipliers (1.41× RMS, 4× for fast peak release)
  - `inSaturation * 10.0 + 1.0` (input saturation scaling is empirical)

⚠️ **Implicit stereo-only:** Only stereo (2-channel in/out) is fully tested; mono paths have commented-out code  

⚠️ **Signed/unsigned confusion:** Attack/release coefficients use both `double` and `float` with occasional casts  

### IIR Filter Usage
The code uses JUCE's `IIRFilter` class (no custom implementation). This needs replacement:
- **Dependency:** `IIRCoefficients::makeHighPass()` + `processSingleSampleRaw()`
- **Can be replaced by:** Standard Butterworth/Chebyshev design or look-up table (only 2–3 filter instances)
- **Risk:** Needs careful validation of cutoff frequency and Q factor matching

---

## Implementation Roadmap by Target Format

### 1️⃣ Shared Audio Core (Platform-Independent)

**Goal:** Extract pure DSP into a plugin-agnostic library

**Create `AudioDSP/CompressorChain` library:**

```cpp
// compressor_chain.h
class CompressorChain {
  // Initialization
  void init(float sampleRate);
  
  // Main process
  void process(const float* in_L, const float* in_R,
               float* out_L, float* out_R,
               int numSamples);
  
  // Parameter setters (atomic-safe)
  void setInputGain(float gainDb);
  void setLevelThreshold(float dbThreshold);
  // ... 14 setter methods
  
  // Getters for metering (RMS, reduction, etc.)
  float getInputGain() const;
  float getLevelReduction() const;
  float getDensityReduction() const;
  
private:
  // Parameter storage (std::atomic<float>)
  std::atomic<float> _inputGain, _inputLoCut, ...;
  
  // Filter objects (replace JUCE IIRFilter)
  IIRFilterState _levelHPF_L, _levelHPF_R, _levelHPF_Sidechain;
  
  // Compressor state (lines 136–148 of PluginProcessor.h)
  double _levelDetection, _levelEnvFollow, _levelAmplification, ...;
  
  // Implementation: port PluginProcessor.cpp:processBlock() logic
};
```

**Benefits:**
- Single source of truth for audio behavior
- Easy unit testing with raw audio buffers
- No plugin-specific boilerplate in DSP code

---

### 2️⃣ macOS AudioUnit (AU) – Legacy (AUv2) + Modern (AUv3)

#### Overview
- **AUv2:** Component-based, macOS 10.5–10.14 (deprecated, browser plugins forbidden)
- **AUv3:** App Extension (iOS/macOS 10.11+, App Store compatible)

#### Implementation Path

**AUv2 (if legacy support required):**
```cpp
// mangrove_auv2.cpp
#include <AudioToolbox/AudioToolbox.h>
#include "compressor_chain.h"

struct MangroveAUv2 {
  CompressorChain dsp;
  
  // Kinda-standard AU callbacks
  OSStatus Initialize();
  OSStatus Render(AudioUnitRenderActionFlags*, 
                  const AudioTimeStamp*,
                  UInt32 inOutputBusNumber,
                  UInt32 inNumberFrames,
                  AudioBufferList* ioData);
  OSStatus GetProperty(AudioUnitPropertyID, ...);
  OSStatus SetProperty(AudioUnitPropertyID, ...);
};

// Register component
AudioComponentDescription desc = {
  .componentType = kAudioUnitType_Dynamics,
  .componentSubType = 'MngV', // or 'mgro'
  .componentManufacturer = 'NSAU',
  .componentFlags = 0
};
```

**AUv3 (recommended, modern, App Store distribution):**
```cpp
// MangroveAUv3.swift + Bridging C++
import AudioToolbox

class MangroveAUv3: AUAudioUnit {
  override func instantiateViewController(...) -> NSViewController {
    // Link to native SwiftUI or Cocoa GUI
  }
  
  override func allocateRenderResources() {
    dsp.init(Float(outputBusses[0].format.sampleRate))
  }
  
  override var internalRenderBlock: AUInternalRenderBlock {
    return { [weak self] actionFlags, timestamp, frameCount, inputBusNumber, inputData, outputData, ... in
      self?.dsp.process(...)
    }
  }
}
```

**Parameter Bridging:**
AU parameters require CFStringRef identifiers and ranges. Map 14 DSP params → AU parameters:

```cpp
AUParameterTree::createWithChildren(@[
  AUParameter(identifier: "inputGain", name: "Input Gain",
              address: 0, min: -24, max: 24, unit: .decibels, unitName: "dB", ...),
  // ... 13 more
]);
```

**GUI Challenge:** 
- AUv2 uses Carbon/Cocoa (outdated); AUv3 uses App Extensions
- **Option A:** Minimal plugin UI (host-drawn sliders)
- **Option B:** SwiftUI/Cocoa custom UI (requires AUv3 + host support for embedded views)
- **Option C:** Hybrid (minimal AU UI + external app for full control)

**Serialization:**
```cpp
// AUv3 uses NSCoding + NSUserDefaults, or custom binary archives
- (void)setState:(NSDictionary *)state {
  self.inputGain = [state[@"inputGain"] floatValue];
  // ...
}
```

**⚠️ Pitfalls:**
- AUv2 plugins must run in the plugin process (no sandboxing); App Store rejects
- AUv3 UI is optional but expected; blank parameter list = poor UX
- OS-specific abstractions (Dispatch, Core Audio, etc.) not cross-platform

**Effort:** 8–12 weeks (AUv3 with custom UI)

---

### 3️⃣ VST 3 (macOS/Windows) – Recommended Primary Target

VST 3 is well-supported across all platforms and has good GUI tooling.

#### Implementation Path

**Option A: Use IPlug2 (wrapper library)**

IPlug2 abstracts VST 3 boilerplate and provides cross-platform parameter/UI:

```cpp
#include "IPlug_include_in_plug_hdr.h"
#include "compressor_chain.h"

class MangroveVST3Plugin : public Plugin {
  CompressorChain dsp;
  
  MangroveVST3Plugin(const InstanceInfo& info)
    : Plugin(info, MakeConfig(...)) {
    
    GetParam(kInputGain)->InitDouble(
      "Input Gain", 0, -24, 24, 0.01, "dB");
    // ... 13 more params
    
    mMakeGraphicsFunc = [&]() { 
      return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, 
                          PLUG_FPS, GetScaleForScreen(GetMainWnd()));
    };
  }
  
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
    dsp.process(inputs[0], inputs[1], outputs[0], outputs[1], nFrames);
    // Update metering for UI
  }
  
  // IGraphics provides widget layer
  void OnParamChange(int paramIdx) override {
    switch(paramIdx) {
      case kInputGain: dsp.setInputGain(GetParam(kInputGain)->Value()); break;
      // ...
    }
  }
};
```

IPlug2 handles:
- VST 3 wrapper generation
- Parameter serialization (state chunk format)
- Host communication (sample rate, block size, etc.)
- Cross-platform (Windows/macOS/Linux via CMake)
- Bundled graphics library (IGraphics with skia/cairo backends)

**Option B: Direct VST 3 SDK**

Raw VST 3 implementation (more control, more boilerplate):

```cpp
#include <public.sdk/source/vst/vstaudioeffect.h>
#include "compressor_chain.h"

class MangroveVST3 : public AudioEffect {
  CompressorChain dsp;
  
  tresult initialize(FUnknown* context) override {
    // Add buses, parameters, etc.
    return AudioEffect::initialize(context);
  }
  
  tresult process(ProcessData& data) override {
    // Call dsp.process() with data.inputs/outputs
    return kResultOk;
  }
  
  tresult setState(IBStream* state) override {
    // Deserialize 14 parameters from stream
    // Binary format: 14 × float32 = 56 bytes
    float params[14];
    state->read(params, sizeof(params), nullptr);
    dsp.setInputGain(params[0]);
    // ...
  }
  
  tresult getState(IBStream* state) override {
    float params[14] = { dsp.getInputGain(), ... };
    state->write(params, sizeof(params), nullptr);
  }
};
```

**Parameter Bridging:**

```cpp
addParameter(
  new RangeParameter(
    USTRING("Input Gain"), kInputGainId,
    USTRING("dB"), -24.0, 24.0, 0.0,
    ParameterInfo::kCanAutomate)
);
```

**GUI Options:**
- IPlug2's IGraphics (easiest, uses vector drawing + platform backends)
- VSTGUI (venerable VST GUI framework)
- Custom OpenGL/Cairo (full control, more code)
- Minimal UI (parameter list only, host draws controls)

**Serialization:**
VST 3 uses binary streams. Key design:
```cpp
// Simple approach: 14 floats in order
void serialize(float buffer[14]) {
  buffer[0] = inputGain;
  buffer[1] = inputLoCut;
  // ...
}
void deserialize(const float buffer[14]) {
  inputGain = buffer[0];
  // ...
}
```

**⚠️ Pitfalls:**
- VST 3.6+ requires AudioProcessor interface changes (breaking changes to AUv2)
- Parameter automation requires careful buffering (sample-accurate modulation)
- GUI thread ↔ audio thread synchronization (parameter queue)
- Host-specific quirks (some DAWs don't support non-standard parameter ranges)

**Effort:** 4–6 weeks with IPlug2; 10–12 weeks with raw VST 3 SDK

**Recommendation:** Use IPlug2 for rapid cross-platform deployment.

---

### 4️⃣ LADSPA (Linux) – Lowest Priority

**Overview:**  
LADSPA is a simple, plugin-less standard for Linux audio. No real-time preset save, minimal GUI, host-driven parameter control.

#### Implementation Path

```cpp
#include <ladspa.h>
#include "compressor_chain.h"

#define PLUGIN_UNIQUE_ID 42
#define PORT_COUNT 14 * 2 + 2 // in_L, in_R, out_L, out_R, + 14 params

static LADSPA_Descriptor g_descriptor = {
  .UniqueID = PLUGIN_UNIQUE_ID,
  .Label = "mangrove_compressor",
  .Name = "Mangrove Compressor Chain",
  .Maker = "Nassau",
  .Copyright = "...",
  .PortCount = PORT_COUNT,
  .PortDescriptors = port_descriptors,
  .PortNames = port_names,
  .PortRangeHints = port_hints,
  .instantiate = instantiate_plugin,
  .activate = activate_plugin,
  .run = run_plugin,
  .deactivate = deactivate_plugin,
  .cleanup = cleanup_plugin
};

typedef struct {
  CompressorChain dsp;
  LADSPA_Data* ports[PORT_COUNT];
} MangrovePlugin;

LADSPA_Handle instantiate_plugin(...) {
  MangrovePlugin* plugin = new MangrovePlugin();
  plugin->dsp.init(sample_rate);
  for (int i = 0; i < PORT_COUNT; i++) {
    plugin->ports[i] = port_data[i];
  }
  return plugin;
}

void run_plugin(LADSPA_Handle handle, unsigned long sample_count) {
  MangrovePlugin* plugin = (MangrovePlugin*)handle;
  
  // Read parameter values from ports
  plugin->dsp.setInputGain(*plugin->ports[kInputGain]);
  plugin->dsp.setInputLoCut(*plugin->ports[kInputLoCut]);
  // ... 12 more
  
  // Process audio
  plugin->dsp.process(
    plugin->ports[kInL], plugin->ports[kInR],
    plugin->ports[kOutL], plugin->ports[kOutR],
    sample_count
  );
}

const LADSPA_Descriptor* ladspa_descriptor(unsigned long idx) {
  return idx == 0 ? &g_descriptor : nullptr;
}
```

**Key Limitations:**
- No built-in serialization (host-dependent presets)
- No GUI (host provides sliders)
- No real-time parameter smoothing (instantaneous changes)
- Single mono instance per plugin (no multichannel, no side-chain)

**Bundling:**
LADSPA plugins are `.so` files dropped into `~/.ladspa/` or `/usr/lib/ladspa/`.

**⚠️ Pitfalls:**
- No preset system → user has to manually recreate settings
- Parameter ranges specified as hints, not enforced
- Process block size is host-determined, no pre-allocation guarantee
- Rarely used in modern DAWs; mainly for CLI tools (JACK, Sox plugins, etc.)

**Effort:** 1–2 weeks (minimal GUI means less work)

**Recommendation:** LADSPA as a bonus export if using IPlug2; low ROI for standalone LADSPA support.

---

## Parameter Serialization Strategy

### Current JUCE Approach
JUCE's `AudioProcessorValueTreeState` serializes to XML:
```xml
<NassauMangrove>
  <PARAM id="inputGain" value="0.0"/>
  <PARAM id="inputLoCut" value="80.0"/>
  <!-- ... -->
</NassauMangrove>
```

### Cross-Platform Serialization Options

#### Option 1: Binary Fixed Layout (Simplest)
```
Byte Layout:
  0-3:   float inputGain
  4-7:   float inputLoCut
  8-11:  float inputSaturate
  12-15: float levelThreshold
  ... (14 × 4 bytes = 56 bytes total)
```
Pros: Fast, compact, simple versioning (prepend version byte)  
Cons: Brittle if parameter order changes

#### Option 2: JSON (Human-Readable)
```json
{
  "version": 1,
  "parameters": {
    "inputGain": 0.0,
    "inputLoCut": 80.0,
    ...
  }
}
```
Pros: Human-editable, extensible, version-aware  
Cons: Larger file size, slower parsing

#### Option 3: Protocol Buffers / MessagePack (Hybrid)
- Compact binary with schema versioning
- Cross-language compatibility (if used elsewhere)

**Recommendation:** Binary fixed layout for VST 3 (fast state save), JSON for user presets/manual editing.

---

## Sample Rate Handling

### Current Issues
The code hardcodes 44.1 kHz in critical places:
```cpp
// PluginProcessor.cpp:174
float release = pow(0.5, 1. / ((*_paramLevelRelease * relSoundGoodAdjustment / 1000.) * 44100.));
```

Modern DAWs use 48 kHz (video), 96 kHz (mastering), or higher.

### Required Changes

**1. Store sample rate in processor:**
```cpp
class CompressorChain {
private:
  float _sampleRate;
public:
  void init(float sampleRate) {
    _sampleRate = sampleRate;
    // Recalculate all time-based coefficients
    recalculateAttackReleaseCoefficients();
  }
};
```

**2. Recalculate attack/release whenever sample rate or parameters change:**
```cpp
void recalculateAttackReleaseCoefficients() {
  float relSoundGoodAdjustment = 1.0;
  _levelReleaseCoeff = pow(0.5, 1. / ((_levelRelease * relSoundGoodAdjustment / 1000.) * _sampleRate));
}
```

**3. Dynamic IIR filter recomputation:**
```cpp
// Already done in the code (line 354–355), but ensure sample rate is used:
_levelHighpassInputLeft.setCoefficients(
  IIRCoefficients::makeHighPass(_sampleRate, localInputLoCutFrequency, 0.8)
);
```

**Impact:** ~2–4 hours refactoring; minimal behavior change if tuning constants are sample-rate-independent.

---

## GUI/Visualization Architecture

### Current JUCE GUI
- 10 horizontal sliders (with custom text overrides)
- 4 toggle buttons
- Real-time metering (input level, compression reduction)
- Static logo image

### Refactoring Approach by Format

#### AudioUnit (AUv3)
**Option 1: Minimal (Host-Drawn)**
- Suppress custom UI; host provides sliders
- Only expose parameter list + ranges
- Fast, works everywhere, but bland

**Option 2: Native SwiftUI**
```swift
struct MangroveAUv3UI: View {
  @State var inputGain: Float
  @State var levelReduction: Float // metering (read-only)
  
  var body: some View {
    VStack {
      GroupBox(label: Text("Input")) {
        Slider(value: $inputGain, in: -24...24)
        // ... custom styling
      }
      // ... density section
      HStack {
        Text("Level Reduction"); Spacer()
        Text(String(format: "%.1f dB", levelReduction))
      }
    }
  }
}
```
Requires AUv3; clean, native feel; limited Windows support.

#### VST 3 with IPlug2
IPlug2's `IGraphics` provides:
- Vector drawing (skia-based, CPU-efficient)
- Cross-platform (Windows/macOS/Linux)
- Slider, button, meter widgets
- Real-time metering via plugin-to-UI queue

```cpp
void OnIdle() override {
  // Fetch metering data from DSP
  float reduction = dsp.getLevelReduction();
  GetUI()->SetParameterFromPlug(kLevelReductionMeter, reduction / 60.0); // normalized
}
```

#### LADSPA
No GUI; host draws sliders.

### Meter Display
Real-time metering (input RMS, compression reduction) is crucial for usability. Add to `CompressorChain`:
```cpp
struct MeterData {
  float inputGain;      // RMS pre-compression
  float levelReduction; // amplification factor (0–1)
  float densityReduction;
};

void getMeterData(MeterData& out) const {
  out.inputGain = _inputGainAccumulator;
  out.levelReduction = _currLevelReduction;
  out.densityReduction = _currDenseReduction;
}
```

Query from UI thread at ~20 Hz (non-blocking):
```cpp
// VST 3 / IPlug2
void ProcessBlock(...) {
  dsp.process(...);
  if (++mMeterCounter % mBlockSize == 0) {
    mMeters = dsp.getMeterData(); // lock-free atomic swap
  }
}
```

---

## Platform-Specific Challenges

### macOS
✅ AudioUnit (AUv3) → native integration (Logic, Final Cut)  
✅ VST 3 → broad DAW support  
⚠️ Notarization required for distribution (Apple signing)  
⚠️ Rosetta 2 translation if building for Intel-only

### Windows
✅ VST 3 → universal  
✅ ASIO support for low-latency (IPlug2 abstracts)  
⚠️ Installer packaging (NSIS, MSI)  
⚠️ DLL loader issues (UAC, library paths)

### Linux
✅ VST 3 (via Wine/Yabridge if needed for VST 2 compatibility)  
✅ LADSPA (native, simple)  
⚠️ glibc/libstdc++ version mismatch across distros  
⚠️ GUI toolkit fragmentation (X11 vs. Wayland)

---

## Code Quality & Testing

### Current State
- ⚠️ No unit tests for DSP
- ⚠️ Magic numbers throughout (hardcoded attack/release ratios, 1.41× multipliers)
- ⚠️ Inconsistent naming (local variables with prefix, state with underscore)
- ✅ Clear signal flow in `processBlock()`

### Required Additions

**Unit Tests (C++ with Catch2 or Google Test):**
```cpp
TEST_CASE("Level compressor threshold behavior") {
  CompressorChain dsp;
  dsp.init(44100);
  dsp.setLevelThreshold(-10); // dB
  dsp.setLevelRatio(2.5);
  
  // Input 20 dB above threshold → output should be ~15 dB above
  float expectedReduction = pow(10, -5.0 / 20); // 0.562...
  // REQUIRE_THAT(reduction, Catch::Matchers::WithinAbs(expectedReduction, 0.001));
}
```

**Regression Suite:**
- Comparison against original JUCE build (impulse responses)
- Sweep tests (frequency response integrity)
- Edge case: silence input, extreme parameters, sample-rate changes mid-stream

**⚠️ Critical:** DSP behavior must be **bit-exact** across platforms (or at least perceptually identical).

---

## Migration Effort Breakdown

| Phase | Component | Effort | Risk | Notes |
|-------|-----------|--------|------|-------|
| 1 | Extract CompressorChain DSP library | 2–3 weeks | LOW | Port 850 lines of processBlock logic, replace JUCE IIRFilter |
| 2 | Implement IIR filter replacement | 1–2 weeks | MEDIUM | Must match original frequency response exactly |
| 3 | Fix sample-rate handling | 1–2 weeks | LOW | Straightforward math fixes |
| 4 | VST 3 wrapper (IPlug2) | 3–4 weeks | LOW | Boilerplate-heavy but well-documented |
| 5 | GUI (IGraphics) | 2–3 weeks | MEDIUM | Layout/styling work, no complex widgets needed |
| 6 | Serialization (binary + JSON) | 1–2 weeks | LOW | Straightforward array→map conversion |
| 7 | AUv3 wrapper (macOS) | 4–6 weeks | MEDIUM | Requires macOS SDK knowledge, SwiftUI optional |
| 8 | LADSPA wrapper (Linux) | 1–2 weeks | LOW | Simple, minimal boilerplate |
| 9 | Testing & validation | 3–4 weeks | HIGH | Perceptual testing, cross-platform regression |
| 10 | Packaging & distribution | 2–3 weeks | MEDIUM | Codesigning, installers, version management |

**Total Estimate:** 20–30 weeks (5–7 months) for full multi-platform release with VST 3 + AUv3 + LADSPA

**MVP (VST 3 on Windows/macOS only):** 8–10 weeks

---

## Recommended Implementation Sequence

### Phase 1: Proof of Concept (Weeks 1–3)
1. Extract `CompressorChain` class with minimal dependencies
2. Replace JUCE `IIRFilter` with custom Butterworth implementation or cookbook recipe
3. Fix sample-rate handling (add `init()` parameter)
4. Validate with synthetic test signals (match original output)

### Phase 2: VST 3 Prototype (Weeks 4–7)
1. Set up IPlug2 project skeleton
2. Connect CompressorChain to IPlug2's process block
3. Implement parameter mapping (14 params → IPlug2)
4. Build minimal GUI (sliders, meter)
5. Test in one DAW (Reaper, Studio One, etc.)

### Phase 3: Full VST 3 + AUv3 (Weeks 8–16)
1. Complete VST 3 GUI polish
2. Implement AUv3 wrapper (optional native UI)
3. Cross-platform testing (macOS/Windows)
4. Preset system (JSON + binary formats)

### Phase 4: Linux & Bonus Formats (Weeks 17–22)
1. LADSPA wrapper
2. VST 2 wrapper (if needed for legacy compatibility, via IPlug2)
3. Linux-specific packaging (Flatpak, snap, AppImage)

### Phase 5: Release & Maintenance (Weeks 23–30)
1. Comprehensive regression testing
2. Documentation (parameter reference, GUI tour, presets)
3. Codesigning & notarization (macOS)
4. Build automation (CI/CD)
5. Version management & updater

---

## Risks & Mitigation

| Risk | Impact | Mitigation |
|------|--------|-----------|
| DSP behavior mismatch | HIGH | Bit-exact reference tests; audio file comparison |
| IIR filter accuracy | HIGH | Use established cookbook recipe (RBJ); validate freq response |
| Parameter serialization loss | MEDIUM | Test round-trip: save → load → verify values |
| Real-time audio glitches | MEDIUM | Buffer parameter changes; use lock-free queues |
| macOS notarization delays | LOW | Start codesigning early; test sandbox restrictions |
| Plugin host incompatibilities | MEDIUM | Test on: Reaper, Studio One, Logic, Pro Tools, Ableton |

---

## Recommended Tools & Libraries

### Audio Processing
- **IPlug2:** Cross-platform VST 3/AU/LADSPA wrapper, IGraphics GUI
- **JUCE modules (subset):** Keep only `juce_audio_basics` for DSP reference
- **Cookbook DSP Algorithms:** For IIR filter design (no external deps)

### Build System
- **CMake 3.15+:** Portable, supports all platforms
- **Conan/vcpkg:** Dependency management (if any external libs needed)

### Testing
- **Catch2 or Google Test:** C++ unit tests
- **Sox / JUCE AudioPlayer:** Generating test signals
- **Audacity / RMS meter:** Visual validation of output

### Development
- **Xcode (macOS):** AUv3 development
- **Visual Studio 2019+ (Windows):** VST 3 development
- **GCC/Clang (Linux):** LADSPA development
- **JetBrains CLion:** IDE supporting all platforms

---

## Conclusion

**Feasibility: HIGH**

The Mangrove plugin is a strong refactoring candidate because:
1. ✅ DSP core is self-contained and well-structured
2. ✅ No complex MIDI, side-chain, or polyphony requirements
3. ✅ Parameters are straightforward (14 floats/bools, no complex dependencies)
4. ✅ Sample-rate issues are straightforward to fix

**Highest Value Path:** VST 3 via IPlug2 (8–10 weeks to MVP) → AUv3 (add 4–6 weeks) → LADSPA bonus (add 1–2 weeks)

**Effort Reduction Opportunities:**
- Skip AUv2 (deprecated, App Store banned)
- Use IPlug2 instead of raw VST 3 SDK (saves ~4 weeks boilerplate)
- Minimal preset editor (text entry only, no graphical curves)
- LADSPA as a bonus if using IPlug2 (no extra effort)

**Next Steps:**
1. Audit IIR filter implementation (current JUCE dependency)
2. Prototype `CompressorChain` extraction with test harness
3. Evaluate IPlug2 integration overhead
4. Define acceptable DSP tolerance (exact match vs. perceptual equivalence)
