# Phase 2: IIR Filter Replacement

**Objective:** Implement custom Butterworth high-pass filter to eliminate JUCE `IIRFilter` dependency.

**Duration:** 1–2 weeks  
**Dependencies:** Phase 1 (CompressorChain class)  
**Produces:** Custom `Source/DSP/iir_filter.h` and `.cpp`  
**Tests Required:** Frequency response validation, coefficient accuracy  

---

## Overview

The CompressorChain uses three high-pass filter instances:
- Input left channel (for input LoCut)
- Input right channel (for input LoCut)
- Sidechain (for level compressor feedback)

Currently these use JUCE's `IIRFilter` class, which is a dependency we need to eliminate. This phase implements a custom Butterworth high-pass filter with the same behavior.

### Filter Specification

- **Type:** Second-order (2-pole) Butterworth high-pass IIR filter
- **Coefficients:** Standard biquad form (5 coefficients)
- **Input:** Cutoff frequency (Hz), sample rate (Hz)
- **State:** Per-channel filter memory (2 samples for 2nd order)
- **Processing:** Single-sample processing for real-time efficiency

---

## Acceptance Criteria

Phase 2 is **complete** when:

- [ ] `IIRFilter` class compiles without warnings
- [ ] Frequency response matches JUCE's IIRFilter within ±0.5 dB at cutoff
- [ ] Filter is stable across all valid frequency ranges
- [ ] State reset works correctly
- [ ] Coefficient calculation is mathematically correct
- [ ] Unit tests validate response at multiple frequencies
- [ ] No NaN/Inf issues with extreme parameters
- [ ] Code passes static analysis (Clang-Tidy)

---

## Detailed Tasks

### Task 2.1: Understand Butterworth Filter Mathematics

**What:** Review IIR filter theory and coefficient calculations  
**Duration:** 2–4 hours  
**No coding required**

**Key Concepts:**

1. **Biquad Form:**
   ```
   y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
   ```
   Where:
   - `b0, b1, b2` = feedforward coefficients
   - `a1, a2` = feedback coefficients (a0 normalized to 1.0)

2. **Butterworth Characteristics:**
   - Maximally flat passband (no ripple)
   - -3dB point at cutoff frequency
   - Roll-off: -12 dB/octave for 2nd order
   - Phase shift: 90° at cutoff

3. **Coefficient Calculation (Normalized Frequency Method):**
   ```
   // Normalized frequency (0 to π)
   wc = 2 * π * fc / fs
   
   // Butterworth denominator
   C = 1 / tan(wc/2)
   
   // High-pass coefficients
   b0 = 1 / (1 + sqrt(2)*C + C²)
   b1 = -2 * b0
   b2 = b0
   a1 = 2*(1 - C²) / (1 + sqrt(2)*C + C²)
   a2 = (1 - sqrt(2)*C + C²) / (1 + sqrt(2)*C + C²)
   ```

4. **Numerical Stability:**
   - Avoid direct trigonometric calculations if possible
   - Use pre-computed tables for common frequencies
   - Check for NaN/Inf in filter state

**Reference:**
- "Audio DSP Filter Design" — RBJ (Robert Bristow-Johnson)
- https://www.earlevel.com/main/2013/10/18/sos-iir-filter-design/

**Deliverable:** Understand the math well enough to implement without reference during coding.

---

### Task 2.2: Create IIRFilter Header

**What:** Define `IIRFilter` class interface  
**Duration:** 2–3 hours  
**File:** `Source/DSP/iir_filter.h`

**Requirements:**
- Simple API (matches JUCE's `IIRFilter` where possible)
- Coefficient calculation method
- Single-sample processing
- State management

**Implementation:**

```cpp
#pragma once

#include <cmath>
#include <cstring>

/**
 * Second-order Butterworth high-pass IIR filter.
 * 
 * Provides -3dB cutoff at specified frequency with minimal phase distortion.
 * Optimized for real-time audio processing.
 * 
 * Thread safety: Not thread-safe. Each thread must have its own instance,
 * or external synchronization must be used.
 */
class IIRFilter {
public:
  IIRFilter();
  ~IIRFilter() = default;
  
  /**
   * Reset filter state (x and y history).
   * Call this when changing sample rate or starting a new block.
   */
  void reset();
  
  /**
   * Set filter coefficients for a high-pass Butterworth response.
   * 
   * @param sampleRate   Sample rate in Hz (e.g., 44100)
   * @param cutoffHz     Cutoff frequency in Hz (-3dB point)
   * @param qualityFactor Q factor (typically 0.707 for Butterworth, unused for 2nd order)
   * 
   * Note: cutoffHz should be in range 20 Hz to (sampleRate/2 - 1000) Hz
   *       Values outside this range are clamped to avoid numerical instability.
   */
  void setCoefficients(float sampleRate, float cutoffHz, float qualityFactor = 0.707f);
  
  /**
   * Process a single sample through the filter.
   * 
   * @param x Input sample
   * @return  Filtered output sample
   * 
   * This method is optimized for per-sample processing in audio loops.
   * For block processing, use processBlock() instead.
   */
  float processSingleSample(float x);
  
  /**
   * Process a block of samples (optimized version).
   * 
   * @param input  Input buffer (numSamples floats)
   * @param output Output buffer (can be same as input for in-place processing)
   * @param numSamples Number of samples to process
   */
  void processBlock(const float* input, float* output, int numSamples);
  
  /**
   * Get current filter state (for debugging/analysis).
   * Returns the two most recent output samples.
   */
  void getState(float& y1, float& y2) const {
    y1 = _y1;
    y2 = _y2;
  }

private:
  // Biquad coefficients (normalized form, a0 = 1.0)
  float _b0, _b1, _b2;  // Feedforward
  float _a1, _a2;       // Feedback
  
  // Filter state (previous samples)
  float _x1 = 0.0f, _x2 = 0.0f;  // Previous inputs
  float _y1 = 0.0f, _y2 = 0.0f;  // Previous outputs
  
  // Calculate coefficients for high-pass Butterworth
  void updateCoefficients(float sampleRate, float cutoffHz);
};
```

**Acceptance:**
- [ ] Header compiles without errors
- [ ] API is clean and minimal
- [ ] Comments explain each method clearly

---

### Task 2.3: Implement Coefficient Calculation

**What:** Implement `setCoefficients()` and `updateCoefficients()`  
**Duration:** 3–4 hours  
**File:** `Source/DSP/iir_filter.cpp` (new file)

**Implementation Strategy:**

Use the normalized frequency method (more stable than bilinear transform at high Q factors):

```cpp
#include "iir_filter.h"
#include <algorithm>
#include <limits>

IIRFilter::IIRFilter()
  : _b0(1.0f), _b1(0.0f), _b2(0.0f),
    _a1(0.0f), _a2(0.0f) {
}

void IIRFilter::reset() {
  _x1 = 0.0f;
  _x2 = 0.0f;
  _y1 = 0.0f;
  _y2 = 0.0f;
}

void IIRFilter::setCoefficients(float sampleRate, float cutoffHz, float qualityFactor) {
  // Clamp cutoff to valid range
  cutoffHz = std::max(20.0f, std::min(cutoffHz, sampleRate * 0.49f - 1000.0f));
  
  updateCoefficients(sampleRate, cutoffHz);
}

void IIRFilter::updateCoefficients(float sampleRate, float cutoffHz) {
  // Avoid division by zero and denormalization
  if (sampleRate < 8000.0f || cutoffHz < 1.0f) {
    // Invalid parameters, use pass-through (unity gain)
    _b0 = 1.0f;
    _b1 = 0.0f;
    _b2 = 0.0f;
    _a1 = 0.0f;
    _a2 = 0.0f;
    return;
  }
  
  // Butterworth high-pass coefficient calculation
  // Using normalized frequency method (numerically stable)
  
  const float PI = 3.14159265358979f;
  
  // Normalized angular frequency
  float wc = 2.0f * PI * cutoffHz / sampleRate;
  
  // Butterworth pole calculation
  // For a 2-pole high-pass filter (Butterworth):
  float C = 1.0f / tanf(wc * 0.5f);
  float C2 = C * C;
  float sqrt2 = 1.41421356f;
  
  // Denominator (same for all coefficients)
  float denom = 1.0f + sqrt2 * C + C2;
  
  // Feedforward (numerator) - high-pass
  _b0 = 1.0f / denom;
  _b1 = -2.0f * _b0;
  _b2 = _b0;
  
  // Feedback
  _a1 = 2.0f * (1.0f - C2) / denom;
  _a2 = (1.0f - sqrt2 * C + C2) / denom;
  
  // Sanity checks
  if (std::isnan(_b0) || std::isnan(_a1) || std::isinf(_b0)) {
    // Numerical error, reset to pass-through
    _b0 = 1.0f;
    _b1 = 0.0f;
    _b2 = 0.0f;
    _a1 = 0.0f;
    _a2 = 0.0f;
  }
}

float IIRFilter::processSingleSample(float x) {
  // Biquad difference equation:
  // y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
  
  float y = _b0 * x + _b1 * _x1 + _b2 * _x2 - _a1 * _y1 - _a2 * _y2;
  
  // Update state
  _x2 = _x1;
  _x1 = x;
  _y2 = _y1;
  _y1 = y;
  
  return y;
}

void IIRFilter::processBlock(const float* input, float* output, int numSamples) {
  for (int i = 0; i < numSamples; ++i) {
    output[i] = processSingleSample(input[i]);
  }
}
```

**Acceptance:**
- [ ] Coefficient calculation compiles without errors
- [ ] Coefficients are mathematically correct
- [ ] No NaN/Inf in output for valid parameters
- [ ] State updates correctly

---

### Task 2.4: Validation Tests (Frequency Response)

**What:** Verify filter behavior matches expected Butterworth response  
**Duration:** 4–6 hours  
**File:** `Tests/iir_tests.cpp`

**Test Strategy:**

1. **DC blocking:** At DC (0 Hz), gain should be ~0 dB (or better: unity for HPF)
2. **Cutoff point:** At specified cutoff frequency, gain should be -3 dB
3. **Roll-off:** Beyond cutoff, gain decreases at -12 dB/octave
4. **Phase:** At cutoff, phase shift should be approximately -90°

**Implementation:**

```cpp
#include "catch.hpp"
#include "iir_filter.h"
#include <cmath>

// Helper: Generate sine wave at given frequency
void generateSineWave(float* buffer, int numSamples, float frequency, float sampleRate) {
  const float TWO_PI = 6.28318530718f;
  for (int i = 0; i < numSamples; ++i) {
    float phase = (float)i * frequency / sampleRate;
    buffer[i] = sinf(phase * TWO_PI);
  }
}

// Helper: Measure RMS amplitude of signal
float measureRMS(const float* signal, int numSamples) {
  float sum = 0.0f;
  for (int i = 0; i < numSamples; ++i) {
    sum += signal[i] * signal[i];
  }
  return sqrtf(sum / numSamples);
}

// Helper: Measure frequency response (dB)
float measureFrequencyResponse(IIRFilter& filter, float frequency, float sampleRate) {
  // Generate 1 second of test signal at given frequency
  const int TEST_SAMPLES = (int)sampleRate;
  float input[TEST_SAMPLES], output[TEST_SAMPLES];
  generateSineWave(input, TEST_SAMPLES, frequency, sampleRate);
  
  filter.reset();
  
  // Skip first 100 ms (transient settling)
  const int SETTLE_SAMPLES = (int)(sampleRate * 0.1f);
  for (int i = 0; i < SETTLE_SAMPLES; ++i) {
    filter.processSingleSample(input[i]);
  }
  
  // Measure steady-state amplitude
  for (int i = SETTLE_SAMPLES; i < TEST_SAMPLES; ++i) {
    output[i] = filter.processSingleSample(input[i]);
  }
  
  float inputRMS = measureRMS(input + SETTLE_SAMPLES, TEST_SAMPLES - SETTLE_SAMPLES);
  float outputRMS = measureRMS(output + SETTLE_SAMPLES, TEST_SAMPLES - SETTLE_SAMPLES);
  
  if (inputRMS < 0.0001f) return 0.0f;
  float gainLinear = outputRMS / inputRMS;
  float gainDB = 20.0f * log10f(gainLinear);
  
  return gainDB;
}

TEST_CASE("IIRFilter: Butterworth response") {
  IIRFilter filter;
  float sampleRate = 44100.0f;
  float cutoffHz = 100.0f;
  
  filter.setCoefficients(sampleRate, cutoffHz);
  
  // Test 1: Below cutoff (should be attenuated ~-20 dB per decade)
  float response10Hz = measureFrequencyResponse(filter, 10.0f, sampleRate);
  REQUIRE(response10Hz < -20.0f); // At least -20 dB
  
  // Test 2: At cutoff (should be ~-3 dB)
  float responseAtCutoff = measureFrequencyResponse(filter, cutoffHz, sampleRate);
  REQUIRE(responseAtCutoff == Catch::Approx(-3.0f).margin(0.5f));
  
  // Test 3: Above cutoff (should approach 0 dB)
  float response1kHz = measureFrequencyResponse(filter, 1000.0f, sampleRate);
  REQUIRE(response1kHz > -0.5f);
}

TEST_CASE("IIRFilter: State management") {
  IIRFilter filter;
  filter.setCoefficients(44100.0f, 100.0f);
  
  // Process some samples
  filter.processSingleSample(0.5f);
  filter.processSingleSample(0.3f);
  
  float y1, y2;
  filter.getState(y1, y2);
  
  // Verify state was captured
  REQUIRE(y1 != 0.0f || y2 != 0.0f);
  
  // Reset and verify state clears
  filter.reset();
  filter.getState(y1, y2);
  REQUIRE(y1 == 0.0f);
  REQUIRE(y2 == 0.0f);
}

TEST_CASE("IIRFilter: Block processing matches sample-by-sample") {
  IIRFilter filterSingle, filterBlock;
  float sampleRate = 44100.0f;
  float cutoffHz = 200.0f;
  
  filterSingle.setCoefficients(sampleRate, cutoffHz);
  filterBlock.setCoefficients(sampleRate, cutoffHz);
  
  // Generate test signal
  const int SAMPLES = 1000;
  float input[SAMPLES], outputSingle[SAMPLES], outputBlock[SAMPLES];
  generateSineWave(input, SAMPLES, 1000.0f, sampleRate);
  
  // Process sample-by-sample
  filterSingle.reset();
  for (int i = 0; i < SAMPLES; ++i) {
    outputSingle[i] = filterSingle.processSingleSample(input[i]);
  }
  
  // Process as block
  filterBlock.reset();
  filterBlock.processBlock(input, outputBlock, SAMPLES);
  
  // Compare (should be identical)
  for (int i = 0; i < SAMPLES; ++i) {
    REQUIRE(outputSingle[i] == Catch::Approx(outputBlock[i]));
  }
}

TEST_CASE("IIRFilter: Stability check (random parameters)") {
  IIRFilter filter;
  
  // Test various cutoff frequencies
  float testFrequencies[] = {20, 50, 100, 500, 1000, 5000, 10000, 15000};
  for (float fc : testFrequencies) {
    filter.setCoefficients(44100.0f, fc);
    
    // Process random noise
    for (int i = 0; i < 10000; ++i) {
      float noise = (rand() % 2000 - 1000) / 1000.0f; // Random -1 to +1
      float output = filter.processSingleSample(noise);
      
      // Verify no NaN/Inf
      REQUIRE(std::isfinite(output));
    }
  }
}
```

**Acceptance:**
- [ ] Frequency response accurate within ±1 dB
- [ ] Roll-off behavior correct (-12 dB/octave)
- [ ] State management works correctly
- [ ] No NaN/Inf at any tested frequency

---

### Task 2.5: Integrate IIRFilter into CompressorChain

**What:** Update CompressorChain to use custom `IIRFilter` instead of JUCE placeholder  
**Duration:** 2–3 hours  
**Files to Modify:**
- `Source/DSP/compressor_chain.h` (replace placeholder)
- `Source/DSP/compressor_chain.cpp` (update initialization and processing)

**Changes:**

1. **Header:** Replace placeholder with actual IIRFilter include:
   ```cpp
   // In compressor_chain.h, replace:
   // class IIRFilterState { ... };
   // With:
   #include "iir_filter.h"
   
   private:
     IIRFilter _levelHighpassInputLeft;
     IIRFilter _levelHighpassInputRight;
     IIRFilter _levelHighpassSidechain;
   ```

2. **Implementation:** Update `init()` to configure filters:
   ```cpp
   void CompressorChain::init(float sampleRate) {
     // ... existing code ...
     
     // Configure filters at initialization sample rate
     _levelHighpassInputLeft.setCoefficients(sampleRate, 200.0f, 0.8f);
     _levelHighpassInputRight.setCoefficients(sampleRate, 200.0f, 0.8f);
     _levelHighpassSidechain.setCoefficients(sampleRate, 200.0f, 0.8f);
   }
   ```

3. **Processing:** Update `process()` to use filter API:
   ```cpp
   // In process() method, replace placeholder calls with:
   if (localInputLoCutFrequency != _inputPreviousFreq) {
     _levelHighpassInputLeft.setCoefficients(_sampleRate, localInputLoCutFrequency, 0.8f);
     _levelHighpassInputRight.setCoefficients(_sampleRate, localInputLoCutFrequency, 0.8f);
     _inputPreviousFreq = localInputLoCutFrequency;
   }
   
   filteredSample = _levelHighpassInputLeft.processSingleSample(sample);
   ```

**Acceptance:**
- [ ] CompressorChain still compiles without warnings
- [ ] Audio output matches Phase 1 golden reference
- [ ] Filter cutoff changes work correctly during processing

---

### Task 2.6: Regression Testing

**What:** Verify CompressorChain output unchanged with new filter  
**Duration:** 3–4 hours

**Procedure:**

1. **Generate golden reference:**
   - Use Phase 1 build (with placeholder filter)
   - Process known test signals
   - Save output as `Tests/fixtures/golden_phase1.wav`

2. **Test with new filter:**
   - Rebuild with custom IIRFilter
   - Process identical input signals
   - Compare outputs

3. **Measurement:**
   ```
   RMS_error = sqrt(mean((output_new - output_golden)^2))
   ```
   - Target: RMS_error < 0.01 dB

**Test Signals:**
- Pink noise (1 second)
- Sine sweep (20 Hz to 20 kHz, 10 seconds)
- Drum hit sample (transient test)
- Complex orchestral music (real-world test)

**Tools:**
- Python/SciPy for RMS calculation
- Audacity for visual inspection
- FFmpeg for audio format conversion

**Acceptance:**
- [ ] RMS error < 0.01 dB on all test signals
- [ ] No audible differences in A/B comparison
- [ ] Peak levels within ±0.1 dB

---

### Task 2.7: Code Review & Documentation

**What:** Final review of IIR filter implementation  
**Duration:** 3–4 hours

**Checklist:**

- [ ] Coefficient math verified against reference implementations
- [ ] No compiler warnings (gcc, clang, MSVC)
- [ ] Clang-Tidy passes
- [ ] Comments explain algorithm clearly
- [ ] Edge cases handled (very low/high frequencies)
- [ ] Function signatures documented
- [ ] Test coverage >95%

**Documentation to Add:**

1. **Algorithm explanation** in `iir_filter.h`:
   ```cpp
   /**
    * IIRFilter Implementation Notes
    * 
    * Butterworth high-pass design using normalized frequency method.
    * This avoids numerical issues with the bilinear transform at very
    * high or very low frequencies.
    * 
    * Coefficient calculation (from RBJ cookbook):
    *   wc = 2π * fc / fs
    *   C = 1 / tan(wc/2)
    *   b0 = 1 / (1 + √2*C + C²)
    *   b1 = -2*b0
    *   b2 = b0
    *   a1 = 2(1 - C²) / (1 + √2*C + C²)
    *   a2 = (1 - √2*C + C²) / (1 + √2*C + C²)
    * 
    * Difference equation:
    *   y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
    * 
    * Characteristics:
    *   - Gain at DC: 0 (ideal high-pass)
    *   - Gain at Nyquist: 0 dB (ideal)
    *   - Gain at cutoff: -3 dB (definition)
    *   - Roll-off: -12 dB/octave (2nd order)
    *   - Phase: -90° at cutoff, approaching 0° at high frequencies
    */
   ```

2. **Create FILTER_THEORY.md** (optional but useful):
   - Explains Butterworth design
   - Compares with other filter types
   - Stability analysis
   - Numerical precision discussion

**Acceptance:**
- [ ] Algorithm verified against RBJ cookbook
- [ ] Documentation complete and clear
- [ ] No warnings on all platforms
- [ ] All tests passing

---

## Deliverables Checklist

- [ ] `Source/DSP/iir_filter.h` (complete, documented)
- [ ] `Source/DSP/iir_filter.cpp` (implementation)
- [ ] `Tests/iir_tests.cpp` (20+ test cases)
- [ ] `Source/DSP/compressor_chain.h` (updated to use IIRFilter)
- [ ] `Source/DSP/compressor_chain.cpp` (updated filter calls)
- [ ] `Tests/fixtures/golden_phase1.wav` (reference audio)
- [ ] Regression test results (RMS error < 0.01 dB)
- [ ] Git commit with Phase 2 summary

---

## Success Criteria

Phase 2 is **complete** when:

✅ IIRFilter class compiles without warnings  
✅ Butterworth frequency response accurate (±1 dB at cutoff)  
✅ No NaN/Inf with valid parameters  
✅ CompressorChain output matches Phase 1 reference  
✅ Unit tests: 15+ cases, all passing  
✅ Regression tests: RMS error < 0.01 dB  
✅ Code passes static analysis  
✅ Documentation complete  
✅ All three filter instances work correctly  
✅ Dynamic frequency changes work during processing  

---

## Next Steps (When Complete)

1. Merge Phase 2 branch to `main`
2. Update [IMPLEMENTATION_PLAN.md](./IMPLEMENTATION_PLAN.md): Phase 2 → ✅ Completed
3. Begin [PHASE_3_SAMPLE_RATE_FIX.md](./PHASE_3_SAMPLE_RATE_FIX.md) (sample rate handling)

---

## Timeline

- Days 1–2: Task 2.1 (theory) + 2.2 (header)
- Days 3–4: Task 2.3 (implementation)
- Days 5–7: Task 2.4 (validation tests)
- Days 8: Task 2.5 (integration)
- Days 9–10: Task 2.6 (regression testing)
- Days 11: Task 2.7 (review & documentation)

**Total: 1–2 weeks**
