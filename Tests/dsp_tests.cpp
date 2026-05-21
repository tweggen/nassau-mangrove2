// Mangrove DSP Unit Tests
// Tests for CompressorChain class

#include "compressor_chain.h"
#include <cassert>
#include <iostream>
#include <cmath>

int main() {
  std::cout << "=== Mangrove DSP Tests ===\n\n";

  // Test 1: Initialization
  {
    std::cout << "Test 1: CompressorChain initialization... ";
    CompressorChain dsp;
    dsp.init(44100.0f);
    assert(dsp.getSampleRate() == 44100.0f);
    assert(dsp.getLatencySamples() == 0);
    std::cout << "✓ PASS\n";
  }

  // Test 2: Parameter setters
  {
    std::cout << "Test 2: Parameter setters... ";
    CompressorChain dsp;
    dsp.init(44100.0f);

    dsp.setInputGain(6.0f);
    dsp.setInputLoCut(100.0f);
    dsp.setInputSaturate(2.0f);

    dsp.setLevelThreshold(-15.0f);
    dsp.setLevelRatio(4.0f);
    dsp.setLevelAttack(5.0f);
    dsp.setLevelRelease(200.0f);
    dsp.setLevelLoCut(true);
    dsp.setLevelTubeGain(false);
    dsp.setLevelFeedback(true);

    dsp.setDensityThreshold(-12.0f);
    dsp.setDensityRatio(2.0f);
    dsp.setDensityAttack(8.0f);
    dsp.setDensityRelease(400.0f);

    std::cout << "✓ PASS\n";
  }

  // Test 3: Meter data access
  {
    std::cout << "Test 3: Meter data access... ";
    CompressorChain dsp;
    dsp.init(44100.0f);

    CompressorChain::MeterData meters = dsp.getMeterData();
    assert(meters.inputGain >= 0.0f);
    assert(meters.levelReduction >= 0.0f);
    assert(meters.densityReduction >= 0.0f);

    std::cout << "✓ PASS\n";
  }

  // Test 4: Audio processing (passthrough)
  {
    std::cout << "Test 4: Audio processing (passthrough)... ";
    CompressorChain dsp;
    dsp.init(44100.0f);

    float inL[512], inR[512];
    float outL[512], outR[512];

    // Fill with test signal
    for (int i = 0; i < 512; ++i) {
      inL[i] = inR[i] = std::sin(2.0f * 3.14159f * i / 512.0f);
    }

    // Process
    dsp.process(inL, inR, outL, outR, 512);

    // Verify output exists (passthrough for now)
    float rms = 0.0f;
    for (int i = 0; i < 512; ++i) {
      rms += outL[i] * outL[i];
    }
    rms = std::sqrt(rms / 512.0f);
    assert(rms > 0.1f); // Should have some signal

    std::cout << "✓ PASS\n";
  }

  // Test 5: Multiple sample rates
  {
    std::cout << "Test 5: Multiple sample rates... ";
    float sampleRates[] = {44100.0f, 48000.0f, 96000.0f, 192000.0f};

    for (float fs : sampleRates) {
      CompressorChain dsp;
      dsp.init(fs);
      assert(dsp.getSampleRate() == fs);
    }

    std::cout << "✓ PASS\n";
  }

  std::cout << "\n=== All Tests Passed ===\n";
  return 0;
}
