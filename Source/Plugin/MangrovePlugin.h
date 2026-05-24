#pragma once
#include "config.h"
#include "IPlug_include_in_plug_hdr.h"
#include "compressor_chain.h"

enum EParams {
    kInputGain = 0, kInputLoCut, kInputSaturate,
    kLevelThreshold, kLevelRatio, kLevelAttack, kLevelRelease,
    kLevelLoCut, kLevelTubeGain, kLevelFeedback, kLevelFast,
    kDensityThreshold, kDensityRatio, kDensityAttack, kDensityRelease,
    kNumParams
};
static_assert(kNumParams == PLUG_N_PARAMS, "Parameter count mismatch");

class MangrovePlugin final : public iplug::Plugin {
public:
    explicit MangrovePlugin(const iplug::InstanceInfo& info);

    void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) override;
    void OnReset() override;
    void OnParamChange(int paramIdx) override;
    void OnIdle() override;

private:
    static iplug::Config MakePluginConfig();

    CompressorChain mChain;
    static constexpr int kMaxBlockSize = 8192;
    float mInL[kMaxBlockSize], mInR[kMaxBlockSize];
    float mOutL[kMaxBlockSize], mOutR[kMaxBlockSize];
};
