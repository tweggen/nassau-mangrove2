#include "compressor_chain.h"
#include <cmath>
#include <algorithm>

// ===== Constructor & Initialization =====

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
  // Placeholder - will be implemented in Phase 3
  // TODO: Implement dynamic attack/release coefficient calculations
}

// ===== Parameter Setters =====

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
  recalculateTimeCoefficients();
}

void CompressorChain::setLevelRelease(float releaseMs) {
  _paramLevelRelease.store(releaseMs, std::memory_order_relaxed);
  recalculateTimeCoefficients();
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

// ===== Metering =====

CompressorChain::MeterData CompressorChain::getMeterData() const {
  MeterData data;
  data.inputGain = _meterInputGain.load(std::memory_order_relaxed);
  data.levelReduction = _meterLevelReduction.load(std::memory_order_relaxed);
  data.densityReduction = _meterDensityReduction.load(std::memory_order_relaxed);
  return data;
}

// ===== Audio Processing =====

void CompressorChain::process(const float* inL, const float* inR,
                               float* outL, float* outR,
                               int numSamples) {
  // ===== INPUT STAGE =====
  // 1. Apply input gain (dB to linear conversion)
  // 2. Apply saturation (soft-clipping for negative samples)
  // 3. Apply high-pass filter (LoCut)

  // Get current parameter values (atomic, lock-free)
  float localInputGain = std::pow(10.0f, _paramInputGain.load(std::memory_order_relaxed) / 20.0f);
  float inputLoCutFreq = _paramInputLoCut.load(std::memory_order_relaxed);
  double inSaturation = _paramInputSaturate.load(std::memory_order_relaxed) * 10.0 + 1.0;

  // Metering accumulator
  double inputGainAccumulator = 0.0;

  // ===== INPUT STAGE PROCESSING LOOP =====
  for (int i = 0; i < numSamples; ++i) {
    // Read input samples
    float sampleL = inL[i];
    float sampleR = inR[i];

    // STEP 1: Apply input gain (dB to linear)
    sampleL *= localInputGain;
    sampleR *= localInputGain;

    // Accumulate for metering (RMS pre-saturation)
    inputGainAccumulator += sampleL * sampleL + sampleR * sampleR;

    // STEP 2: Apply input saturation (soft-clipping, negative samples only)
    // Formula: out = (1/(1-(x/divisor)) - 1) * divisor
    // This only affects negative samples; positive samples pass through unchanged
    if (inSaturation > 1.01f) {
      float divisor = 20.0f / static_cast<float>(inSaturation);
      const float MaxOut = 4.0f; // Clamp output to prevent extreme values

      // Left channel saturation
      if (sampleL < 0.0f) {
        sampleL = (1.0f / (1.0f - (sampleL / divisor)) - 1.0f) * divisor;
        if (sampleL > MaxOut) {
          sampleL = MaxOut;
        }
      }
      // Note: positive samples left unchanged

      // Right channel saturation
      if (sampleR < 0.0f) {
        sampleR = (1.0f / (1.0f - (sampleR / divisor)) - 1.0f) * divisor;
        if (sampleR > MaxOut) {
          sampleR = MaxOut;
        }
      }
    }

    // STEP 3: Apply input high-pass filter (LoCut)
    // Actual filter implementation will be in Phase 2
    // For now: placeholder (no filtering)
    if (inputLoCutFreq > 20.1f) {
      // TODO (Phase 2): Apply actual IIR filter
      // out1 = _levelHighpassInputLeft.processSingleSample(sampleL);
      // out2 = _levelHighpassInputRight.processSingleSample(sampleR);
      // For Phase 1, HPF is disabled (pass-through)
    }

    // Write processed output
    outL[i] = sampleL;
    outR[i] = sampleR;
  }

  // ===== METERING =====
  // Calculate RMS of input after gain but before compression
  float inputRMS = std::sqrt(static_cast<float>(inputGainAccumulator / (2.0 * numSamples)));
  _meterInputGain.store(inputRMS, std::memory_order_relaxed);

  // Placeholder compression reduction (will be set in Phase 1.5-1.6)
  _meterLevelReduction.store(1.0f, std::memory_order_relaxed);
  _meterDensityReduction.store(1.0f, std::memory_order_relaxed);

  // PHASE 1 PROGRESS:
  // ✅ Task 1.4 - Input stage (gain, saturation, HPF placeholder)
  // TODO: Task 1.5 - Implement level compressor
  // TODO: Task 1.6 - Implement density compressor
  // TODO: Task 1.7 - Implement metering updates from compression
}
