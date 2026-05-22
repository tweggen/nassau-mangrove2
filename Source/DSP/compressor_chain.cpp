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

  // Initialize sidechain HPF: fixed 200 Hz, Q=1.0
  _levelHighpassSidechain.setHighPass(_sampleRate, 200.0, 1.0);

  // Initialize time-based coefficients (attack/release)
  recalculateTimeCoefficients();
}

void CompressorChain::recalculateTimeCoefficients() {
  const double PI = 3.14159265358979323846;

  // Load current parameter values
  float levelAttackMs = _paramLevelAttack.load(std::memory_order_relaxed);
  float levelReleaseMs = _paramLevelRelease.load(std::memory_order_relaxed);
  float densityAttackMs = _paramDensityAttack.load(std::memory_order_relaxed);
  float densityReleaseMs = _paramDensityRelease.load(std::memory_order_relaxed);

  // Clamp to valid ranges to prevent division by zero or invalid math
  levelAttackMs = std::max(0.001f, levelAttackMs);
  levelReleaseMs = std::max(10.0f, levelReleaseMs);
  densityAttackMs = std::max(0.001f, densityAttackMs);
  densityReleaseMs = std::max(10.0f, densityReleaseMs);

  // Convert from time (ms) to coefficient (per-sample)
  // Formula: coeff = 1 - exp(-2*pi*freq / sampleRate)
  // where freq = 1000 / (timeMs * 2.2) empirically chosen for musical feel

  double sampleRate = static_cast<double>(_sampleRate);

  // Level compressor coefficients
  double levelAttackFreq = 1000.0 / (levelAttackMs * 2.2);
  double levelReleaseFreq = 1000.0 / (levelReleaseMs * 2.2);
  _levelAttackCoeff = 1.0 - std::exp(-2.0 * PI * levelAttackFreq / sampleRate);
  _levelReleaseCoeff = 1.0 - std::exp(-2.0 * PI * levelReleaseFreq / sampleRate);

  // Density compressor coefficients (typically faster)
  double densityAttackFreq = 1000.0 / (densityAttackMs * 2.2);
  double densityReleaseFreq = 1000.0 / (densityReleaseMs * 2.2);
  _densityAttackCoeff = 1.0 - std::exp(-2.0 * PI * densityAttackFreq / sampleRate);
  _densityReleaseCoeff = 1.0 - std::exp(-2.0 * PI * densityReleaseFreq / sampleRate);
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
  // Get current parameter values (atomic, lock-free)
  float localInputGain = std::pow(10.0f, _paramInputGain.load(std::memory_order_relaxed) / 20.0f);
  float inputLoCutFreq = _paramInputLoCut.load(std::memory_order_relaxed);
  double inSaturation = _paramInputSaturate.load(std::memory_order_relaxed) * 10.0 + 1.0;

  // Level compressor parameters
  float levelThreshold = _paramLevelThreshold.load(std::memory_order_relaxed);
  float levelRatio = _paramLevelRatio.load(std::memory_order_relaxed);
  bool levelLoCut = _paramLevelLoCut.load(std::memory_order_relaxed) > 0.5f;
  bool levelTubeGain = _paramLevelTubeGain.load(std::memory_order_relaxed) > 0.5f;
  bool levelFeedback = _paramLevelFeedback.load(std::memory_order_relaxed) > 0.5f;

  // Density compressor parameters
  float densityThreshold = _paramDensityThreshold.load(std::memory_order_relaxed);
  float densityRatio = _paramDensityRatio.load(std::memory_order_relaxed);

  // Metering accumulators
  double inputGainAccumulator = 0.0;

  // ===== PER-SAMPLE PROCESSING LOOP =====
  for (int i = 0; i < numSamples; ++i) {
    // Read input samples
    float sampleL = inL[i];
    float sampleR = inR[i];

    // ===== INPUT STAGE =====
    // STEP 1: Apply input gain (dB to linear)
    sampleL *= localInputGain;
    sampleR *= localInputGain;

    // Accumulate for metering (RMS pre-saturation)
    inputGainAccumulator += sampleL * sampleL + sampleR * sampleR;

    // STEP 2: Apply input saturation (soft-clipping, negative samples only)
    if (inSaturation > 1.01f) {
      float divisor = 20.0f / static_cast<float>(inSaturation);
      const float MaxOut = 4.0f;

      if (sampleL < 0.0f) {
        sampleL = (1.0f / (1.0f - (sampleL / divisor)) - 1.0f) * divisor;
        if (sampleL > MaxOut) sampleL = MaxOut;
      }

      if (sampleR < 0.0f) {
        sampleR = (1.0f / (1.0f - (sampleR / divisor)) - 1.0f) * divisor;
        if (sampleR > MaxOut) sampleR = MaxOut;
      }
    }

    // STEP 3: Input high-pass filter
    if (inputLoCutFreq > 20.1f) {
      if (inputLoCutFreq != _inputPreviousFreq) {
        _levelHighpassInputLeft.setHighPass(_sampleRate, inputLoCutFreq, 0.8);
        _levelHighpassInputRight.setHighPass(_sampleRate, inputLoCutFreq, 0.8);
        _inputPreviousFreq = inputLoCutFreq;
      }
      sampleL = _levelHighpassInputLeft.processSingleSample(sampleL);
      sampleR = _levelHighpassInputRight.processSingleSample(sampleR);
    }

    // ===== LEVEL COMPRESSOR SIDECHAIN DETECTION =====
    // Detect signal level for sidechain
    double detection = std::sqrt(sampleL * sampleL + sampleR * sampleR + 1e-10);
    _levelDetection = detection;

    // Apply optional sidechain high-pass filter
    if (levelLoCut) {
      float absSidechain = std::fabs(sampleL) + std::fabs(sampleR);
      float filtered = _levelHighpassSidechain.processSingleSample(absSidechain);
      _levelFilteredSideChain = std::fabs(filtered) + 1e-10;
    } else {
      _levelFilteredSideChain = detection;
    }

    // Feedback vs feedforward: use input or output sidechain
    // For feedforward, use filtered sidechain now
    // For feedback, we'll use the output after compression (circular, applied below)
    double sideChainLevel = _levelFilteredSideChain;
    if (levelFeedback) {
      sideChainLevel = _levelFilteredSideChain * _levelAmplification;
    }

    // ===== LEVEL COMPRESSOR ENVELOPE FOLLOWER =====
    // Convert sidechain level to dB
    double sidechainDb = 20.0 * std::log10(std::max(sideChainLevel, 1e-10));

    // Envelope follower with attack/release
    if (sidechainDb > _levelEnvFollow) {
      // Attack phase: ramp up quickly
      _levelEnvFollow += (sidechainDb - _levelEnvFollow) * _levelAttackCoeff;
      _attackReason = AttackPeak;
    } else {
      // Release phase: ramp down
      _levelEnvFollow += (sidechainDb - _levelEnvFollow) * _levelReleaseCoeff;
    }

    // ===== LEVEL COMPRESSOR CURVE CALCULATION =====
    double targetAmplification = 1.0;
    double envDb = _levelEnvFollow;

    if (envDb > levelThreshold) {
      double excessDb = envDb - levelThreshold;

      if (levelRatio >= 9.999) {
        // Vari-Mu mode: use soft atan curve for smooth compression
        double compressedDb = std::atan(excessDb * 0.25) * 2.0;
        targetAmplification = std::pow(10.0, (levelThreshold + compressedDb - envDb) / 20.0);
      } else {
        // Hard-knee: standard ratio-based compression
        double reductionDb = excessDb * (1.0 - 1.0 / levelRatio);
        targetAmplification = std::pow(10.0, -reductionDb / 20.0);
      }
    }

    // ===== LEVEL COMPRESSOR ATTACK/RELEASE RAMPING =====
    // Smooth ramp toward target amplification using attack/release coefficients
    if (targetAmplification < _levelAmplification) {
      _levelAmplification += (targetAmplification - _levelAmplification) * _levelAttackCoeff;
    } else {
      _levelAmplification += (targetAmplification - _levelAmplification) * _levelReleaseCoeff;
    }

    // Clamp to reasonable range
    _levelAmplification = std::max(0.001, std::min(10.0, _levelAmplification));

    // ===== TUBE SATURATION (OPTIONAL) =====
    if (levelTubeGain && _levelAmplification > 1.0) {
      float saturation = static_cast<float>((_levelAmplification - 1.0) * 0.5);
      if (saturation > 0.01f) {
        float divisor = 1.0f / saturation;
        sampleL = std::atan(sampleL / divisor) * divisor;
        sampleR = std::atan(sampleR / divisor) * divisor;
      }
    }

    // ===== APPLY LEVEL COMPRESSION GAIN =====
    sampleL *= static_cast<float>(_levelAmplification);
    sampleR *= static_cast<float>(_levelAmplification);

    // ===== DENSITY COMPRESSOR (FAST TRANSIENT LIMITER) =====
    // Peak detection: measure instantaneous peak (not RMS)
    double peakDetection = std::max(std::fabs(sampleL), std::fabs(sampleR));
    _densityDetection = peakDetection;

    // Density compressor uses simple feedforward (no sidechain filter)
    _densitySideChain = peakDetection;

    // Envelope follower for peak limiter (very fast)
    // Convert to dB
    double densityDb = 20.0 * std::log10(std::max(_densitySideChain, 1e-10));

    // Fast envelope following (attack and release both use attack coefficient)
    // This makes the limiter respond very quickly to peaks
    _densityEnvFollow += (densityDb - _densityEnvFollow) * _densityAttackCoeff;

    // ===== DENSITY COMPRESSOR CURVE CALCULATION =====
    // Simple limiter: hard ceiling at threshold
    double densityTargetAmplification = 1.0;
    double densityEnvDb = _densityEnvFollow;

    if (densityEnvDb > densityThreshold) {
      double excessDb = densityEnvDb - densityThreshold;

      if (densityRatio >= 9.999) {
        // Limiter mode: hard ceiling (infinite ratio, soft knee via atan)
        double limitDb = std::atan(excessDb * 0.5) * 1.0;
        densityTargetAmplification = std::pow(10.0, (densityThreshold + limitDb - densityEnvDb) / 20.0);
      } else {
        // Soft limiting with finite ratio
        double reductionDb = excessDb * (1.0 - 1.0 / densityRatio);
        densityTargetAmplification = std::pow(10.0, -reductionDb / 20.0);
      }
    }

    // ===== DENSITY COMPRESSOR RAMPING =====
    // Fast attack: use attack coefficient for both attack and release
    // This makes peaks respond very quickly
    _densityAmplification += (densityTargetAmplification - _densityAmplification) * _densityAttackCoeff;

    // But use release coefficient for the downramp to be less aggressive
    if (densityTargetAmplification > _densityAmplification) {
      _densityAmplification += (densityTargetAmplification - _densityAmplification) * _densityAttackCoeff;
    } else {
      _densityAmplification += (densityTargetAmplification - _densityAmplification) * _densityReleaseCoeff;
    }

    // Clamp to reasonable range
    _densityAmplification = std::max(0.001, std::min(10.0, _densityAmplification));

    // ===== APPLY DENSITY COMPRESSION GAIN =====
    // Density limiter is applied after level compressor
    sampleL *= static_cast<float>(_densityAmplification);
    sampleR *= static_cast<float>(_densityAmplification);

    // Write processed output
    outL[i] = sampleL;
    outR[i] = sampleR;
  }

  // ===== METERING =====
  float inputRMS = std::sqrt(static_cast<float>(inputGainAccumulator / (2.0 * numSamples)));
  _meterInputGain.store(inputRMS, std::memory_order_relaxed);

  // Store current amplification as meter reduction value
  float levelReduction = static_cast<float>(_levelAmplification);
  _meterLevelReduction.store(levelReduction, std::memory_order_relaxed);

  // Store density limiter reduction
  float densityReduction = static_cast<float>(_densityAmplification);
  _meterDensityReduction.store(densityReduction, std::memory_order_relaxed);

  // PHASE 1 PROGRESS:
  // ✅ Task 1.4 - Input stage (gain, saturation, HPF placeholder)
  // ✅ Task 1.5 - Level compressor (sidechain, envelope, compression curve, tube saturation)
  // ✅ Task 1.6 - Density compressor (peak detection, fast limiter, hard ceiling mode)
  // ✅ Task 1.7 - Metering (inputGain, levelReduction, densityReduction all active)
}
