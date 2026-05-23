#include "MangrovePlugin.h"
#include "IPlug_include_in_plug_src.h"
#include "MangroveUI.h"
#include <algorithm>
using namespace iplug;
using namespace iplug::igraphics;

MangrovePlugin::MangrovePlugin(const InstanceInfo& info)
    : Plugin(info, MakeConfig(kNumParams, 1))
{
    GetParam(kInputGain)->InitDouble("Input Gain", 0., -24., 24., 0.01, "dB");
    GetParam(kInputLoCut)->InitDouble("Input Lo Cut", 80., 20., 300., 0.1, "Hz");
    GetParam(kInputSaturate)->InitDouble("Input Saturate", 0., 0., 5., 0.01, "");
    GetParam(kLevelThreshold)->InitDouble("Level Threshold", -10., -60., 0., 0.1, "dB");
    GetParam(kLevelRatio)->InitDouble("Level Ratio", 2.5, 1., 10., 0.01, "");
    GetParam(kLevelAttack)->InitDouble("Level Attack", 10., 0., 100., 0.1, "ms");
    GetParam(kLevelRelease)->InitDouble("Level Release", 300., 10., 500., 1., "ms");
    GetParam(kLevelLoCut)->InitBool("Level Lo Cut", false);
    GetParam(kLevelTubeGain)->InitBool("Level Tube Gain", false);
    GetParam(kLevelFeedback)->InitBool("Level Feedback", true);
    GetParam(kDensityThreshold)->InitDouble("Density Threshold", -10., -36., 0., 0.1, "dB");
    GetParam(kDensityRatio)->InitDouble("Density Ratio", 1., 1., 10., 0.01, "");
    GetParam(kDensityAttack)->InitDouble("Density Attack", 10., 0.001, 100., 0.001, "ms");
    GetParam(kDensityRelease)->InitDouble("Density Release", 300., 10., 2000., 1., "ms");

    mMakeGraphicsFunc = [&]() {
        return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS);
    };
    mLayoutFunc = [&](IGraphics* ui) {
        MangroveUI::Layout(*ui, *this);
    };
}

void MangrovePlugin::OnReset()
{
    const double sr = GetSampleRate();
    if (sr >= 8000. && sr <= 192000.) {
        mChain.init(static_cast<float>(sr));
        for (int i = 0; i < kNumParams; ++i)
            OnParamChange(i);
    }
}

void MangrovePlugin::OnParamChange(int p)
{
    const double v = GetParam(p)->Value();
    switch (p) {
        case kInputGain:        mChain.setInputGain(static_cast<float>(v));        break;
        case kInputLoCut:       mChain.setInputLoCut(static_cast<float>(v));       break;
        case kInputSaturate:    mChain.setInputSaturate(static_cast<float>(v));    break;
        case kLevelThreshold:   mChain.setLevelThreshold(static_cast<float>(v));   break;
        case kLevelRatio:       mChain.setLevelRatio(static_cast<float>(v));       break;
        case kLevelAttack:      mChain.setLevelAttack(static_cast<float>(v));      break;
        case kLevelRelease:     mChain.setLevelRelease(static_cast<float>(v));     break;
        case kLevelLoCut:       mChain.setLevelLoCut(v >= 0.5);                    break;
        case kLevelTubeGain:    mChain.setLevelTubeGain(v >= 0.5);                break;
        case kLevelFeedback:    mChain.setLevelFeedback(v >= 0.5);                break;
        case kDensityThreshold: mChain.setDensityThreshold(static_cast<float>(v)); break;
        case kDensityRatio:     mChain.setDensityRatio(static_cast<float>(v));     break;
        case kDensityAttack:    mChain.setDensityAttack(static_cast<float>(v));    break;
        case kDensityRelease:   mChain.setDensityRelease(static_cast<float>(v));   break;
        default: break;
    }
}

void MangrovePlugin::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
    const int n = std::min(nFrames, kMaxBlockSize);
    for (int i = 0; i < n; ++i) {
        mInL[i] = static_cast<float>(inputs[0][i]);
        mInR[i] = static_cast<float>((NChannelsConnected(kInput) > 1 ? inputs[1] : inputs[0])[i]);
    }
    mChain.process(mInL, mInR, mOutL, mOutR, n);
    for (int i = 0; i < n; ++i) {
        outputs[0][i] = static_cast<sample>(mOutL[i]);
        if (NChannelsConnected(kOutput) > 1) outputs[1][i] = static_cast<sample>(mOutR[i]);
    }
}

void MangrovePlugin::OnIdle()
{
    // Push meter data to UI controls if needed
}
