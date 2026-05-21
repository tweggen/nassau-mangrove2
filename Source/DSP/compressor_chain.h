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

  // ===== IIR Filter State (Placeholder for Phase 2) =====
  // Will be replaced in Phase 2 with custom IIRFilter class
  // For now, pass-through (no filtering)
  class IIRFilterState {
  public:
    void reset() {}
    void setCoefficients(float, float, float) {}
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
