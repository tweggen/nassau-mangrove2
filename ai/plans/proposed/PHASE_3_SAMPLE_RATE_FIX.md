# Phase 3: Sample Rate Handling

**Objective:** Implement dynamic sample-rate coefficients to replace hardcoded 44.1 kHz assumptions.

**Duration:** 1–2 weeks  
**Dependencies:** Phase 1 (CompressorChain), Phase 2 (IIRFilter)  
**Produces:** Updated `compressor_chain.cpp` with sample-rate-aware calculations  
**Tests Required:** Multi-sample-rate validation (44.1, 48, 96, 192 kHz)  

---

## Overview

The original code has multiple hardcoded 44.1 kHz references in time-based calculations:

```cpp
// Line 174 (PluginProcessor.cpp)
float release = pow(0.5, 1. / ((*_paramLevelRelease * relSoundGoodAdjustment / 1000.) * 44100.));

// Line 278
float localLevelAttack = pow(0.5, 1. / ((*_paramLevelAttack * attSoundGoodAdjustment / 1000.)*44100.));

// Line 354–355
_levelHighpassInputLeft.setCoefficients(IIRCoefficients::makeHighPass(44100., localInputLoCutFrequency, 0.8));
```

Modern DAWs use 48 kHz (video), 96 kHz (mastering), or 192 kHz (high-res audio). This phase makes the DSP sample-rate agnostic.

---

## Acceptance Criteria

Phase 3 is **complete** when:

- [ ] `CompressorChain::init()` stores sample rate in private member
- [ ] All attack/release coefficients computed dynamically in `recalculateTimeCoefficients()`
- [ ] IIR filters configured with stored sample rate (not hardcoded)
- [ ] Parameter changes trigger coefficient recalculation
- [ ] Audio output is identical across 44.1, 48, 96, 192 kHz (RMS < 0.01 dB)
- [ ] Attack/release timing is consistent (e.g., 10 ms attack takes same real time at any sample rate)
- [ ] Unit tests validate behavior at multiple sample rates
- [ ] Code passes compiler checks (no warnings)

---

## Detailed Tasks

### Task 3.1: Audit All Sample-Rate Dependencies

**What:** Identify and document all hardcoded sample-rate references  
**Duration:** 2–3 hours  
**No coding required**

**Steps:**

1. **Search original code:**
   ```bash
   grep -n "44100" Source/PluginProcessor.cpp
   grep -n "44100" Source/DSP/compressor_chain.cpp
   grep -n "44.1" Source/PluginProcessor.cpp
   ```

2. **Categorize findings:**

   | Location | Type | Current Code | Fix Required |
   |----------|------|--------------|--------------|
   | Line 174 | Release coeff | `pow(0.5, 1/(x*44100))` | Dynamic |
   | Line 278 | Attack coeff | `pow(0.5, 1/(x*44100))` | Dynamic |
   | Line 354 | HPF left | `makeHighPass(44100, ...)` | Dynamic |
   | Line 355 | HPF right | `makeHighPass(44100, ...)` | Dynamic |
   | Line 446 | HPF sidechain | `makeHighPass(44100, ...)` | Dynamic |
   | Line 748 | Density attack | `pow(0.5, 1/(x*44100))` | Dynamic |
   | Line 750 | Density release | `pow(0.5, 1/(x*44100))` | Dynamic |

3. **Understand timing semantics:**
   - Attack/Release are in milliseconds (user parameter)
   - Should represent real-world time (10 ms = 0.01 seconds)
   - Must scale correctly with sample rate
   - Example: 10 ms at 44.1 kHz = 441 samples; at 48 kHz = 480 samples

4. **Document current vs. expected behavior:**
   ```
   Current (broken at 96 kHz):
   - 10 ms attack at 44.1 kHz: 441 samples (0.01 s ✓)
   - 10 ms attack at 96 kHz: 441 samples (0.0046 s ✗) — should be 960
   
   Expected (fixed):
   - 10 ms attack at any sample rate: sampleRate * 0.01 samples
   ```

**Acceptance:**
- [ ] All 7 hardcoded references documented
- [ ] Understand difference between "per-block" and "per-sample" timing
- [ ] Know which coefficients need recalculation

---

### Task 3.2: Store Sample Rate in CompressorChain

**What:** Add sample-rate storage and update `init()` method  
**Duration:** 1 hour  
**Files to Modify:** `Source/DSP/compressor_chain.h`, `.cpp`

**Header Changes:**

In `compressor_chain.h`, add to private section:

```cpp
private:
  // ===== Configuration =====
  float _sampleRate = 44100.0f;  // DEFAULT: Will be set in init()
  
  // ... existing members
```

**Implementation Changes:**

Update `compressor_chain.cpp`:

```cpp
void CompressorChain::init(float sampleRate) {
  // Validate input
  if (sampleRate < 8000.0f || sampleRate > 192000.0f) {
    // Invalid sample rate; use previous or default
    return;
  }
  
  // Store sample rate
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
  
  // Recalculate time-dependent coefficients
  recalculateTimeCoefficients();
}

float CompressorChain::getSampleRate() const {
  return _sampleRate;
}
```

**Add getter method** to header (public section):

```cpp
/// Query the sample rate this instance is configured for.
float getSampleRate() const { return _sampleRate; }
```

**Acceptance:**
- [ ] `_sampleRate` member variable added
- [ ] `init()` stores sample rate
- [ ] `getSampleRate()` getter added
- [ ] Compiles without warnings

---

### Task 3.3: Implement Dynamic Attack/Release Coefficient Calculation

**What:** Implement `recalculateTimeCoefficients()` method  
**Duration:** 3–4 hours  
**Files to Modify:** `Source/DSP/compressor_chain.cpp`

**Background:**

Attack and Release use exponential decay coefficients:

```
target_value = filter * previous_value + (1 - filter) * new_input

filter = e^(-1 / (time_constant * sample_rate))
       ≈ 0.5^(1 / (time_ms / 1000 * sample_rate))
       = 0.5^(1000 / (time_ms * sample_rate))
```

Example:
- 10 ms release at 44.1 kHz: 0.5^(1000/(10*44100)) = 0.5^0.0023 ≈ 0.9984
- 10 ms release at 96 kHz: 0.5^(1000/(10*96000)) = 0.5^0.0010 ≈ 0.9993

**Implementation:**

```cpp
void CompressorChain::recalculateTimeCoefficients() {
  if (_sampleRate < 8000.0f) {
    return; // Invalid sample rate
  }
  
  // ===== Level Compressor Attack/Release =====
  
  // Attack: 0 to 100 ms (default 10 ms)
  // Tuning constant (from original code): 0.1 (attack takes 10x longer than nominal)
  float attSoundGoodAdjustment = 0.1f;
  float levelAttackMs = _paramLevelAttack.load(std::memory_order_relaxed);
  float levelAttackTimeConstantMs = levelAttackMs * attSoundGoodAdjustment;
  
  // Coefficient: higher = slower attack (more previous state retained)
  // Using pow(0.5, 1/(time_ms/1000 * sample_rate))
  float levelAttackCoeff = powf(
    0.5f, 
    1000.0f / (levelAttackTimeConstantMs * _sampleRate)
  );
  
  // Release: 10 to 500 ms (default 300 ms)
  // Tuning constant: 1.0 (no adjustment)
  float relSoundGoodAdjustment = 1.0f;
  float levelReleaseMs = _paramLevelRelease.load(std::memory_order_relaxed);
  float levelReleaseTimeConstantMs = levelReleaseMs * relSoundGoodAdjustment;
  
  // Coefficient for standard (slow) release
  float levelReleaseCoeff = powf(
    0.5f,
    1000.0f / (levelReleaseTimeConstantMs * _sampleRate)
  );
  
  // Cache coefficients for use in process()
  // (These will be used during audio processing)
  // NOTE: These are not stored as members; they're recomputed per-block
  // if sample rate changes. For now, we print them for validation.
  
  #ifdef DEBUG_COEFFICIENTS
  printf("Level Attack coeff (%.1f ms at %.0f Hz): %.6f\n",
         levelAttackTimeConstantMs, _sampleRate, levelAttackCoeff);
  printf("Level Release coeff (%.1f ms at %.0f Hz): %.6f\n",
         levelReleaseTimeConstantMs, _sampleRate, levelReleaseCoeff);
  #endif
  
  // ===== Density Compressor Attack/Release =====
  
  // Density Attack: 0.001 to 100 ms (default 10 ms)
  float densityAttackMs = _paramDensityAttack.load(std::memory_order_relaxed);
  float densityAttackCoeff = (densityAttackMs < 0.0001f)
    ? 0.0f  // Instant attack
    : powf(
        0.5f,
        1000.0f / (densityAttackMs * _sampleRate)
      ) / 10.0f;  // Divide by 10 for faster behavior
  
  // Density Release: 10 to 2000 ms (default 300 ms)
  float densityReleaseMs = _paramDensityRelease.load(std::memory_order_relaxed);
  float densityReleaseCoeff = powf(
    0.5f,
    1000.0f / (densityReleaseMs * _sampleRate)
  );
  
  #ifdef DEBUG_COEFFICIENTS
  printf("Density Attack coeff (%.3f ms at %.0f Hz): %.6f\n",
         densityAttackMs, _sampleRate, densityAttackCoeff);
  printf("Density Release coeff (%.1f ms at %.0f Hz): %.6f\n",
         densityReleaseMs, _sampleRate, densityReleaseCoeff);
  #endif
}
```

**Issue:** These coefficients are computed but not stored (they change per-block if parameters change). We need to cache them in the state.

**Improved Approach:** Store coefficients as member variables:

Add to `compressor_chain.h` private section:

```cpp
private:
  // ===== Cached Coefficients (recalculated when parameters change) =====
  float _levelAttackCoeff = 0.0f;
  float _levelReleaseCoeff = 0.0f;
  float _densityAttackCoeff = 0.0f;
  float _densityReleaseCoeff = 0.0f;
```

Update `recalculateTimeCoefficients()` to cache:

```cpp
void CompressorChain::recalculateTimeCoefficients() {
  // ... same calculations as above ...
  
  // Cache coefficients for use in process()
  _levelAttackCoeff = levelAttackCoeff;
  _levelReleaseCoeff = levelReleaseCoeff;
  _densityAttackCoeff = densityAttackCoeff;
  _densityReleaseCoeff = densityReleaseCoeff;
}
```

**Acceptance:**
- [ ] Method compiles without warnings
- [ ] Coefficients computed correctly for multiple sample rates
- [ ] No NaN/Inf values
- [ ] Formulas match original code behavior

---

### Task 3.4: Update IIR Filter Initialization

**What:** Pass `_sampleRate` to IIR filter setup  
**Duration:** 1–2 hours  
**Files to Modify:** `Source/DSP/compressor_chain.cpp` (init() method)

**Current Code (hardcoded):**
```cpp
_levelHighpassInputLeft.setCoefficients(44100.0f, 200.0f, 0.8f);
_levelHighpassInputRight.setCoefficients(44100.0f, 200.0f, 0.8f);
_levelHighpassSidechain.setCoefficients(44100.0f, 200.0f, 0.8f);
```

**Updated Code:**
```cpp
void CompressorChain::init(float sampleRate) {
  // ... existing code ...
  
  _sampleRate = sampleRate;
  
  // ... state reset ...
  
  // Initialize filters with actual sample rate
  _levelHighpassInputLeft.setCoefficients(_sampleRate, 200.0f, 0.8f);
  _levelHighpassInputRight.setCoefficients(_sampleRate, 200.0f, 0.8f);
  _levelHighpassSidechain.setCoefficients(_sampleRate, 200.0f, 0.8f);
  
  // Recalculate time coefficients
  recalculateTimeCoefficients();
}
```

**In process() method**, when cutoff frequency changes:

```cpp
// Original (hardcoded):
if (localInputLoCutFrequency != _inputPreviousFreq) {
  _levelHighpassInputLeft.setCoefficients(44100., localInputLoCutFrequency, 0.8f);
  _levelHighpassInputRight.setCoefficients(44100., localInputLoCutFrequency, 0.8f);
}

// Updated:
if (localInputLoCutFrequency != _inputPreviousFreq) {
  _levelHighpassInputLeft.setCoefficients(_sampleRate, localInputLoCutFrequency, 0.8f);
  _levelHighpassInputRight.setCoefficients(_sampleRate, localInputLoCutFrequency, 0.8f);
  _inputPreviousFreq = localInputLoCutFrequency;
}
```

**Acceptance:**
- [ ] All three filter instances use `_sampleRate`
- [ ] No hardcoded 44100 left in filter setup
- [ ] Compiles without warnings

---

### Task 3.5: Update process() to Use Cached Coefficients

**What:** Use cached attack/release coefficients in audio processing loop  
**Duration:** 2–3 hours  
**Files to Modify:** `Source/DSP/compressor_chain.cpp` (process() method)

**Current (wrong):**
```cpp
// Inside process() loop
float attSoundGoodAdjustment = 0.1;
float localLevelAttack = pow(0.5, 1. / ((*_paramLevelAttack * attSoundGoodAdjustment / 1000.)*44100.));
```

**Updated (correct):**
```cpp
// At start of process() block (not in per-sample loop!)
float localLevelAttack = _levelAttackCoeff;  // Pre-calculated in recalculateTimeCoefficients()

// Then use in loop:
absLevelAmplification = (localLevelAttack * absLevelAmplification)
                      + ((1.0 - localLevelAttack) * absTargetAmplification);
```

**All occurrences to fix:**

1. Line 278 (approximate): `localLevelAttack` calculation
   - Replace with: `float localLevelAttack = _levelAttackCoeff;`

2. Line 306: `levelStandardRelease` calculation
   - Replace with: `float levelStandardRelease = _levelReleaseCoeff;`

3. Line 748: `localDensityAttack` calculation
   - Replace with: `float localDensityAttack = _densityAttackCoeff;`

4. Line 750: `localDensityRelease` calculation
   - Replace with: `float localDensityRelease = _densityReleaseCoeff;`

**Acceptance:**
- [ ] All hardcoded 44100 removed from process()
- [ ] Coefficients loaded from cached members
- [ ] No floating-point operations in hot path
- [ ] Compiles without warnings

---

### Task 3.6: Handle Parameter Changes (Attack/Release Updates)

**What:** Ensure coefficients recalculate when user changes attack/release parameters  
**Duration:** 1–2 hours  
**Files to Modify:** `Source/DSP/compressor_chain.cpp` (setter methods)

**Current Code:**
```cpp
void CompressorChain::setLevelAttack(float attackMs) {
  _paramLevelAttack.store(attackMs, std::memory_order_relaxed);
  // Coefficients not recalculated!
}
```

**Updated Code:**
```cpp
void CompressorChain::setLevelAttack(float attackMs) {
  _paramLevelAttack.store(attackMs, std::memory_order_relaxed);
  recalculateTimeCoefficients();  // Recalculate affected coefficients
}

void CompressorChain::setLevelRelease(float releaseMs) {
  _paramLevelRelease.store(releaseMs, std::memory_order_relaxed);
  recalculateTimeCoefficients();
}

void CompressorChain::setDensityAttack(float attackMs) {
  _paramDensityAttack.store(attackMs, std::memory_order_relaxed);
  recalculateTimeCoefficients();
}

void CompressorChain::setDensityRelease(float releaseMs) {
  _paramDensityRelease.store(releaseMs, std::memory_order_relaxed);
  recalculateTimeCoefficients();
}
```

**Acceptance:**
- [ ] All attack/release setters call `recalculateTimeCoefficients()`
- [ ] Changes take effect immediately in next process() call
- [ ] No thread safety issues (safe to call from UI thread)

---

### Task 3.7: Write Multi-Sample-Rate Tests

**What:** Validate behavior at different sample rates  
**Duration:** 4–6 hours  
**File:** `Tests/dsp_tests.cpp` (add tests)

**Test Strategy:**

1. **Consistency test:** Same input, different sample rates, same output amplitude

```cpp
TEST_CASE("CompressorChain: consistent behavior across sample rates") {
  std::vector<float> sampleRates = {44100.0f, 48000.0f, 96000.0f, 192000.0f};
  
  for (float fs : sampleRates) {
    CompressorChain dsp;
    dsp.init(fs);
    dsp.setInputGain(0.0f);
    dsp.setLevelThreshold(-10.0f);
    dsp.setLevelRatio(2.5f);
    dsp.setLevelAttack(10.0f);  // 10 ms attack
    dsp.setLevelRelease(300.0f); // 300 ms release
    
    // Generate 1 second of input
    int numSamples = (int)fs;
    std::vector<float> input(numSamples);
    std::fill(input.begin(), input.end(), 0.1f); // -20 dB
    
    std::vector<float> output(numSamples);
    dsp.process(input.data(), input.data(),
                output.data(), output.data(),
                numSamples);
    
    // Measure RMS
    float rms = measureRMS(output.data(), numSamples);
    
    // Store for comparison
    sampleRateResults[fs] = rms;
  }
  
  // All sample rates should produce similar output amplitude
  // (Allow 0.5 dB variation due to floating point precision)
  float baseRMS = sampleRateResults[44100.0f];
  for (auto [fs, rms] : sampleRateResults) {
    REQUIRE(std::abs(rms - baseRMS) < 0.05f); // < 0.5 dB difference
  }
}
```

2. **Attack/Release timing test:** Verify 10 ms attack takes same real-world time

```cpp
TEST_CASE("CompressorChain: attack timing is sample-rate independent") {
  std::vector<float> sampleRates = {44100.0f, 96000.0f};
  std::vector<int> attackSamples;
  
  for (float fs : sampleRates) {
    CompressorChain dsp;
    dsp.init(fs);
    dsp.setLevelAttack(10.0f); // 10 ms
    
    // Process 100 ms burst
    int numSamples = (int)(fs * 0.1f); // 100 ms
    std::vector<float> input(numSamples);
    std::fill(input.begin(), input.end(), 0.5f); // Constant signal
    
    std::vector<float> output(numSamples);
    dsp.process(input.data(), input.data(),
                output.data(), output.data(),
                numSamples);
    
    // Measure how many samples until compression reaches 90% of target
    // (This is attack time)
    int samplesUntilAttack = 0;
    for (int i = 0; i < numSamples; ++i) {
      if (output[i] < input[0] * 0.95f) { // 5% reduction
        samplesUntilAttack = i;
        break;
      }
    }
    
    attackSamples.push_back(samplesUntilAttack);
  }
  
  // Same attack time (in ms) at both sample rates
  float attackTimeMs_44k = (float)attackSamples[0] / 44100.0f * 1000.0f;
  float attackTimeMs_96k = (float)attackSamples[1] / 96000.0f * 1000.0f;
  
  REQUIRE(std::abs(attackTimeMs_44k - attackTimeMs_96k) < 1.0f); // < 1 ms difference
}
```

3. **Regression: Match Phase 2 output across sample rates**

```cpp
TEST_CASE("CompressorChain: output matches golden reference (multi-sample-rate)") {
  // Golden reference created at 44.1 kHz in Phase 2
  // Now verify at 48, 96 kHz, output RMS is similar
  
  // Load golden reference
  std::vector<float> goldenRef = loadWaveFile("Tests/fixtures/golden_phase2.wav");
  
  for (float fs : {48000.0f, 96000.0f, 192000.0f}) {
    // Resample golden reference to fs
    std::vector<float> resampledGolden = resample(goldenRef, 44100.0f, fs);
    
    // Process with CompressorChain
    CompressorChain dsp;
    dsp.init(fs);
    // ... set all parameters same as golden ...
    
    std::vector<float> output(resampledGolden.size());
    dsp.process(resampledGolden.data(), resampledGolden.data(),
                output.data(), output.data(),
                resampledGolden.size());
    
    // Compare RMS amplitude (should be similar to golden)
    float outputRMS = measureRMS(output.data(), output.size());
    float goldenRMS = measureRMS(resampledGolden.data(), resampledGolden.size());
    
    float rmsDiffDB = 20.0f * log10f(outputRMS / goldenRMS);
    REQUIRE(std::abs(rmsDiffDB) < 0.5f); // < 0.5 dB difference
  }
}
```

**Acceptance:**
- [ ] 3+ sample-rate tests written
- [ ] All tests pass
- [ ] Timing is sample-rate independent
- [ ] RMS amplitude consistent across rates

---

### Task 3.8: Regression Testing

**What:** Verify Phase 2 output unchanged  
**Duration:** 3–4 hours

**Procedure:**

1. **Create new test audio using Phase 2 build:**
   ```bash
   cd build_phase2
   ./TestRunner --generate-golden-48k
   ```

2. **Rebuild with Phase 3 changes:**
   ```bash
   cd build_phase3
   cmake ..
   cmake --build .
   ```

3. **Compare outputs:**
   ```bash
   python3 compare_audio.py \
     ../build_phase2/golden_48k.wav \
     output_phase3_48k.wav
   ```

4. **Expected:** RMS error < 0.01 dB

**Tools Needed:**
- Python script: `compare_audio.py` (calculates RMS error)
- Reference audio: `golden_phase2_44k.wav` (from Phase 2)

**Python Script:**
```python
#!/usr/bin/env python3
import scipy.io.wavfile as wav
import numpy as np
import sys

def rms_error(signal1, signal2):
    """Calculate RMS error between two signals."""
    diff = signal1 - signal2
    rms = np.sqrt(np.mean(diff**2))
    rms_db = 20 * np.log10(rms + 1e-10)
    return rms, rms_db

if __name__ == "__main__":
    fs1, audio1 = wav.read(sys.argv[1])
    fs2, audio2 = wav.read(sys.argv[2])
    
    # Both should have same sample rate
    assert fs1 == fs2, f"Sample rate mismatch: {fs1} vs {fs2}"
    
    # Ensure same length
    min_len = min(len(audio1), len(audio2))
    audio1 = audio1[:min_len]
    audio2 = audio2[:min_len]
    
    rms, rms_db = rms_error(audio1.astype(float), audio2.astype(float))
    print(f"RMS Error: {rms:.6f} ({rms_db:.2f} dB)")
    
    if rms_db < -60:
        print("✓ PASS: Negligible difference")
    elif rms_db > -40:
        print("✗ FAIL: Significant difference")
        sys.exit(1)
    else:
        print("⚠ WARNING: Minor difference (acceptable)")
```

**Acceptance:**
- [ ] RMS error < 0.01 dB on all test signals
- [ ] Visually inspect waveforms (no obvious changes)
- [ ] Frequency spectrum similar (within ±1 dB)

---

### Task 3.9: Code Review & Documentation

**What:** Final review and documentation  
**Duration:** 2–3 hours

**Checklist:**

- [ ] All 44.1 kHz hardcodes removed
- [ ] Sample rate stored and passed to all time-dependent calculations
- [ ] No compiler warnings (all platforms)
- [ ] Comments explain timing formulas
- [ ] Public getter `getSampleRate()` added
- [ ] Setter methods trigger recalculation
- [ ] Tests pass at 44.1, 48, 96, 192 kHz
- [ ] Regression tests show no output change

**Documentation to Add:**

In `compressor_chain.h`:

```cpp
/**
 * Timing Coefficients
 * 
 * All time-dependent calculations (attack, release, envelope following)
 * use exponential decay with sample-rate dependent coefficients.
 * 
 * Coefficient formula:
 *   coeff = 0.5^(1000 / (time_ms * sample_rate))
 * 
 * Where:
 *   - time_ms: user parameter (10 to 500 ms typical)
 *   - sample_rate: Hz (set in init())
 *   - coeff: dimensionless (0 to 1)
 * 
 * Example (attack, 10 ms at 44.1 kHz):
 *   coeff = 0.5^(1000 / (10 * 44100)) = 0.9984
 *   
 *   Each sample: output = coeff * previous + (1-coeff) * input
 *   Result: reaches 95% of target in ~100 samples = 2.3 ms
 *   (Note: includes tuning factor of 0.1, so real attack is 10× slower)
 * 
 * Sample rate changes:
 *   Call init(newSampleRate) when host changes sample rate.
 *   This recalculates all coefficients for the new rate.
 */
```

**Acceptance:**
- [ ] All documentation complete
- [ ] Comments explain algorithm clearly
- [ ] Code follows naming conventions
- [ ] No unexplained magic numbers

---

## Deliverables Checklist

- [ ] `Source/DSP/compressor_chain.h` (updated with sample rate support)
- [ ] `Source/DSP/compressor_chain.cpp` (sample-rate-aware implementation)
- [ ] `Tests/dsp_tests.cpp` (added 3+ multi-sample-rate tests)
- [ ] `compare_audio.py` (regression testing script)
- [ ] `Tests/fixtures/golden_phase3_*.wav` (reference files at 48k, 96k)
- [ ] Git commit with Phase 3 summary

---

## Success Criteria

Phase 3 is **complete** when:

✅ Sample rate stored and used in all calculations  
✅ Attack/release coefficients computed dynamically  
✅ IIR filters configured with correct sample rate  
✅ Parameter changes trigger recalculation  
✅ Audio output identical at 44.1, 48, 96, 192 kHz (RMS < 0.01 dB)  
✅ Attack timing sample-rate independent  
✅ No hardcoded 44100 remaining  
✅ Unit tests: 3+ pass  
✅ Regression tests: golden reference match  
✅ Code passes compiler checks  
✅ Documentation complete  

---

## Next Steps (When Complete)

1. Merge Phase 3 branch to `main`
2. Update [IMPLEMENTATION_PLAN.md](./IMPLEMENTATION_PLAN.md): Phase 3 → ✅ Completed
3. Begin [PHASE_4_VST3_WRAPPER.md](./PHASE_4_VST3_WRAPPER.md) (VST 3 integration)

---

## Timeline

- Day 1: Task 3.1 (audit dependencies)
- Days 2: Task 3.2–3.3 (storage + coefficient calculation)
- Day 3: Task 3.4–3.5 (filter setup + process updates)
- Day 4: Task 3.6 (parameter change handling)
- Days 5–6: Task 3.7 (tests)
- Day 7: Task 3.8 (regression testing)
- Day 8: Task 3.9 (review)

**Total: 1–2 weeks**

---

## Common Pitfalls

| Issue | Cause | Solution |
|-------|-------|----------|
| Coefficients wrong at 96 kHz | Forgot to update formula | Verify pow() uses `_sampleRate`, not 44100 |
| Parameter changes don't take effect | Setter doesn't recalculate | Add `recalculateTimeCoefficients()` calls |
| Output sound different | Coefficient scaling wrong | Check formula: `1000 / (time_ms * fs)` |
| Tests fail at 192 kHz | Floating-point precision | Use double for intermediate calculations |
| Can't compile | recalculateTimeCoefficients() called in wrong place | Must be called from init() and setters only |
