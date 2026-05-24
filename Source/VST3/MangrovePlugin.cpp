#include "config.h"
#include "IPlug_include_in_plug_hdr.h"
#include "MangrovePlugin.h"
#include <algorithm>

using namespace iplug;

// Create plugin configuration
Config MakeConfig(int nParams, int nPresets)
{
    return Config(nParams, nPresets, PLUG_CHANNEL_IO,
                  PLUG_NAME, PLUG_NAME, "Nassau",
                  PLUG_VERSION_HEX, PLUG_UNIQUE_ID, PLUG_MFR_ID,
                  PLUG_LATENCY, PLUG_DOES_MIDI_IN, PLUG_DOES_MIDI_OUT,
                  PLUG_DOES_MPE, PLUG_DOES_STATE_CHUNKS, kEffect, PLUG_HAS_UI,
                  PLUG_WIDTH, PLUG_HEIGHT, false, 600, 600, 400, 400,
                  "com.nassau.mangrove", "");
}

MangrovePlugin::MangrovePlugin(const InstanceInfo& info)
    : Plugin(info, MakeConfig(PLUG_N_PARAMS, PLUG_N_PRESETS))
{
    // ── Input Stage ──────────────────────────────────────────────────────────
    GetParam(kInputGain)->InitDouble(
        "Input Gain", 0.0, -24.0, 24.0, 0.1, "dB");

    GetParam(kInputLoCut)->InitDouble(
        "Input LoCut", 80.0, 20.0, 300.0, 1.0, "Hz");

    GetParam(kInputSaturate)->InitDouble(
        "Input Saturate", 0.0, 0.0, 5.0, 0.1, "");

    // ── Level Compressor ─────────────────────────────────────────────────────
    GetParam(kLevelThreshold)->InitDouble(
        "Level Threshold", -10.0, -60.0, 0.0, 0.5, "dB");

    GetParam(kLevelRatio)->InitDouble(
        "Level Ratio", 2.5, 1.0, 10.0, 0.1, "");

    GetParam(kLevelAttack)->InitDouble(
        "Level Attack", 10.0, 0.0, 100.0, 1.0, "ms");

    GetParam(kLevelRelease)->InitDouble(
        "Level Release", 300.0, 10.0, 500.0, 10.0, "ms");

    GetParam(kLevelLoCut)->InitBool(
        "Level LoCut", false);

    GetParam(kLevelTubeGain)->InitBool(
        "Level TubeGain", false);

    GetParam(kLevelFeedback)->InitBool(
        "Level Feedback", true);

    // ── Density Compressor ───────────────────────────────────────────────────
    GetParam(kDensityThreshold)->InitDouble(
        "Density Threshold", -10.0, -36.0, 0.0, 0.5, "dB");

    GetParam(kDensityRatio)->InitDouble(
        "Density Ratio", 1.0, 1.0, 10.0, 0.1, "");

    GetParam(kDensityAttack)->InitDouble(
        "Density Attack", 10.0, 0.001, 100.0, 0.5, "ms");

    GetParam(kDensityRelease)->InitDouble(
        "Density Release", 300.0, 10.0, 2000.0, 10.0, "ms");
}

void MangrovePlugin::OnReset()
{
    const double sr = GetSampleRate();
    if (sr >= 8000.0 && sr <= 192000.0) {
        mChain.init(static_cast<float>(sr));
        for (int i = 0; i < kNumParams; ++i) {
            OnParamChange(i, EParamSource::kReset, -1);
        }
    }
}

void MangrovePlugin::OnParamChange(int paramIdx,
                                    EParamSource /*source*/,
                                    int /*sampleOffset*/)
{
    const double v = GetParam(paramIdx)->Value();

    switch (paramIdx) {
        case kInputGain:
            mChain.setInputGain(static_cast<float>(v));
            break;
        case kInputLoCut:
            mChain.setInputLoCut(static_cast<float>(v));
            break;
        case kInputSaturate:
            mChain.setInputSaturate(static_cast<float>(v));
            break;
        case kLevelThreshold:
            mChain.setLevelThreshold(static_cast<float>(v));
            break;
        case kLevelRatio:
            mChain.setLevelRatio(static_cast<float>(v));
            break;
        case kLevelAttack:
            mChain.setLevelAttack(static_cast<float>(v));
            break;
        case kLevelRelease:
            mChain.setLevelRelease(static_cast<float>(v));
            break;
        case kLevelLoCut:
            mChain.setLevelLoCut(v >= 0.5);
            break;
        case kLevelTubeGain:
            mChain.setLevelTubeGain(v >= 0.5);
            break;
        case kLevelFeedback:
            mChain.setLevelFeedback(v >= 0.5);
            break;
        case kDensityThreshold:
            mChain.setDensityThreshold(static_cast<float>(v));
            break;
        case kDensityRatio:
            mChain.setDensityRatio(static_cast<float>(v));
            break;
        case kDensityAttack:
            mChain.setDensityAttack(static_cast<float>(v));
            break;
        case kDensityRelease:
            mChain.setDensityRelease(static_cast<float>(v));
            break;
        default:
            break;
    }
}

void MangrovePlugin::ProcessBlock(sample** inputs,
                                   sample** outputs,
                                   int nFrames)
{
    const int n = std::min(nFrames, kMaxBlockSize);

    // This is a stereo plugin: 2 input channels, 2 output channels
    const sample* inL = inputs[0];
    const sample* inR = inputs[1];

    for (int i = 0; i < n; ++i) {
        mInL[i] = static_cast<float>(inL[i]);
        mInR[i] = static_cast<float>(inR[i]);
    }

    mChain.process(mInL, mInR, mOutL, mOutR, n);

    sample* outL = outputs[0];
    sample* outR = outputs[1];

    for (int i = 0; i < n; ++i) {
        outL[i] = static_cast<sample>(mOutL[i]);
        outR[i] = static_cast<sample>(mOutR[i]);
    }
}

#include "IPlug_include_in_plug_src.h"
