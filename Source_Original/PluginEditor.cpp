/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

extern "C" {
	extern uint8_t nassau_logo_png[];
	extern size_t nassau_logo_png_size;
};

//==============================================================================
MangroveAudioProcessorEditor::MangroveAudioProcessorEditor (
		MangroveAudioProcessor& p,
		AudioProcessorValueTreeState& vts
	)
    : AudioProcessorEditor (&p)
	, _processor (p)
	, _valueTreeState (vts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (WindowWidth, WindowHeight);
	setResizable (false, false);

	_sliderInputGain.setSliderStyle(Slider::LinearHorizontal);
	_sliderInputGain.setTextBoxStyle(Slider::TextBoxRight, false, 50, 15);
	_sliderInputGain.setRange(INPUT_GAIN_MIN, INPUT_GAIN_MAX, INPUT_GAIN_STEP);
	// _sliderInputGain.setTextValueSuffix(" dB");
	addAndMakeVisible(_sliderInputGain);
	_slatInputGain.reset(new SliderAttachment(_valueTreeState, "inputGain", _sliderInputGain));

	_sliderInputLoCut.setSliderStyle(Slider::LinearHorizontal);
	_sliderInputLoCut.setTextBoxStyle(Slider::TextBoxRight, false, 50, 15);
	_sliderInputLoCut.setRange(INPUT_LO_CUT_MIN, INPUT_LO_CUT_MAX, INPUT_LO_CUT_STEP);
	_sliderInputLoCut.setSkewFactor(0.5);
	// _sliderInputLoCut.setTextValueSuffix(" Hz");
	addAndMakeVisible(_sliderInputLoCut);
	// _sliderInputLoCut.setEnabled(false);
	_slatInputLoCut.reset(new SliderAttachment(_valueTreeState, "inputLoCut", _sliderInputLoCut));

	_sliderInputSaturate.setSliderStyle(Slider::LinearHorizontal);
	_sliderInputSaturate.setTextBoxStyle(Slider::TextBoxRight, false, 50, 15);
	_sliderInputSaturate.setRange(INPUT_SATURATE_MIN, INPUT_SATURATE_MAX, INPUT_SATURATE_STEP);
	addAndMakeVisible(_sliderInputSaturate);
	// _sliderInputSaturate.setEnabled(false);
	_slatInputSaturate.reset(new SliderAttachment(_valueTreeState, "inputSaturate", _sliderInputSaturate));

	_sliderLevelThreshold.setSliderStyle(Slider::LinearHorizontal);
	_sliderLevelThreshold.setTextBoxStyle(Slider::TextBoxRight, false, 50, 15);
	_sliderLevelThreshold.setRange(LEVEL_THRESHOLD_MIN, LEVEL_THRESHOLD_MAX, LEVEL_THRESHOLD_STEP);
	// _sliderLevelThreshold.setTextValueSuffix(" dB");
	addAndMakeVisible(_sliderLevelThreshold);
	_slatLevelThreshold.reset(new SliderAttachment(_valueTreeState, "levelThreshold", _sliderLevelThreshold));

	_sliderLevelRatio.setSliderStyle(Slider::LinearHorizontal);
	_sliderLevelRatio.setTextBoxStyle(Slider::TextBoxRight, false, 50, 15);
	_sliderLevelRatio.setRange(LEVEL_RATIO_MIN, LEVEL_RATIO_MAX, LEVEL_RATIO_STEP);
	_sliderLevelRatio.setSkewFactor(0.5);
	addAndMakeVisible(_sliderLevelRatio);
	_slatLevelRatio.reset(new SliderAttachment(_valueTreeState, "levelRatio", _sliderLevelRatio));

	_sliderLevelAttack.setSliderStyle(Slider::LinearHorizontal);
	_sliderLevelAttack.setTextBoxStyle(Slider::TextBoxRight, false, 50, 15);
	_sliderLevelAttack.setRange(LEVEL_ATTACK_MIN, LEVEL_ATTACK_MAX , LEVEL_ATTACK_STEP);
	_sliderLevelAttack.setSkewFactor(0.2);
	// _sliderLevelAttack.setTextValueSuffix(" ms");
	addAndMakeVisible(_sliderLevelAttack);
	_slatLevelAttack.reset(new SliderAttachment(_valueTreeState, "levelAttack", _sliderLevelAttack));

	_sliderLevelRelease.setSliderStyle(Slider::LinearHorizontal);
	_sliderLevelRelease.setTextBoxStyle(Slider::TextBoxRight, false, 50, 15);
	_sliderLevelRelease.setRange(LEVEL_RELEASE_MIN, LEVEL_RELEASE_MAX, LEVEL_RELEASE_STEP);
	_sliderLevelRelease.setSkewFactor(0.2);
	// _sliderLevelRelease.setTextValueSuffix(" ms");
	addAndMakeVisible(_sliderLevelRelease);
	_slatLevelRelease.reset(new SliderAttachment(_valueTreeState, "levelRelease", _sliderLevelRelease));

	// _toggleLevelLoCut
	addAndMakeVisible(_toggleLevelLoCut);
	_buatLevelLoCut.reset(new ButtonAttachment(_valueTreeState, "levelLoCut", _toggleLevelLoCut));
	// _toggleLevelTubeGain
	addAndMakeVisible(_toggleLevelTubeGain);
	_buatLevelTubeGain.reset(new ButtonAttachment(_valueTreeState, "levelTubeGain", _toggleLevelTubeGain));

	addAndMakeVisible(_toggleLevelFeedback);
	_buatLevelFeedback.reset(new ButtonAttachment(_valueTreeState, "levelFeedback", _toggleLevelFeedback));

	_sliderDensityThreshold.setSliderStyle(Slider::LinearHorizontal);
	_sliderDensityThreshold.setTextBoxStyle(Slider::TextBoxRight, false, 50, 15);
	_sliderDensityThreshold.setRange(DENSITY_THRESHOLD_MIN, DENSITY_THRESHOLD_MAX, DENSITY_THRESHOLD_STEP);
	// _sliderDensityThreshold.setTextValueSuffix(" dB");
	addAndMakeVisible(_sliderDensityThreshold);
	_slatDensityThreshold.reset(new SliderAttachment(_valueTreeState, "densityThreshold", _sliderDensityThreshold));

	_sliderDensityRatio.setSliderStyle(Slider::LinearHorizontal);
	_sliderDensityRatio.setTextBoxStyle(Slider::TextBoxRight, false, 50, 15);
	_sliderDensityRatio.setRange(DENSITY_RATIO_MIN, DENSITY_RATIO_MAX, DENSITY_RATIO_STEP);
	// _sliderDensityRatio.setTextValueSuffix(" dB");
	_sliderDensityRatio.setSkewFactor(0.5);
	addAndMakeVisible(_sliderDensityRatio);
	_slatDensityRatio.reset(new SliderAttachment(_valueTreeState, "densityRatio", _sliderDensityRatio));

	_sliderDensityAttack.setSliderStyle(Slider::LinearHorizontal);
	_sliderDensityAttack.setTextBoxStyle(Slider::TextBoxRight, false, 50, 15);
	_sliderDensityAttack.setRange(DENSITY_ATTACK_MIN, DENSITY_ATTACK_MAX, DENSITY_ATTACK_STEP);
	//_sliderDensityAttack.setTextValueSuffix(" ms");
	_sliderDensityAttack.setSkewFactor(0.2);
	addAndMakeVisible(_sliderDensityAttack);
	_slatDensityAttack.reset(new SliderAttachment(_valueTreeState, "densityAttack", _sliderDensityAttack));

	_sliderDensityRelease.setSliderStyle(Slider::LinearHorizontal);
	_sliderDensityRelease.setTextBoxStyle(Slider::TextBoxRight, false, 50, 15);
	_sliderDensityRelease.setRange(DENSITY_RELEASE_MIN, DENSITY_RELEASE_MAX, DENSITY_RELEASE_STEP);
	// _sliderDensityRelease.setTextValueSuffix(" ms");
	_sliderDensityRelease.setSkewFactor(0.2);
	addAndMakeVisible(_sliderDensityRelease);
	_slatDensityRelease.reset(new SliderAttachment(_valueTreeState, "densityRelease", _sliderDensityRelease));

	_nassauLogo = ImageFileFormat::loadFrom((const void *)nassau_logo_png, nassau_logo_png_size);
	startTimerHz(20);
}

MangroveAudioProcessorEditor::~MangroveAudioProcessorEditor()
{
}

//==============================================================================
void MangroveAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

	g.setOpacity(1.0);
	g.setColour(Colour(96, 96, 112));
	g.drawRect(5, 5, WindowWidth - 10, WindowHeight - 10, 2);
	g.drawRect(10, 10, WindowWidth - 20, WindowHeight - 20, 2);

	g.setOpacity(0.1);
	g.drawImageAt(_nassauLogo, 10, 50);
	g.setOpacity(1.0);

    g.setColour (Colours::white);
    g.setFont (15.0f);
//    g.drawFittedText ("Nasssau: Mangrove", Rectangle<int>(5, 5, getWidth()-10, 15), Justification::centred, 1);

	/*
	 * Draw meter ticks for both of the scales.
	 *
	 * We draw ticks every 6 db, adding numbers every 12 dB.
	 */
	{
		g.setFont(7.5f);
		g.setColour(Colour(92,197,255));
		for (int dB = -84; dB <= 12; dB += 6) {
			int y = ((dB + 84) * MeterHeight)/96 + MeterTop;
			g.drawHorizontalLine(y, MeterInputX - 8, MeterInputX - 1);
			g.drawHorizontalLine(y, MeterVariX - 8, MeterVariX - 1);
			if ((dB+120) % 12 == 0) {
				char s[8];
				sprintf(s, "%d", -72-dB);
				g.drawFittedText(s, Rectangle<int>(MeterInputX - 12, y - 10, 12, 10), Justification::bottomRight, 1);
				g.drawFittedText(s, Rectangle<int>(MeterVariX - 12, y - 10, 12, 10), Justification::bottomRight, 1);
			}
		}
	}

	/*
	 * Input controls.
	 */
	g.setFont(11.0f);
	g.setColour(Colours::white);
	g.drawFittedText("Gain: ", Rectangle<int>(InputLeft, 93, LabelsWidth, 15), Justification::topLeft, 1);
	g.drawFittedText("In LoCut: ", Rectangle<int>(InputLeft, 143, LabelsWidth, 15), Justification::topLeft, 1);
	g.drawFittedText("Class A Satur.: ", Rectangle<int>(InputLeft, 193, LabelsWidth, 15), Justification::topLeft, 1);

	/*
	 * Draw the input meter.
	 */
	{
		float gainHeight = log10(_processor.inputGain) * 20.0;
		if (gainHeight < -84.) gainHeight = -84.;
		if (gainHeight > 12.) gainHeight = 12.;

		gainHeight = (gainHeight + 84.) / 96. * ((float)MeterHeight);
		int height = (int)gainHeight;
		int top = MeterHeight - height;

		g.setColour(Colour(0x11, 0x22, 0x11));
		g.fillRect(MeterInputX, MeterTop, 8, top);
		g.setColour(Colour(0x55, 0x99, 0x11));
		g.fillRect(MeterInputX, MeterTop+top, 8, height);
	}

	/*
	 * Vari controls.
	 */
	g.setFont(11.0f);
	g.setColour(Colours::white);
	g.drawFittedText("Threshold: ", Rectangle<int>(VariLeft, 43, LabelsWidth, 15), Justification::topLeft, 1);
	g.drawFittedText("Ratio: ", Rectangle<int>(VariLeft, 93, LabelsWidth, 15), Justification::topLeft, 1);
	g.drawFittedText("Attack: ", Rectangle<int>(VariLeft, 143, LabelsWidth, 15), Justification::topLeft, 1);
	g.drawFittedText("Release: ", Rectangle<int>(VariLeft, 193, LabelsWidth, 15), Justification::topLeft, 1);
	g.drawFittedText("Side LoCut: ", Rectangle<int>(VariLeft, 245, 50, 15), Justification::topLeft, 1);
	g.drawFittedText("Class AA Satur: ", Rectangle<int>(VariLeft, 265, 50, 15), Justification::topLeft, 1);
	g.drawFittedText("Feedback: ", Rectangle<int>(VariLeft, 285, 50, 15), Justification::topLeft, 1);

	/*
	 * Draw vari reduction
	 */
	{
		float gainWidth = log10(_processor.currLevelReduction) * 20.0;
		if (gainWidth < -36.) gainWidth = -36.;
		if (gainWidth > 0.) gainWidth = 0.;

		int width = ((gainWidth + 36.) / 36.0 * ((float)HMeterWidth));
		int right = HMeterWidth - width;

		g.setColour(Colour(0x11, 0x22, 0x11));
		g.fillRect(VariLeft, HMeterTopY, right, 8);
		g.setColour(Colour(0x55, 0x99, 0x11));
		g.fillRect(VariLeft+right, HMeterTopY, width, 8);
	}

	/*
	 * Draw vari atan 
	 */
	{
		float x = _processor.muReduction;
		float gainWidth = (x>0.000001)?(log10(_processor.muReduction) * 20.0):-120.;
		if (gainWidth < -36.) gainWidth = -36.;
		if (gainWidth > 0.) gainWidth = 0.;

		int width = ((gainWidth + 36.) / 36.0 * ((float)HMeterWidth));
		int right = HMeterWidth - width;

		g.setColour(Colour(0x11, 0x22, 0x11));
		g.fillRect(VariLeft, HMeterTopY + 12, right, 8);
		g.setColour(Colour(0x55, 0x99, 0x11));
		g.fillRect(VariLeft + right, HMeterTopY + 12, width, 8);
	}

	/*
	 * Draw the vari meter.
	 */
	{
		float gainHeight = log10(_processor.muOutputGain) * 20.0;
		if (gainHeight < -84.) gainHeight = -84.;
		if (gainHeight > 12.) gainHeight = 12.;

		gainHeight = (gainHeight + 84.) / 96. * ((float)MeterHeight);
		int height = (int)gainHeight;
		int top = MeterHeight - height;

		g.setColour(Colour(0x11, 0x22, 0x11));
		g.fillRect(MeterVariX, MeterTop, 8, top);
		g.setColour(Colour(0x55, 0x99, 0x11));
		g.fillRect(MeterVariX, MeterTop + top, 8, height);
	}

	/*
	 * Draw density reduction
	 */
	{
		float gainWidth = log10(_processor.currDenseReduction) * 20.0;
		if (gainWidth < -36.) gainWidth = -36.;
		if (gainWidth > 0.) gainWidth = 0.;

		int width = ((gainWidth + 36.) / 36.0 * ((float)HMeterWidth));
		int right = HMeterWidth - width;

		g.setColour(Colour(0x11, 0x22, 0x11));
		g.fillRect(DensityLeft, HMeterTopY, right, 8);
		g.setColour(Colour(0x55, 0x99, 0x11));
		g.fillRect(DensityLeft + right, HMeterTopY, width, 8);
	}

	/*
	 * Draw the output meter.
	 */
	{
		float gainHeight = log10(_processor.dsOutputGain) * 20.0;
		if (gainHeight < -84.) gainHeight = -84.;
		if (gainHeight > 12.) gainHeight = 12.;

		gainHeight = (gainHeight + 84.) / 96. * ((float)MeterHeight);
		int height = (int)gainHeight;
		int top = MeterHeight - height;

		g.setColour(Colour(0x11, 0x22, 0x11));
		g.fillRect(MeterOutputX, MeterTop, 8, top);
		g.setColour(Colour(0x55, 0x99, 0x11));
		g.fillRect(MeterOutputX, MeterTop + top, 8, height);
	}

	g.setColour(Colours::white);
	g.drawFittedText("Threshold: ", Rectangle<int>(DensityLeft, 43, LabelsWidth, 15), Justification::topLeft, 1);
	g.drawFittedText("Ratio: ", Rectangle<int>(DensityLeft, 93, LabelsWidth, 15), Justification::topLeft, 1);
	g.drawFittedText("Attack: ", Rectangle<int>(DensityLeft, 143, LabelsWidth, 15), Justification::topLeft, 1);
	g.drawFittedText("Release: ", Rectangle<int>(DensityLeft, 193, LabelsWidth, 15), Justification::topLeft, 1);

}

void MangroveAudioProcessorEditor::timerCallback()
{
	// TXWTODO: Only if running...
	repaint();
}

void MangroveAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	_sliderInputGain.setBounds(InputLeft, 108, LabelsWidth, 20);
	_sliderInputLoCut.setBounds(InputLeft, 158, LabelsWidth, 20);
	_sliderInputSaturate.setBounds(InputLeft, 208, LabelsWidth, 20);

	_sliderLevelThreshold.setBounds(VariLeft, 58, LabelsWidth, 20);
	_sliderLevelRatio.setBounds(VariLeft, 108, LabelsWidth, 20);
	_sliderLevelAttack.setBounds(VariLeft, 158, LabelsWidth, 20);
	_sliderLevelRelease.setBounds(VariLeft, 208, LabelsWidth, 20);

	_toggleLevelLoCut.setBounds(330, 243, 25, 20);
	_toggleLevelTubeGain.setBounds(330, 263, 25, 20);
	_toggleLevelFeedback.setBounds(330, 283, 25, 20);

	_sliderDensityThreshold.setBounds(DensityLeft, 58, LabelsWidth, 20);
	_sliderDensityRatio.setBounds(DensityLeft, 108, LabelsWidth, 20);
	_sliderDensityAttack.setBounds(DensityLeft, 158, LabelsWidth, 20);
	_sliderDensityRelease.setBounds(DensityLeft, 208, LabelsWidth, 20);

}
