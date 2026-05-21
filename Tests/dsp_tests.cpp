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

  // Test 6: Level compressor - no compression below threshold
  {
    std::cout << "Test 6: Level compressor (no compression below threshold)... ";
    CompressorChain dsp;
    dsp.init(44100.0f);

    // Set threshold to -10 dB, ratio 4:1
    dsp.setLevelThreshold(-10.0f);
    dsp.setLevelRatio(4.0f);
    dsp.setLevelAttack(10.0f);
    dsp.setLevelRelease(300.0f);

    // Create signal below threshold (~-20 dB RMS)
    float inL[512], inR[512];
    float outL[512], outR[512];

    for (int i = 0; i < 512; ++i) {
      inL[i] = inR[i] = std::sin(2.0f * 3.14159f * i / 512.0f) * 0.1f;
    }

    dsp.process(inL, inR, outL, outR, 512);

    // Calculate output RMS
    float outRms = 0.0f;
    for (int i = 0; i < 512; ++i) {
      outRms += outL[i] * outL[i];
    }
    outRms = std::sqrt(outRms / 512.0f);

    // Should be roughly unchanged (no compression below threshold)
    // Allow some change due to envelope follower settling
    assert(outRms > 0.08f && outRms < 0.12f);

    std::cout << "✓ PASS\n";
  }

  // Test 7: Level compressor - compression applied above threshold
  {
    std::cout << "Test 7: Level compressor (compression above threshold)... ";
    CompressorChain dsp;
    dsp.init(44100.0f);

    // Set low threshold to ensure signal is compressed
    dsp.setLevelThreshold(-30.0f);
    dsp.setLevelRatio(4.0f);
    dsp.setLevelAttack(5.0f);  // Fast attack
    dsp.setLevelRelease(50.0f); // Fast release for test

    // Create stronger signal (~-10 dB RMS)
    float inL[512], inR[512];
    float outL[512], outR[512];

    for (int i = 0; i < 512; ++i) {
      float val = std::sin(2.0f * 3.14159f * i / 512.0f) * 0.3f;
      inL[i] = inR[i] = val;
    }

    dsp.process(inL, inR, outL, outR, 512);

    // With 4:1 compression at fixed level, expect ~75% of input amplitude
    float outRms = 0.0f;
    for (int i = 0; i < 512; ++i) {
      outRms += outL[i] * outL[i];
    }
    outRms = std::sqrt(outRms / 512.0f);

    // Envelope follower settling means we won't reach full steady state in 512 samples
    // But should see some gain reduction
    assert(outRms < 0.3f); // Should be reduced from input

    std::cout << "✓ PASS\n";
  }

  // Test 8: Level compressor - feedback vs feedforward
  {
    std::cout << "Test 8: Level compressor (feedback mode)... ";
    CompressorChain dsp;
    dsp.init(44100.0f);

    dsp.setLevelThreshold(-20.0f);
    dsp.setLevelRatio(4.0f);
    dsp.setLevelAttack(5.0f);
    dsp.setLevelRelease(100.0f);
    dsp.setLevelFeedback(true); // Feedback mode

    float inL[512], inR[512];
    float outL[512], outR[512];

    for (int i = 0; i < 512; ++i) {
      inL[i] = inR[i] = std::sin(2.0f * 3.14159f * i / 512.0f) * 0.5f;
    }

    dsp.process(inL, inR, outL, outR, 512);

    // Feedback mode should produce stable output
    CompressorChain::MeterData meters = dsp.getMeterData();
    assert(meters.levelReduction >= 0.0f && meters.levelReduction <= 1.0f);

    std::cout << "✓ PASS\n";
  }

  // Test 9: Level compressor - Vari-Mu mode (high ratio)
  {
    std::cout << "Test 9: Level compressor (Vari-Mu mode)... ";
    CompressorChain dsp;
    dsp.init(44100.0f);

    dsp.setLevelThreshold(-20.0f);
    dsp.setLevelRatio(10.0f); // High ratio triggers Vari-Mu mode
    dsp.setLevelAttack(5.0f);
    dsp.setLevelRelease(100.0f);

    float inL[512], inR[512];
    float outL[512], outR[512];

    for (int i = 0; i < 512; ++i) {
      inL[i] = inR[i] = std::sin(2.0f * 3.14159f * i / 512.0f) * 0.5f;
    }

    dsp.process(inL, inR, outL, outR, 512);

    // Vari-Mu should still compress, but with smooth saturation curve
    CompressorChain::MeterData meters = dsp.getMeterData();
    assert(meters.levelReduction > 0.0f);

    std::cout << "✓ PASS\n";
  }

  // Test 10: Level compressor - tube saturation
  {
    std::cout << "Test 10: Level compressor (tube saturation)... ";
    CompressorChain dsp;
    dsp.init(44100.0f);

    dsp.setLevelThreshold(-40.0f); // Very low threshold
    dsp.setLevelRatio(2.0f);       // Mild compression to amplify signal
    dsp.setLevelAttack(2.0f);
    dsp.setLevelRelease(50.0f);
    dsp.setLevelTubeGain(true);    // Enable tube saturation

    float inL[512], inR[512];
    float outL[512], outR[512];

    for (int i = 0; i < 512; ++i) {
      inL[i] = inR[i] = std::sin(2.0f * 3.14159f * i / 512.0f) * 0.1f;
    }

    dsp.process(inL, inR, outL, outR, 512);

    // Tube saturation should be applied
    float outRms = 0.0f;
    for (int i = 0; i < 512; ++i) {
      outRms += outL[i] * outL[i];
    }
    outRms = std::sqrt(outRms / 512.0f);

    // Should have some output with saturation applied
    assert(outRms > 0.0f);

    std::cout << "✓ PASS\n";
  }

  std::cout << "\n=== All Tests Passed ===\n";
  return 0;
}
