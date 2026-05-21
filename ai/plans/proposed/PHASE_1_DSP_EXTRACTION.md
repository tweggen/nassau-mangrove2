# Phase 1: DSP Extraction

**Objective:** Extract audio processing logic from JUCE-dependent `PluginProcessor` into a standalone `CompressorChain` C++ class with zero external dependencies.

**Duration:** 2–3 weeks  
**Dependencies:** None (this is the first phase)  
**Produces:** Reusable DSP library (`Source/DSP/compressor_chain.h`, `.cpp`)  
**Tests Required:** Unit tests covering all compression stages  

---

## Overview

The current `PluginProcessor::processBlock()` (lines 226–853 in `PluginProcessor.cpp`) contains ~850 lines of audio processing logic. This phase extracts that logic into a platform-independent `CompressorChain` class that:

- ✅ Takes raw audio buffers (float pointers) as input
- ✅ Applies gain, saturation, and dual-stage compression
- ✅ Stores all state internally (no reliance on JUCE's `AudioBuffer`)
- ✅ Accepts parameters via atomic setters (thread-safe, lock-free)
- ✅ Has no JUCE dependencies (uses only C++ standard library)

### Key Algorithms Extracted

1. **Input Stage** (lines 252–290)
   - Gain application (dB to linear conversion)
   - High-pass filter for input LoCut
   - Soft saturation for negative samples

2. **Level Compressor** (lines 320–696)
   - Sidechain detection (absolute value + filtering)
   - High-pass filter on sidechain (optional)
   - Envelope follower (attack/release)
   - Compression curve (hard-knee vs. Vari-Mu)
   - Feedback vs. feedforward control mode
   - Tube saturation (soft clipping)

3. **Density Compressor** (lines 752–849)
   - Peak or RMS detection
   - Fast envelope follower
   - Hard-knee compression curve
   - Limiter mode (ratio ≥ 9.999)

---

## Acceptance Criteria

Phase 1 is **complete** when:

- [ ] `CompressorChain` class compiles without warnings
- [ ] All 14 parameters have setter methods with atomic storage
- [ ] `process()` method accepts two stereo float buffers (in/out)
- [ ] Meter data available via `getMeterData()` getter
- [ ] Unit tests exist for each sub-stage (input, level, density)
- [ ] Audio output matches original JUCE code (RMS error < 0.01 dB)
- [ ] No JUCE includes in `compressor_chain.h` or `.cpp`
- [ ] No dynamic allocation in audio hot path
- [ ] Code passes Clang-Tidy and compiler warning checks
- [ ] Documentation complete (function headers + algorithm comments)

---

## Detailed Tasks

### Task 1.1: Create Project Structure & CMake Setup

**What:** Set up directory layout and CMake build configuration  
**Duration:** 2–4 hours  
**Files to Create/Modify:**
- `CMakeLists.txt` (root)
- `Source/DSP/CMakeLists.txt`
- `Tests/CMakeLists.txt`

**Steps:**

1. Create directories:
   ```bash
   mkdir -p Source/DSP
   mkdir -p Tests
   ```

2. Create root `CMakeLists.txt`:
   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(MangrovePlugin VERSION 2.0.0 LANGUAGES CXX)
   
   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
   
   add_subdirectory(Source/DSP)
   add_subdirectory(Tests)
   ```

3. Create `Source/DSP/CMakeLists.txt`:
   ```cmake
   add_library(MangroveCompressorDSP STATIC
     compressor_chain.cpp
   )
   target_include_directories(MangroveCompressorDSP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
   target_compile_options(MangroveCompressorDSP PRIVATE -fvisibility=hidden)
   ```

4. Create `Tests/CMakeLists.txt`:
   ```cmake
   add_executable(MangroveTests
     dsp_tests.cpp
   )
   target_link_libraries(MangroveTests MangroveCompressorDSP)
   add_test(NAME CompressorDSP COMMAND MangroveTests)
   ```

5. Create `Tests/catch.hpp` (Catch2 header-only framework):
   - Download from https://raw.githubusercontent.com/catchorg/Catch2/devel/single_include/catch2/catch.hpp
   - Save to `Tests/catch.hpp`

6. Build & verify:
   ```bash
   cd /path/to/mangrove-refactored
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

**Acceptance:** CMake configures without errors; empty test executable builds.

---

### Task 1.2: Create CompressorChain Header (API Definition)

**What:** Define the public interface of `CompressorChain`  
**Duration:** 2–4 hours  
**File to Create:** `Source/DSP/compressor_chain.h`

**Requirements:**
- No JUCE includes
- Use only C++ standard library (`<atomic>`, `<cmath>`)
- Documented method signatures
- Clear parameter ranges and defaults

**Steps:**

1. Create `Source/DSP/compressor_chain.h`:

```cpp
#pragma once

#include <atomic>
#include <cmath>

/**
 * MangroveCompressorChain
 * 
 * Dual-stage dynamic range compressor:
 * - Level: Vari-mu capable, feedback/feedforward selectable
 * - Density: High-frequency limiting
 * 
 * Thread-safe parameter updates via std::atomic<float>.
 * Lock-free audio processing (no mutex in process()).
 */
class CompressorChain {
public:
  // ===== Lifecycle =====
  
  /**
   * Create a new compressor instance.
   * Call init() before processing audio.
   */
  CompressorChain();
  ~CompressorChain();
  
  /**
   * Initialize DSP state and filter coefficients.
   * Must be called before first process() call and whenever sample rate changes.
   * 
   * @param sampleRate Sample rate in Hz (e.g., 44100, 48000, 96000)
   */
  void init(float sampleRate);
  
  // ===== Audio Processing =====
  
  /**
   * Process one block of stereo audio.
   * 
   * @param inL  Input left channel (mono array)
   * @param inR  Input right channel (mono array)
   * @param outL Output left channel (can be same as inL for in-place processing)
   * @param outR Output right channel (can be same as inR)
   * @param numSamples Number of samples to process
   * 
   * Note: For in-place processing, pass identical pointers for in/out.
   */
  void process(const float* inL, const float* inR,
               float* outL, float* outR,
               int numSamples);
  
  // ===== Input Stage Parameters =====
  
  /// Input gain in dB. Range: -24 to +24. Default: 0.
  void setInputGain(float gainDb);
  
  /// Input high-pass filter cutoff in Hz. Range: 20 to 300. Default: 80.
  /// Values below 20.1 disable the filter.
  void setInputLoCut(float freqHz);
  
  /// Input saturation amount. Range: 0 to 5. Default: 0.
  /// Controls soft-clipping nonlinearity for negative samples.
  void setInputSaturate(float amount);
  
  // ===== Level Compressor Parameters =====
  
  /// Threshold in dB. Range: -60 to 0. Default: -10.
  /// Signal level above this triggers compression.
  void setLevelThreshold(float dbThreshold);
  
  /// Compression ratio. Range: 1 to 10. Default: 2.5.
  /// Values >= 9.999 activate Vari-Mu (atan) curve.
  void setLevelRatio(float ratio);
  
  /// Attack time in ms. Range: 0 to 100. Default: 10.
  /// Time to reach target amplification after threshold crossing.
  void setLevelAttack(float attackMs);
  
  /// Release time in ms. Range: 10 to 500. Default: 300.
  /// Time to restore amplification when signal drops below threshold.
  void setLevelRelease(float releaseMs);
  
  /// Enable/disable high-pass filter on compressor sidechain. Default: false.
  /// When enabled, reduces sensitivity to low frequencies.
  void setLevelLoCut(bool enabled);
  
  /// Enable/disable soft-clipping (tube saturation). Default: false.
  /// When enabled, adds harmonic distortion when signal is amplified.
  void setLevelTubeGain(bool enabled);
  
  /// Enable/disable feedback control mode. Default: true.
  /// Feedback: compressor responds to output level.
  /// Feedforward: compressor responds to input level.
  void setLevelFeedback(bool feedbackMode);
  
  // ===== Density Compressor Parameters =====
  
  /// Threshold in dB. Range: -36 to 0. Default: -10.
  /// Controls onset of fast limiting on peak transients.
  void setDensityThreshold(float dbThreshold);
  
  /// Compression ratio. Range: 1 to 10. Default: 1.
  /// Values >= 9.999 activate limiter mode (ceiling).
  void setDensityRatio(float ratio);
  
  /// Attack time in ms. Range: 0.001 to 100. Default: 10.
  /// Very fast (1 ms typical) for transient catching.
  void setDensityAttack(float attackMs);
  
  /// Release time in ms. Range: 10 to 2000. Default: 300.
  /// Longer than level compressor for gentle unmasking.
  void setDensityRelease(float releaseMs);
  
  // ===== Metering =====
  
  struct MeterData {
    float inputGain;         // RMS level of input signal
    float levelReduction;    // Current amplification factor (0 to 1)
    float densityReduction;  // Current limiter amplification (0 to 1)
  };
  
  /**
   * Get current meter values (for UI display).
   * Safe to call from any thread; uses lock-free snapshot.
   */
  MeterData getMeterData() const;
  
  // ===== Utility =====
  
  /// Query current sample rate (set in init()).
  float getSampleRate() const { return _sampleRate; }
  
  /// Query input impedance (for latency reporting; this plugin has 0 latency).
  int getLatencySamples() const { return 0; }

private:
  // ===== Configuration =====
  float _sampleRate = 44100.0f;
  
  // ===== Input Stage State =====
  double _inputPreviousFreq = -1.0;
  
  // ===== Level Compressor State =====
  double _levelDetection = 0.0;
  double _levelFilteredSideChain = 0.0;
  double _levelEnvFollow = 0.0;
  double _levelAmplification = 1.0;
  double _levelCurrentRelease = 0.0;
  
  enum AttackReason {
    AttackNone,
    AttackPeak,
    AttackRMS
  };
  AttackReason _attackReason = AttackNone;
  
  // ===== Density Compressor State =====
  double _densityDetection = 0.0;
  double _densitySideChain = 0.0;
  double _densityEnvFollow = 0.0;
  double _densityAmplification = 1.0;
  
  // ===== Parameters (Atomic for Lock-Free Access) =====
  std::atomic<float> _paramInputGain{0.0f};
  std::atomic<float> _paramInputLoCut{80.0f};
  std::atomic<float> _paramInputSaturate{0.0f};
  
  std::atomic<float> _paramLevelThreshold{-10.0f};
  std::atomic<float> _paramLevelRatio{2.5f};
  std::atomic<float> _paramLevelAttack{10.0f};
  std::atomic<float> _paramLevelRelease{300.0f};
  std::atomic<float> _paramLevelLoCut{0.0f};
  std::atomic<float> _paramLevelTubeGain{0.0f};
  std::atomic<float> _paramLevelFeedback{1.0f};
  
  std::atomic<float> _paramDensityThreshold{-10.0f};
  std::atomic<float> _paramDensityRatio{1.0f};
  std::atomic<float> _paramDensityAttack{10.0f};
  std::atomic<float> _paramDensityRelease{300.0f};
  
  // ===== Cached Meter Data =====
  mutable std::atomic<float> _meterInputGain{0.0f};
  mutable std::atomic<float> _meterLevelReduction{1.0f};
  mutable std::atomic<float> _meterDensityReduction{1.0f};
  
  // ===== IIR Filter State (Placeholder) =====
  // Will be replaced in Phase 2 with custom IIRFilter class
  // For now, JUCE IIRFilter is used (transitional)
  class IIRFilterState {
  public:
    void reset() {}
    void setCoefficients(float b0, float b1, float b2, 
                         float a1, float a2) {}
    float processSingleSample(float x) { return x; }
  };
  
  IIRFilterState _levelHighpassInputLeft;
  IIRFilterState _levelHighpassInputRight;
  IIRFilterState _levelHighpassSidechain;
  
  // ===== Implementation Details =====
  void recalculateTimeCoefficients();
  
  // Prevent copy/move (audio objects should be created once)
  CompressorChain(const CompressorChain&) = delete;
  CompressorChain& operator=(const CompressorChain&) = delete;
};
```

**Acceptance:**
- [ ] Header compiles without warnings
- [ ] All 14 parameters have setter methods
- [ ] `process()` signature matches specification
- [ ] `MeterData` struct defined
- [ ] No JUCE includes

---

### Task 1.3: Implement CompressorChain Constructor & Initialization

**What:** Implement `CompressorChain::CompressorChain()` and `init()`  
**Duration:** 2–3 hours  
**File:** `Source/DSP/compressor_chain.cpp` (new file)

**Steps:**

1. Create skeleton `Source/DSP/compressor_chain.cpp`:

```cpp
#include "compressor_chain.h"
#include <cmath>
#include <algorithm>

CompressorChain::CompressorChain() {
  // Constructor initializes all atomic parameters to defaults
  // (defaults set in header initializers)
}

CompressorChain::~CompressorChain() {
  // No dynamic allocation; nothing to clean up
}

void CompressorChain::init(float sampleRate) {
  // Validate input
  if (sampleRate < 8000.0f || sampleRate > 192000.0f) {
    return; // Invalid sample rate, ignore
  }
  
  _sampleRate = sampleRate;
  
  // Reset state variables
  _inputPreviousFreq = -1.0;
  
  _levelDetection = 0.0;
  _levelFilteredSideChain = 0.0;
  _levelEnvFollow = 0.0;
  _levelAmplification = 1.0;
  _attackReason = AttackNone;
  
  _densityDetection = 0.0;
  _densitySideChain = 0.0;
  _densityEnvFollow = 0.0;
  _densityAmplification = 1.0;
  
  // Reset filters
  _levelHighpassInputLeft.reset();
  _levelHighpassInputRight.reset();
  _levelHighpassSidechain.reset();
  
  // Initialize time-based coefficients (attack/release)
  recalculateTimeCoefficients();
}

void CompressorChain::recalculateTimeCoefficients() {
  // This is called whenever sample rate changes or release/attack parameters change
  // Placeholder; will be implemented in Task 1.5
  // TODO: Implement attack/release coefficient calculations
}

// Setter methods (implementation continues below)

void CompressorChain::setInputGain(float gainDb) {
  _paramInputGain.store(gainDb, std::memory_order_relaxed);
}

void CompressorChain::setInputLoCut(float freqHz) {
  _paramInputLoCut.store(freqHz, std::memory_order_relaxed);
}

void CompressorChain::setInputSaturate(float amount) {
  _paramInputSaturate.store(amount, std::memory_order_relaxed);
}

void CompressorChain::setLevelThreshold(float dbThreshold) {
  _paramLevelThreshold.store(dbThreshold, std::memory_order_relaxed);
}

void CompressorChain::setLevelRatio(float ratio) {
  _paramLevelRatio.store(ratio, std::memory_order_relaxed);
}

void CompressorChain::setLevelAttack(float attackMs) {
  _paramLevelAttack.store(attackMs, std::memory_order_relaxed);
  recalculateTimeCoefficients(); // Attack changes, recompute coefficients
}

void CompressorChain::setLevelRelease(float releaseMs) {
  _paramLevelRelease.store(releaseMs, std::memory_order_relaxed);
  recalculateTimeCoefficients(); // Release changes, recompute coefficients
}

void CompressorChain::setLevelLoCut(bool enabled) {
  _paramLevelLoCut.store(enabled ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void CompressorChain::setLevelTubeGain(bool enabled) {
  _paramLevelTubeGain.store(enabled ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void CompressorChain::setLevelFeedback(bool feedbackMode) {
  _paramLevelFeedback.store(feedbackMode ? 1.0f : 0.0f, std::memory_order_relaxed);
}

void CompressorChain::setDensityThreshold(float dbThreshold) {
  _paramDensityThreshold.store(dbThreshold, std::memory_order_relaxed);
}

void CompressorChain::setDensityRatio(float ratio) {
  _paramDensityRatio.store(ratio, std::memory_order_relaxed);
}

void CompressorChain::setDensityAttack(float attackMs) {
  _paramDensityAttack.store(attackMs, std::memory_order_relaxed);
  recalculateTimeCoefficients();
}

void CompressorChain::setDensityRelease(float releaseMs) {
  _paramDensityRelease.store(releaseMs, std::memory_order_relaxed);
  recalculateTimeCoefficients();
}

CompressorChain::MeterData CompressorChain::getMeterData() const {
  MeterData data;
  data.inputGain = _meterInputGain.load(std::memory_order_relaxed);
  data.levelReduction = _meterLevelReduction.load(std::memory_order_relaxed);
  data.densityReduction = _meterDensityReduction.load(std::memory_order_relaxed);
  return data;
}

void CompressorChain::process(const float* inL, const float* inR,
                               float* outL, float* outR,
                               int numSamples) {
  // TODO: Implement main audio processing (port from PluginProcessor.cpp)
  // For now, pass through input to output
  if (inL != outL) {
    std::copy(inL, inL + numSamples, outL);
  }
  if (inR != outR) {
    std::copy(inR, inR + numSamples, outR);
  }
}
```

**Acceptance:**
- [ ] All setter methods implemented (14 total)
- [ ] `init()` resets all state variables
- [ ] `getMeterData()` returns current meter state
- [ ] Code compiles without warnings
- [ ] No JUCE includes

---

### Task 1.4: Port Input Stage Processing

**What:** Implement input gain, saturation, and high-pass filter  
**Duration:** 4–6 hours  
**Reference:** Lines 252–290 of original `PluginProcessor.cpp`

**Steps:**

1. **Add placeholder for IIR filter processing** in `compressor_chain.cpp`:
   - For now, skip actual filter implementation (Phase 2)
   - Add comment: `// TODO: Replace with custom IIR filter (Phase 2)`

2. **Implement input gain application** (dB to linear):
   ```cpp
   // Within process() method
   float localInputGain = pow(10.0f, *_paramInputGain / 20.0f);
   float inputL = inL[i] * localInputGain;
   float inputR = inR[i] * localInputGain;
   ```

3. **Implement input saturation** (soft-clipping for negative samples):
   ```cpp
   double inSaturation = *_paramInputSaturate * 10.0 + 1.0;
   if (inSaturation > 1.01) {
     float divisor = 20.0f / inSaturation;
     if (inputL < 0.0f) {
       inputL = (1.0f / (1.0f - (inputL / divisor)) - 1.0f) * divisor;
       if (inputL > 4.0f) inputL = 4.0f;
     }
     // ... same for inputR
   }
   ```

4. **Create unit test for input stage:**
   ```cpp
   // In Tests/dsp_tests.cpp
   TEST_CASE("Input stage: gain application") {
     CompressorChain dsp;
     dsp.init(44100);
     dsp.setInputGain(6.0f); // +6 dB = 2x amplification
     
     float inL[1] = {0.5f}, inR[1] = {0.5f};
     float outL[1], outR[1];
     dsp.process(inL, inR, outL, outR, 1);
     
     // Output should be ~2x input
     REQUIRE(std::abs(outL[0] - 1.0f) < 0.01f);
     REQUIRE(std::abs(outR[0] - 1.0f) < 0.01f);
   }
   ```

**Acceptance:**
- [ ] Input gain correctly converts dB to linear
- [ ] Saturation applies only to negative samples
- [ ] Unit test passes
- [ ] No compiler warnings

---

### Task 1.5: Port Level Compressor Logic

**What:** Implement the complex level compression algorithm (lines 320–696 of original)  
**Duration:** 8–12 hours  
**Complexity:** HIGH

**Caution:** This is the most complex part of the DSP. Break into sub-tasks:

#### 1.5.1 Sidechain Detection & Envelope Following

Extract the signal level detection and envelope following logic:

```cpp
// Within process() loop, after input stage:
float absSidechainInput = fabs(outputL) + fabs(outputR);
double filterDetectionAttack = feedbackMode ? 0.9984 : levelAttack;
absLevelDetection = (filterDetectionAttack * absLevelDetection) 
                  + ((1.0 - filterDetectionAttack) * absSidechainInput);

// Envelope follower pull-up/release
if (absLevelDetection > absLevelEnvFollow) {
  absLevelEnvFollow = (0.8 * absLevelEnvFollow) + (0.2 * absLevelDetection);
} else {
  absLevelEnvFollow = (levelRelease * absLevelEnvFollow);
}
```

#### 1.5.2 Compression Curve Calculation

Implement hard-knee and Vari-Mu curves:

```cpp
float dbLevelEnvFollow = (absLevelEnvFollow >= 0.000001) 
                        ? (log10(absLevelEnvFollow) * 20.0f)
                        : -120.0f;

float absTargetLevel;

if (levelRatio < (10.0f - 0.001f)) {
  // Hard-knee
  if (dbLevelEnvFollow > dbLevelThreshold) {
    float outputLevel = dbLevelThreshold 
                      + (dbLevelEnvFollow - dbLevelThreshold) / levelRatio;
    absTargetLevel = pow(10.0f, outputLevel / 20.0f);
  } else {
    absTargetLevel = absLevelEnvFollow;
  }
} else {
  // Vari-Mu (atan curve)
  absTargetLevel = atan(absLevelEnvFollow / absLevelThreshold) * absLevelThreshold;
}
```

#### 1.5.3 Attack/Release & Feedback Control

Implement the complex feedback vs. feedforward logic (lines 558–695):

```cpp
if (feedbackMode) {
  // Feedback mode: amplification rises with release, drops with attack
  if (absLevelDetection > absTargetLevel) {
    float absTargetAmplification = absTargetLevel / absLevelDetection;
    if (absTargetAmplification < absLevelAmplification) {
      float ratio = absTargetAmplification / absLevelAmplification;
      if (ratio < 0.01f) {
        absTargetAmplification = absLevelAmplification * 0.01f;
      }
      absLevelAmplification = (levelAttack * absLevelAmplification)
                            + ((1.0 - levelAttack) * absTargetAmplification);
    }
  }
} else {
  // Feedforward mode: instant response
  if (absLevelEnvFollow > 0.000001) {
    if (absTargetLevel > absLevelEnvFollow) {
      absLevelAmplification = absTargetLevel / absLevelEnvFollow;
    }
  } else {
    absLevelAmplification = 1.0;
  }
}
```

#### 1.5.4 Tube Saturation

Apply soft-clipping when tube gain is enabled:

```cpp
if (levelTubeGain) {
  if (outputL < 1.0f && outputL > -1.0f) {
    outputL = (1.0f / (1.0f - 1.0f/3.0f)) 
            * (outputL - (1.0f/3.0f) * outputL * outputL * outputL);
  } else {
    outputL = (outputL > 0.0f) ? 1.0f : -1.0f;
  }
  // ... same for outputR
}
```

#### 1.5.5 Unit Tests

```cpp
TEST_CASE("Level compressor: basic threshold") {
  CompressorChain dsp;
  dsp.init(44100);
  dsp.setLevelThreshold(-10.0f); // dB
  dsp.setLevelRatio(2.5f);
  dsp.setLevelAttack(10.0f);
  dsp.setLevelRelease(300.0f);
  
  // Process a burst of signal 20 dB above threshold
  // Expected: reduction of approximately 6 dB (20 - (-10)) / 2.5
  float signal[441] = {}; // 10 ms at 44.1 kHz
  std::fill(signal, signal + 441, 0.1f); // -20 dB
  
  float outL[441], outR[441];
  dsp.process(signal, signal, outL, outR, 441);
  
  // Verify meter shows reduction
  auto meter = dsp.getMeterData();
  REQUIRE(meter.levelReduction < 1.0f); // Compressed
}

TEST_CASE("Level compressor: Vari-Mu mode") {
  CompressorChain dsp;
  dsp.init(44100);
  dsp.setLevelRatio(9.99f); // Activate Vari-Mu
  // ... process and verify atan curve behavior
}
```

**Acceptance:**
- [ ] Sidechain detection & envelope following work correctly
- [ ] Hard-knee compression curve verified
- [ ] Vari-Mu (atan) curve verified
- [ ] Feedback vs. feedforward modes produce different results
- [ ] Tube saturation applies only when enabled
- [ ] Unit tests pass
- [ ] Audio output matches original JUCE version (RMS < 0.01 dB error)

---

### Task 1.6: Port Density Compressor Logic

**What:** Implement the fast density limiter (lines 752–849)  
**Duration:** 4–6 hours

**Steps:**

1. **Peak detection:**
   ```cpp
   if (densityRatio >= 1.001) {
     for (int i = 0; i < numSamples; ++i) {
       float sidechainInput1 = fabs(outL[i]);
       float sidechainInput2 = fabs(outR[i]);
       float absSidechainInput = std::max(sidechainInput1, sidechainInput2);
       densityDetection = absSidechainInput;
       
       // Envelope follower
       if (densityDetection > densityEnvFollow) {
         densityEnvFollow = (densityAttack * densityEnvFollow)
                          + ((1.0 - densityAttack) * densityDetection);
       } else {
         densityEnvFollow = (densityRelease * densityEnvFollow);
       }
     }
   }
   ```

2. **Compression curve:**
   ```cpp
   float outputDensity;
   if (densityRatio < (10.0f - 0.001f)) {
     outputDensity = densityThreshold
                   + (densityEnvFollow - densityThreshold) / densityRatio;
   } else {
     // Limiter: hard ceiling
     outputDensity = densityThreshold;
   }
   
   if (densityEnvFollow > 0.000001) {
     densityAmplification = outputDensity / densityEnvFollow;
   } else {
     densityAmplification = 1.0;
   }
   ```

3. **Unit tests:**
   ```cpp
   TEST_CASE("Density compressor: limiter mode") {
     CompressorChain dsp;
     dsp.init(44100);
     dsp.setDensityRatio(9.99f); // Limiter
     dsp.setDensityThreshold(-10.0f);
     // ... process and verify hard ceiling behavior
   }
   ```

**Acceptance:**
- [ ] Peak detection works correctly
- [ ] Hard-knee limiting curve verified
- [ ] Limiter mode verified
- [ ] Unit tests pass
- [ ] Output matches original

---

### Task 1.7: Implement Metering & State Updates

**What:** Update meter data atomically after each block  
**Duration:** 2–3 hours

**Steps:**

1. **Accumulate meter data during processing:**
   ```cpp
   double inputGainAccumulator = 0.0;
   double levelReductionMinimum = 1.0;
   double densityReductionMinimum = 1.0;
   
   for (int i = 0; i < numSamples; ++i) {
     // ... process sample
     inputGainAccumulator += input * input;
     levelReductionMinimum = std::min(levelReductionMinimum, absLevelAmplification);
     densityReductionMinimum = std::min(densityReductionMinimum, densityAmplification);
   }
   
   // RMS of input
   float inputGainRMS = sqrt(inputGainAccumulator / numSamples);
   
   // Update meters atomically
   _meterInputGain.store(inputGainRMS, std::memory_order_relaxed);
   _meterLevelReduction.store(levelReductionMinimum, std::memory_order_relaxed);
   _meterDensityReduction.store(densityReductionMinimum, std::memory_order_relaxed);
   ```

2. **Save state for next block:**
   ```cpp
   // At end of process()
   _levelDetection = absLevelDetection;
   _levelEnvFollow = absLevelEnvFollow;
   _levelAmplification = absLevelAmplification;
   _attackReason = attackReason;
   // ... etc for all state variables
   ```

**Acceptance:**
- [ ] Meter data updates correctly
- [ ] State persists between blocks
- [ ] Atomic operations use `memory_order_relaxed`

---

### Task 1.8: Write Comprehensive Tests

**What:** Create unit test suite for all DSP components  
**Duration:** 6–8 hours  
**File:** `Tests/dsp_tests.cpp`

**Test Categories:**

1. **Initialization & State**
   - Sample rates 44.1, 48, 96, 192 kHz
   - Parameter setter/getter round-trip
   - State reset on init()

2. **Input Stage**
   - Gain conversion accuracy
   - Saturation nonlinearity
   - High-pass filter (once Phase 2 complete)

3. **Level Compressor**
   - Hard-knee response
   - Vari-Mu (atan) response
   - Attack/release timing
   - Feedback vs. feedforward modes
   - Tube saturation

4. **Density Compressor**
   - Peak detection accuracy
   - Limiting ceiling
   - Fast attack behavior

5. **Edge Cases**
   - Silence (all zeros)
   - Extreme parameters
   - Sample-rate changes mid-stream
   - Very large/small amplifications

6. **Metering**
   - Meter accuracy vs. calculated reduction
   - Atomic meter updates

**Structure:**

```cpp
#include "catch.hpp"
#include "compressor_chain.h"
#include <cmath>
#include <algorithm>

TEST_CASE("CompressorChain initialization") {
  CompressorChain dsp;
  dsp.init(44100);
  REQUIRE(dsp.getSampleRate() == 44100);
  
  auto meter = dsp.getMeterData();
  REQUIRE(meter.inputGain == Catch::Approx(0.0f));
  REQUIRE(meter.levelReduction == Catch::Approx(1.0f));
}

TEST_CASE("CompressorChain parameter setters") {
  CompressorChain dsp;
  dsp.init(44100);
  
  dsp.setInputGain(6.0f);
  dsp.setLevelThreshold(-10.0f);
  dsp.setLevelRatio(4.0f);
  // ... no assertion; just verify no crash
}

// ... additional tests
```

**Acceptance:**
- [ ] 20+ test cases
- [ ] All tests pass
- [ ] Code coverage >90% for new classes

---

### Task 1.9: Code Review & Documentation

**What:** Final review, documentation, cleanup  
**Duration:** 4–6 hours

**Steps:**

1. **Run compiler checks:**
   ```bash
   clang-tidy Source/DSP/compressor_chain.cpp -checks=-*,readability-*,modernize-*
   ```

2. **Add comprehensive comments:**
   - Algorithm explanation for sidechain detection
   - Attack/release coefficient calculations
   - Vari-Mu curve justification
   - Magic numbers explained

3. **Create README:**
   ```markdown
   # CompressorChain DSP Library
   
   Self-contained audio compressor implementation.
   
   ## API Example
   
   ```cpp
   CompressorChain dsp;
   dsp.init(44100);
   dsp.setInputGain(6.0f);
   dsp.setLevelThreshold(-10.0f);
   dsp.setLevelRatio(2.5f);
   
   float inL[512], inR[512];
   float outL[512], outR[512];
   // ... fill inputs
   
   dsp.process(inL, inR, outL, outR, 512);
   auto meters = dsp.getMeterData();
   ```
   
   ## Parameter Ranges
   
   [Document all 14 parameters with ranges and defaults]
   ```

4. **Create TESTING.md:**
   ```markdown
   # DSP Testing Guide
   
   ## Unit Tests
   ```bash
   cd build && ctest
   ```
   
   ## Regression Testing
   
   Compare audio output with original JUCE build:
   ...
   ```

5. **Verify no JUCE dependencies:**
   ```bash
   grep -r "Juce" Source/DSP/ || echo "No JUCE dependencies found ✓"
   grep -r "#include <juce" Source/DSP/ || echo "No JUCE includes found ✓"
   ```

**Acceptance:**
- [ ] Zero compiler warnings
- [ ] Code formatted consistently
- [ ] README and TESTING.md complete
- [ ] No JUCE dependencies
- [ ] All tests pass

---

## Integration Testing

### Golden Audio File Comparison

After Phase 1 completion, compare output with original JUCE build:

1. **Generate test signal:**
   - 10 seconds of pink noise (or music sample)
   - 44.1 kHz sample rate

2. **Run through both versions:**
   - Original JUCE plugin
   - New CompressorChain DSP

3. **Compare RMS error:**
   ```
   RMS_error = sqrt(mean((output_new - output_original)^2))
   ```
   - Target: RMS_error < 0.01 dB

4. **Repeat at 48 kHz and 96 kHz** (Phase 3)

### Tools Needed

- **Audacity:** Generate test signals, visual inspection
- **Python/SciPy:** RMS error calculation
- **ffmpeg:** Audio format conversion

---

## Deliverables Checklist

- [ ] `Source/DSP/compressor_chain.h` (complete, documented)
- [ ] `Source/DSP/compressor_chain.cpp` (complete implementation)
- [ ] `CMakeLists.txt` (all 3: root, DSP/, Tests/)
- [ ] `Tests/dsp_tests.cpp` (20+ test cases)
- [ ] `Tests/catch.hpp` (test framework)
- [ ] `Source/DSP/README.md` (API documentation)
- [ ] `Source/DSP/TESTING.md` (test procedures)
- [ ] Git commit with Phase 1 complete summary
- [ ] All tests passing on Windows, macOS, Linux

---

## Success Criteria

Phase 1 is **complete** when:

✅ CompressorChain compiles without warnings  
✅ All 14 parameters have setters + atomic storage  
✅ process() produces audio output  
✅ No JUCE #includes in .h or .cpp  
✅ Unit tests: 20+ cases, all passing  
✅ Audio output matches original (RMS < 0.01 dB at 44.1 kHz)  
✅ Code passes Clang-Tidy static analysis  
✅ Documentation complete (README + TESTING guide)  
✅ Git history clean (logical commits)  

---

## Next Steps (When Complete)

1. Merge Phase 1 branch to `main`
2. Update [IMPLEMENTATION_PLAN.md](./IMPLEMENTATION_PLAN.md): Phase 1 → ✅ Completed
3. Begin [PHASE_2_IIR_REPLACEMENT.md](./PHASE_2_IIR_REPLACEMENT.md) (IIR filter extraction)

---

## Estimated Timeline

- Week 1: Tasks 1.1–1.4 (structure + constructor + input stage)
- Week 2: Tasks 1.5–1.6 (complex compression logic)
- Week 3: Tasks 1.7–1.9 (metering, tests, review)

**Total: 2–3 weeks**

---

## Common Pitfalls & Solutions

| Issue | Solution |
|-------|----------|
| IIRFilter JUCE dependency remains | Phase 2 specifically handles this; use placeholder for now |
| Audio output mismatch | Enable all #if 1 code sections; verify parameter ranges |
| State not persisting | Ensure all state variables saved at end of process() |
| Tests fail at different sample rates | Phase 3 fixes sample-rate handling |
| Memory leaks detected | Check std::unique_ptr usage; Valgrind frequently |

---

## Questions & Escalations

**If stuck:** Open GitHub issue with `[PHASE_1]` tag and:
- Code snippet
- Expected vs. actual behavior
- Steps to reproduce

**Example:**
```
[PHASE_1] CompressorChain output doesn't match original

- Silence input produces non-zero output
- Occurs only with feedback mode enabled
- Reproducible with `setLevelFeedback(true); process(...)`

Attached: test_case.cpp, audio_diff.wav
```
