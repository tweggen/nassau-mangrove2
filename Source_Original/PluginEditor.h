/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================

typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;

class MangroveInputLoCutSlider
	: public Slider
{
public:
	MangroveInputLoCutSlider() : Slider() {}
	virtual ~MangroveInputLoCutSlider() {}

	virtual String getTextFromValue(double value) {
		if (value < (INPUT_LO_CUT_MIN+0.1)) return "Disabled";
		else return Slider::getTextFromValue(value);
	}
private:
};

class MangroveVariMuSlider
	: public Slider
{
public:
	MangroveVariMuSlider() : Slider() {}
	virtual ~MangroveVariMuSlider() {}

	virtual String getTextFromValue(double value) {
		if (value >= (LEVEL_RATIO_MAX-0.001)) return "Vari-Mu";
		else if (value <= 1.001) return "Disabled";
		else return Slider::getTextFromValue(value);
	}
private:
};

class MangroveDensityRatioSlider
	: public Slider
{
public:
	MangroveDensityRatioSlider() : Slider() {}
	virtual ~MangroveDensityRatioSlider() {}

	virtual String getTextFromValue(double value) {
		if (value <= 1.001) return "Disabled";
		else if (value >= (DENSITY_RATIO_MAX-0.001)) return "Limiter";
		else return Slider::getTextFromValue(value);
	}
private:
};

/**
*/
class MangroveAudioProcessorEditor
	: public AudioProcessorEditor
	, private Timer
{
public:
    MangroveAudioProcessorEditor (
		MangroveAudioProcessor&,
		AudioProcessorValueTreeState& vts);
    ~MangroveAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
	enum {
		MeterHeight = 200,
		MeterTop = 43,
		MeterInputX = 240,
		MeterVariX = 420,
		MeterOutputX = 600,
		LabelsWidth = 130,

		HMeterTopY = 25,
		HMeterWidth = 140,

		InputLeft = 80,
		VariLeft = 260,
		DensityLeft = 440,
		OutputLeft = 620,

		WindowWidth = 630,
		WindowHeight = 330
	};
	void timerCallback() override;

	Image _nassauLogo;

    MangroveAudioProcessor& _processor;

	AudioProcessorValueTreeState& _valueTreeState;

	Slider _sliderInputGain, _sliderInputSaturate;
	MangroveInputLoCutSlider _sliderInputLoCut;
	std::unique_ptr<SliderAttachment> _slatInputGain, _slatInputLoCut, _slatInputSaturate;

	ToggleButton _toggleInputLoCut, _toggleInputSaturate;
	std::unique_ptr<ButtonAttachment> _buatInputLoCut, _buatInputSaturate;

	Slider _sliderLevelThreshold, _sliderLevelAttack, _sliderLevelRelease;
	MangroveVariMuSlider _sliderLevelRatio;
	std::unique_ptr<SliderAttachment> _slatLevelThreshold, _slatLevelRatio, _slatLevelAttack, _slatLevelRelease;
	ToggleButton _toggleLevelLoCut, _toggleLevelTubeGain, _toggleLevelFeedback;
	std::unique_ptr<ButtonAttachment> _buatLevelLoCut, _buatLevelTubeGain, _buatLevelFeedback;

	Slider _sliderDensityThreshold, _sliderDensityAttack, _sliderDensityRelease;
	MangroveDensityRatioSlider _sliderDensityRatio;
	std::unique_ptr<SliderAttachment> _slatDensityThreshold, _slatDensityRatio, _slatDensityAttack, _slatDensityRelease;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MangroveAudioProcessorEditor)
};
