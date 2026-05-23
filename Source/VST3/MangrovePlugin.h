#pragma once

#include "config.h"
#include "IPlug_include_in_plug_hdr.h"
#include "compressor_chain.h"

enum EParams {
    kInputGain = 0,
    kInputLoCut,
    kInputSaturate,
    kLevelThreshold,
    kLevelRatio,
    kLevelAttack,
    kLevelRelease,
    kLevelLoCut,
    kLevelTubeGain,
    kLevelFeedback,
    kDensityThreshold,
    kDensityRatio,
    kDensityAttack,
    kDensityRelease,
    kNumParams
};

static_assert(kNumParams == PLUG_N_PARAMS, "Parameter count mismatch");

class MangrovePlugin final : public iplug::Plugin {
public:
    explicit MangrovePlugin(const iplug::InstanceInfo& info);

    void ProcessBlock(iplug::sample** inputs,
                      iplug::sample** outputs,
                      int nFrames) override;

    void OnReset() override;

    void OnParamChange(int paramIdx,
                       iplug::EParamSource source,
                       int sampleOffset) override;

private:
    CompressorChain mChain;

    static constexpr int kMaxBlockSize = 8192;
    float mInL[kMaxBlockSize];
    float mInR[kMaxBlockSize];
    float mOutL[kMaxBlockSize];
    float mOutR[kMaxBlockSize];
};
