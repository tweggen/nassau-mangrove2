/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
*/
class MangroveAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    MangroveAudioProcessor();
    ~MangroveAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	float inputGain;
	float muOutputGain;
	float dsOutputGain;
    float muReduction;
	float currLevelReduction;
	float currDenseReduction;

private:
	IIRFilter _levelHighpassInputLeft;
	IIRFilter _levelHighpassInputRight;
	IIRFilter _levelHighpassSidechain;

    std::atomic<float>* _paramInputGain = nullptr;
#define INPUT_GAIN_MIN (-24.0)
#define INPUT_GAIN_MAX 24.0
#define INPUT_GAIN_DEFAULT 0.0
#define INPUT_GAIN_STEP 1.0
    std::atomic<float>* _paramInputLoCut = nullptr;
#define INPUT_LO_CUT_MIN 20.0
#define INPUT_LO_CUT_MAX 300.0
#define INPUT_LO_CUT_DEFAULT 80.0
#define INPUT_LO_CUT_STEP 10.0
    std::atomic<float>* _paramInputSaturate = nullptr;
#define INPUT_SATURATE_MIN 0.0
#define INPUT_SATURATE_MAX 5.0
#define INPUT_SATURATE_DEFAULT 0.0
#define INPUT_SATURATE_STEP 0.1

    std::atomic<float>* _paramLevelThreshold = nullptr;
#define LEVEL_THRESHOLD_MIN (-60.0)
#define LEVEL_THRESHOLD_MAX 0.0
#define LEVEL_THRESHOLD_DEFAULT (-10.0)
#define LEVEL_THRESHOLD_STEP 1.0
    std::atomic<float>* _paramLevelRatio = nullptr;
#define LEVEL_RATIO_MIN 1.0
#define LEVEL_RATIO_MAX 10.0
#define LEVEL_RATIO_DEFAULT 2.5
#define LEVEL_RATIO_STEP 0.5
    std::atomic<float>* _paramLevelAttack = nullptr;
#define LEVEL_ATTACK_MIN 0.0
#define LEVEL_ATTACK_MAX 100.0
#define LEVEL_ATTACK_DEFAULT 10.0
#define LEVEL_ATTACK_STEP 10.0
    std::atomic<float>* _paramLevelRelease = nullptr;
#define LEVEL_RELEASE_MIN 10.0
#define LEVEL_RELEASE_MAX 500.0
#define LEVEL_RELEASE_STEP 100.0
#define LEVEL_RELEASE_DEFAULT 300.0
    std::atomic<float>* _paramLevelLoCut = nullptr;
#define LEVEL_LO_CUT_DEFAULT false
    std::atomic<float>* _paramLevelTubeGain = nullptr;
#define LEVEL_TUBE_GAIN_DEFAULT false
    std::atomic<float>* _paramLevelFeedback = nullptr;
#define LEVEL_FEEDBACK_DEFAULT true

	std::atomic<float>* _paramDensityThreshold = nullptr;
#define DENSITY_THRESHOLD_MIN (-36.0)
#define DENSITY_THRESHOLD_MAX 0.0
#define DENSITY_THRESHOLD_DEFAULT (-10.0)
#define DENSITY_THRESHOLD_STEP 1.0
    std::atomic<float>* _paramDensityRatio = nullptr;
#define DENSITY_RATIO_MIN 1.0
#define DENSITY_RATIO_MAX 10.0
#define DENSITY_RATIO_DEFAULT 1.0
#define DENSITY_RATIO_STEP 0.5
    std::atomic<float>* _paramDensityAttack = nullptr;
#define DENSITY_ATTACK_MIN 0.001
#define DENSITY_ATTACK_MAX 100.0
#define DENSITY_ATTACK_DEFAULT 10.0
#define DENSITY_ATTACK_STEP 10.0
    std::atomic<float>* _paramDensityRelease = nullptr;
#define DENSITY_RELEASE_MIN 10.0
#define DENSITY_RELEASE_MAX 2000.0
#define DENSITY_RELEASE_STEP 100.0
#define DENSITY_RELEASE_DEFAULT 300.0

	AudioProcessorValueTreeState _parameters;

	double _inputPreviousFreq;

	double _levelDetection;
	double _levelFilteredSideChain;
	double _levelEnvFollow;
	double _levelAmplification;
    // double _levelStandardRelease;
    double _levelCurrentRelease;

	double _densityDetection;
	double _densitySideChain;
	double _densityEnvFollow;
	double _densityAmplification;

    enum AttackReason {
        AttackNone,
        AttackPeak,
        AttackRMS
    };

    AttackReason _attackReason;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MangroveAudioProcessor)
};
