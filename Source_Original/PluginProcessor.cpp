/*
 * Mangrove level strip.
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MangroveAudioProcessor::MangroveAudioProcessor()
	:
#ifndef JucePlugin_PreferredChannelConfigurations
     AudioProcessor (
		 BusesProperties()
        #if ! JucePlugin_IsMidiEffect
	        #if ! JucePlugin_IsSynth
		    .withInput  ("Input",  AudioChannelSet::stereo(), true)
			#endif
			.withOutput ("Output", AudioChannelSet::stereo(), true)
        #endif
     ),
#endif
	_parameters(*this, nullptr, Identifier("NassauMangrove"), {
		std::make_unique<AudioParameterFloat>(
			"inputGain", "Input Gain",
			INPUT_GAIN_MIN, INPUT_GAIN_MAX, INPUT_GAIN_DEFAULT),
		std::make_unique<AudioParameterFloat>(
			"inputLoCut","Input LoCut",
			INPUT_LO_CUT_MIN, INPUT_LO_CUT_MAX, INPUT_LO_CUT_DEFAULT),
		std::make_unique<AudioParameterFloat>(
			"inputSaturate","Input Saturate",
			INPUT_SATURATE_MIN, INPUT_SATURATE_MAX, INPUT_SATURATE_DEFAULT),
		std::make_unique<AudioParameterFloat>(
			"levelThreshold","Level Threshold",
			LEVEL_THRESHOLD_MIN, LEVEL_THRESHOLD_MAX, LEVEL_THRESHOLD_DEFAULT),
		std::make_unique<AudioParameterFloat>(
			"levelRatio","Level Ratio",
			LEVEL_RATIO_MIN, LEVEL_RATIO_MAX, LEVEL_RATIO_DEFAULT),
		std::make_unique<AudioParameterFloat>(
			"levelAttack","Level Attack",
			LEVEL_ATTACK_MIN, LEVEL_ATTACK_MAX, LEVEL_ATTACK_DEFAULT),
		std::make_unique<AudioParameterFloat>(
			"levelRelease","Level Release",
			LEVEL_RELEASE_MIN, LEVEL_RELEASE_MAX, LEVEL_RELEASE_DEFAULT),
		std::make_unique<AudioParameterBool>(
			"levelLoCut", "Level LoCut",
			LEVEL_LO_CUT_DEFAULT),
		std::make_unique<AudioParameterBool>(
			"levelTubeGain", "Level TubeGain",
			LEVEL_TUBE_GAIN_DEFAULT),
		std::make_unique<AudioParameterBool>(
			"levelFeedback", "Level Feedback",
			LEVEL_FEEDBACK_DEFAULT),
		std::make_unique<AudioParameterFloat>(
			"densityThreshold","Density Threshold",
			DENSITY_THRESHOLD_MIN, DENSITY_THRESHOLD_MAX, DENSITY_THRESHOLD_DEFAULT),
		std::make_unique<AudioParameterFloat>(
			"densityRatio","Density Ratio",
			DENSITY_RATIO_MIN, DENSITY_RATIO_MAX, DENSITY_RATIO_DEFAULT),
		std::make_unique<AudioParameterFloat>(
			"densityAttack","Density Attack",
			DENSITY_ATTACK_MIN, DENSITY_ATTACK_MAX, DENSITY_ATTACK_DEFAULT),
		std::make_unique<AudioParameterFloat>(
			"densityRelease","Density Release",
			DENSITY_RELEASE_MIN, DENSITY_RELEASE_MAX, DENSITY_RELEASE_DEFAULT),
		})
{
	_paramInputGain = _parameters.getRawParameterValue("inputGain");
	_paramInputLoCut = _parameters.getRawParameterValue("inputLoCut");
	_paramInputSaturate = _parameters.getRawParameterValue("inputSaturate");

	_paramLevelThreshold = _parameters.getRawParameterValue("levelThreshold");
	_paramLevelRatio = _parameters.getRawParameterValue("levelRatio");
	_paramLevelAttack = _parameters.getRawParameterValue("levelAttack");
	_paramLevelRelease = _parameters.getRawParameterValue("levelRelease");
	_paramLevelLoCut = _parameters.getRawParameterValue("levelLoCut");
	_paramLevelTubeGain = _parameters.getRawParameterValue("levelTubeGain");
	_paramLevelFeedback = _parameters.getRawParameterValue("levelFeedback");

	_paramDensityThreshold = _parameters.getRawParameterValue("densityThreshold");
	_paramDensityRatio = _parameters.getRawParameterValue("densityRatio");
	_paramDensityAttack = _parameters.getRawParameterValue("densityAttack");
	_paramDensityRelease = _parameters.getRawParameterValue("densityRelease");

	_levelHighpassSidechain.setCoefficients(IIRCoefficients::makeHighPass(44100., 200., 1.0));

	inputGain = 0.0;
	muOutputGain = 0.0;
	dsOutputGain = 0.0;
	muReduction = 0.125;
	currLevelReduction = 0.0;
	currDenseReduction = 0.0;
}

MangroveAudioProcessor::~MangroveAudioProcessor()
{
}

//==============================================================================
const String MangroveAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MangroveAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MangroveAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MangroveAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MangroveAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MangroveAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MangroveAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MangroveAudioProcessor::setCurrentProgram (int index)
{
}

const String MangroveAudioProcessor::getProgramName (int index)
{
    return {};
}

void MangroveAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MangroveAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	_inputPreviousFreq = -1.0;

	_levelDetection = 0.0;
	_levelFilteredSideChain = 0.0;
	_levelEnvFollow = 0.0;
	_levelAmplification = 1.0;
	_attackReason = AttackNone;
	{
		float relSoundGoodAdjustment = 1.0;
		float release = pow(0.5, 1. / ((*_paramLevelRelease * relSoundGoodAdjustment / 1000.) * 44100.));
		_levelCurrentRelease = release;
	}


	_densityDetection = 0.0;
	_densitySideChain = 0.0;
	_densityEnvFollow = 0.0;
	_densityAmplification = 1.0;

	_levelHighpassSidechain.reset();
	_levelHighpassInputLeft.reset();
	_levelHighpassInputRight.reset();

	inputGain = 0.0;
	muOutputGain = 0.0;
	dsOutputGain = 0.0;
	muReduction = 0.125;
	currLevelReduction = 0.0;
	currDenseReduction = 0.0;
}

void MangroveAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MangroveAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (/*layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     &&*/ layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MangroveAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
		// buffer.clear(i, 0, buffer.getNumSamples());
	}

	/*
	 * - Apply Input Gain
	 * - Apply LoCut
	 * - Apply Input Saturation
	 *
	 * - Leveling:
	 *   - Using the current amplification parameters, compute next output.
	 *   - Using the next output, compute new amplification parameters
	 *     - apply release to detection
	 *     - [hpf sidechain]
	 *	   - Rectify sidechain
	 *     - apply attack to sidechain
	 *     - Inject into input lpf
	 *     - If it maxes the current detection level, insert into detection
	 */

	// input Gain comes in in dB
	float localInputGain = pow(10., *_paramInputGain / 20.0);

	const float** channelInputs = (const float**)alloca(totalNumInputChannels * sizeof(const float*));
	float** channelOutputs = (float**)alloca(totalNumOutputChannels * sizeof(float*));

	const float** densityChannelInputs;

	for (int i = 0; i < totalNumInputChannels; ++i) {
		channelInputs[i] = buffer.getReadPointer(i);
	}
	for (int i = 0; i < totalNumOutputChannels; ++i) {
		channelOutputs[i] = buffer.getWritePointer(i);
	}

	densityChannelInputs = (const float**) channelOutputs;

	/*
	 * Attack/Release comes in in ms.
	 */
	// TXWTODO: We assume an srate of 44100

	/*
	 * The formula below would be correct if the attack soundgood adjustment is one.
	 */
	float attSoundGoodAdjustment = 0.1;
	float localLevelAttack = pow(0.5, 1. / ((*_paramLevelAttack * attSoundGoodAdjustment / 1000.)*44100.));


	float localLevelRatio = *_paramLevelRatio;
	bool localLevelSaturation = *_paramLevelTubeGain >= 0.5;
	bool localLevelLoCut = *_paramLevelLoCut >= 0.5;
	bool localLevelFeedback = *_paramLevelFeedback >= 0.5;
	float dbLevelThreshold = *_paramLevelThreshold;

	float absLevelThreshold = pow(10., dbLevelThreshold / 20.);
	if (localLevelLoCut) {
		// Decrease Threshold by 18db if using locut.
		// absLevelThreshold /= 8.0;
		// dbLevelThreshold -= 18.0;
	}


	double inSaturation = *_paramInputSaturate * 10.0 + 1.0;

	double absLevelDetection = _levelDetection;
	double levelFilteredSideChain = _levelFilteredSideChain;
	double absLevelEnvFollow = _levelEnvFollow;
	double absLevelAmplification = _levelAmplification;
	AttackReason attackReason = _attackReason;
	double localLevelRelease = _levelCurrentRelease;
	double levelStandardRelease;
	{
		float relSoundGoodAdjustment = 1.0;
		float release = pow(0.5, 1. / ((*_paramLevelRelease * relSoundGoodAdjustment / 1000.) * 44100.));
		levelStandardRelease = release;
	}

	double inGainAccumulator = 0.0;
	double muGainAccumulator = 0.0;
	double muAmplification = 100.0;
	double dsAmplification = 100.0;
	double dsGainAccumulator = 0.0;
	muReduction = 10.0;

	/*
	 * Using the current detection level, compute the output.
	 */
	if (totalNumInputChannels == 2 && totalNumOutputChannels == 2) {
		const float* pIn1 = channelInputs[0];
		const float* pIn2 = channelInputs[1];

		float* pOut1 = channelOutputs[0];
		float* pOut2 = channelOutputs[1];

		int l = buffer.getNumSamples();
		for (int i = 0; i < l; ++i) {
			/*
			 * Compute new output. We need it first.
			 */
			float in1 = *pIn1;
			float in2 = *pIn2;

			in1 *= localInputGain;
			in2 *= localInputGain;

			inGainAccumulator += in1 * in1 + in2 * in2;

			/*
			 * Use the amplification that resulted from the last cycle.
			 */
			float out1 = in1;
			float out2 = in2;

#if 1
			float localInputLoCutFrequency = *_paramInputLoCut;

			if (localInputLoCutFrequency > INPUT_LO_CUT_MIN+0.1) {
				/*
				 * Before saturating, apply input lowcut
				 */
				if (localInputLoCutFrequency != _inputPreviousFreq) {
					_levelHighpassInputLeft.setCoefficients(IIRCoefficients::makeHighPass(44100., localInputLoCutFrequency, 0.8));
					_levelHighpassInputRight.setCoefficients(IIRCoefficients::makeHighPass(44100., localInputLoCutFrequency, 0.8));
				}
				out1 = _levelHighpassInputLeft.processSingleSampleRaw(out1);
				out2 = _levelHighpassInputRight.processSingleSampleRaw(out2);
			}
#endif
			/*
			 * Do we need to recompute the filter?
			 */
			const float MaxIn = 4.;
#if 1
			// In Saturation.
			// (1/(1-((x/2)))-1)*2
			if (inSaturation > 1.01) {
				float divisor = 20.0 / inSaturation;
				/*
				 * Max out at division by zero location.
				 */
				if (out1 >= 0.0)
				{
					// Leave out1 as is.
				}
				else {
					out1 = (1.0 / (1.0 - ((out1 / divisor))) - 1.0)*divisor;
					if (out1 > MaxIn) out1 = MaxIn;
				}
				if (out2 >= 0.0)
				{
					// Leave out 2 as is
				}
				else {
					out2 = (1.0 / (1.0 - ((out2 / divisor))) - 1.0)*divisor;
					if (out2 > MaxIn) out2 = MaxIn;
				}
			}
#endif


#if 1
			if (localLevelRatio > 1.001) {

				out1 *= absLevelAmplification;
				out2 *= absLevelAmplification;
#if 1
				/*
				 * Shall we tube clip the input?
				 */
				if (localLevelSaturation) {
					if (out1 < 1.0) {
						if (out1 > -1.0) {
							out1 = (1 / (1 - 1 / 3))*(out1 - 1 / 3 * out1*out1*out1);
						}
						else {
							out1 = -1.0;
						}
					}
					else {
						out1 = 1.0;
					}
					if (out2 < 1.0) {
						if (out2 > -1.0) {
							out2 = (1 / (1 - 1 / 3))*(out2 - 1 / 3 * out2*out2*out2);
						}
						else {
							out2 = -1.0 /* + ((out2-1.0)/5.0) */;
						}
					}
					else {
						out2 = 1.0;
					}
				}
#endif

				// Compatibiliity: Use attack for forward varimu bug thing.
				double filterDetectionAttack =
					localLevelFeedback ? 0.9984 : localLevelAttack; // Hardcoded 10ms for feedback.

				/*
				 * Feedback the result to the sidechain input.
				 * The input is passed through a low-pass filter. The
				 * intensity of the filter is the attack rate.
				 * The output is used to pull up the envelope follower if required.
				 */

				 /*
				  * With this way of rectification, we kind of double the peaks. However, it sounds
				  * like a realistic approach to the desired threshold.
				  */
				float absSidechainInput = fabs(out1) + fabs(out2);
				float absSidechainHPFResult =
					localLevelLoCut
					? _levelHighpassSidechain.processSingleSampleRaw(absSidechainInput)
					: absSidechainInput;
				absLevelDetection = (filterDetectionAttack * absLevelDetection) + ((1. - filterDetectionAttack) * absSidechainHPFResult);

				if (localLevelFeedback) {
					/*
					 * This is feedback control. Let EnvFollow directly reflect detection, but
					 * let amplification rise slowly against one.
					 */
					absLevelAmplification =
						(localLevelRelease * absLevelAmplification)
						+ ((1.0 - localLevelRelease) * 1.0);
				}

				/*
				 * Let release release over 5ms
				 */
				double tenms = 0.9984;
				localLevelRelease = (tenms * localLevelRelease) + ((1. - tenms) * levelStandardRelease);

				{
					// Control envelope follower for foward.
					/*
					 * Pull up envelope follower if required.
					 */
					if (absLevelDetection > absLevelEnvFollow) {
						/*
						 *  Sidechain is hotter? Pull up. TODO: Add a constant.
						 */
						absLevelEnvFollow = (0.8*absLevelEnvFollow) + (0.2*absLevelDetection);
					}
					else {
						/*
						 * Side chain is softer? Release
						 */
						absLevelEnvFollow = (localLevelRelease * absLevelEnvFollow) /* + (0.0001 * 0.0) */;
					}
				}

				/*
				 * From the result of the envelope follower, compute the resulting
				 * amplification of the next input signal.
				 */

				float dbLevelEnvFollow = (absLevelEnvFollow >= 0.000001) ? (log10(absLevelEnvFollow) * 20.) : -120.;

				/*
				 * Default to the current level.
				 */
				float dbTargetLevel = dbLevelEnvFollow;
				float absTargetLevel = absLevelEnvFollow;

				/*
				 * Anything below 29.5 is considered hardknee, standard compression.
				 * Anything above is saturation.
				 *
				 * This control the attack phase of the envfollow, plus it computes the
				 * dbTargetLevel for forward control.
				 *
				 * Note, that feedback compression would use the dbTargetLevel only if absLevelDetection
				 * is higher than threshold.
				 */
				float absSidechainPeakInput = absSidechainHPFResult;
				float absSidechainDetectionInput = localLevelFeedback ? absLevelDetection : absLevelEnvFollow;
				float dbSidechainDetectionInput = (absSidechainDetectionInput >= 0.000001) ? (20.0 * log10(absSidechainDetectionInput)) : -120;


				if (localLevelRatio < (LEVEL_RATIO_MAX - 0.001)) {
#if 1
					/*
					 * This is the standard hard-knee curve.
					 */
					if (dbSidechainDetectionInput > dbLevelThreshold) {
						/*
						 * Output Level is dbLevelThreshold at dbLevelThreshold.
						 * At dbLevelThreshold+1.0 it is dbLevelThreshold + 0.3.
						 * Amplification is Output Level / inputLevel.
						 */
						float outputLevel = dbLevelThreshold
							+ (dbSidechainDetectionInput - dbLevelThreshold) / localLevelRatio;
						if (dbSidechainDetectionInput > 0.0) {
							dbTargetLevel = outputLevel; //outputLevel / dbSidechainDetectionInput;
						}
						else {
							// yields in a 1.0 amplification.
							dbTargetLevel = 0.0;
						}
					}
					else {
						dbTargetLevel = dbSidechainDetectionInput; // leave standard.
					}
					absTargetLevel = pow(10., dbTargetLevel / 20.);
#endif
				}
				else {
#if 1				
					/*
					 * This is an atan based curve, trying to emulate a vari-mu curve.
					 */
					absTargetLevel = atan(absSidechainDetectionInput / absLevelThreshold) * absLevelThreshold;
					float newReduction = absTargetLevel / absSidechainDetectionInput;
					if (newReduction < muReduction)
					{
						muReduction = newReduction;
					}
					// dbTargetLevel = (absTargetLevel >0.000001)?(log10(absTargetLevel)*20.):-120.;
#endif
				}

				AttackReason nowAttackReason = AttackNone;
				AttackReason lastAttackReason = attackReason;

				/*
				 * Now, depending on feedback or forward control, set absLevelAmplification.
				 * Notice, that absLevelAmplification will rise again with release slope if set to feedback.
				 */
				if (localLevelFeedback) {
#if 1
					/*
					 * First understand the reason of attack.
					 */
					if ((absSidechainDetectionInput*1.41) > absTargetLevel)
					{
						attackReason = AttackRMS;
					}
					else {
						if (absSidechainPeakInput > absTargetLevel)
						{
							attackReason = AttackPeak;
						}
						else
						{
							attackReason = AttackNone;
						}
					}

					/*
					 * We now need to setup the release according to this.
					 *
					 * If we had an attack reason RMS, we can reset release to original (slow)
					 * slope.
					 * If we had an attack reason Peak, we can reset release to fast.
					 * In any situation, we release the release afterwards.
					 */
					switch (attackReason)
					{
					case AttackPeak:
						if (lastAttackReason != AttackPeak)
						{
							localLevelRelease = levelStandardRelease / 4.0;
						}
						break;
					case AttackRMS:
						// localLevelRelease = levelStandardRelease;
						break;
					case AttackNone:
						// Let release release.
						break;
					}

					/*
					 * Use Attack value to bring down amplification.
					 * Depending on the reason of attack, modify the release value.
					 */ 
					if (absSidechainDetectionInput > absTargetLevel) {
						
						if (absSidechainDetectionInput >= 0.000001)
						{
#if 1
							float absTargetAmplification = absTargetLevel / absSidechainDetectionInput;
							if (absTargetAmplification < absLevelAmplification) {
								float ratio = absTargetAmplification / absLevelAmplification;
#if 1
								float maxRatio = 0.01;
								/*
								 * Limit absolute attack speed.
								 */
								if (ratio < maxRatio)
								{
									absTargetAmplification = absLevelAmplification * maxRatio;
								}
#endif
								/*
								 * We should bring down the signal.
								 */
								absLevelAmplification =
									((localLevelAttack)*absLevelAmplification)
									+ ((1.0 - localLevelAttack) * absTargetAmplification);
							}
							else {
								/*
								 * Signal is soft enough, leave it in release.
								 */
							}
#else
							float dbTargetAmplification = dbTargetLevel - dbSidechainDetectionInput;
							float dbLevelAmplification = (absLevelAmplification > 0.000001) ? (log10(absLevelAmplification) * 20.) : -120;
							if (dbTargetAmplification < dbLevelAmplification) {
								/*
								 * We should bring down the signal.
								 */
								dbLevelAmplification =
									((localLevelAttack)*dbLevelAmplification)
									+ ((1.0 - localLevelAttack) * dbTargetAmplification);
								absLevelAmplification = pow( 10., dbLevelAmplification / 20.);
							}
							else {
								/*
								 * Signal is soft enough, leave it in release.
								 */
							}
#endif
						}
						else
						{
							/*
							 * Leave the level untouched.
							 */
						}
					}
					else {
						// leave absLevelAmplification untouched, it will release.
					}
#endif
				}
				else {
#if 1
					// Forward control:
					if (absLevelEnvFollow > 0.000001) {
						/*
						 * Which it should be.
						 * Now compute amplification and note, that we only weaken the
						 * signal.
						 */
						if (absTargetLevel > absLevelEnvFollow) {
							absLevelAmplification = absTargetLevel / absLevelEnvFollow;
						}
						else {
							// absLevelAmplification = 1.0;
							// Let it release.
						}
					}
					else {
						/*
						 * No signal? Then leave it untouched.
						 */
						absLevelAmplification = 1.0;
					}
#endif
				}
			}
#endif
			*pOut1 = out1;
			*pOut2 = out2;
			muGainAccumulator += out1 * out1 + out2 * out2;

			if (absLevelAmplification < muAmplification) {
				muAmplification = absLevelAmplification;
			}

			/*
			 * When saturation is active, absLevelAmplification also influences
			 * the amount of saturation introduced when computing the
			 * output. The lower the amplification, the sooner we saturate.
			 * In this version, we just apply clipping if activated.
			 */

			++pIn1;
			++pIn2;
			++pOut1;
			++pOut2;
		}
#if 1
		inGainAccumulator = sqrt(inGainAccumulator/(2.*l));
		inputGain = inGainAccumulator;

		muGainAccumulator = sqrt(muGainAccumulator / (2.*l));
		muOutputGain = muGainAccumulator;

		currLevelReduction = muAmplification;

		_levelDetection = absLevelDetection;
		_levelFilteredSideChain = levelFilteredSideChain;
		_levelEnvFollow = absLevelEnvFollow;
		_levelAmplification = absLevelAmplification;
		_attackReason = attackReason;
		_levelCurrentRelease = localLevelRelease;

		/*
		 * All other stages just use the output buffer. We applied at least the 
		 * input gain.
		 */
		pOut1 = channelOutputs[0];
		pOut2 = channelOutputs[1];

		float localDensityRatio = *_paramDensityRatio;
		float localDensityThreshold = pow(10., *_paramDensityThreshold / 20.);
		float localDensityAttack;
		if (*_paramDensityAttack < 0.0001) {
			localDensityAttack = 0.0;
		}
		else {
			localDensityAttack = pow(0.5, 1. / ((*_paramDensityAttack / 1000.)*44100.)) / 10.0;
		}
		float localDensityRelease = pow(0.5, 1. / ((*_paramDensityRelease / 1000.)*44100.));

		if (localDensityRatio >= 1.001) {
			/*
			 * Now a classic, fast, forward feeded compression.
			 */

			/*
			 * Again, local state copies.
			 */
			double densityDetection = _densityDetection;
			double densityEnvFollow = _densityEnvFollow;
			double densitySideChain = _densitySideChain;
			double densityAmplification = _densityAmplification;

			for (int i = 0; i < l; ++i) {

				float in1 = *pOut1;
				float in2 = *pOut2;

				// TXWTODO: This is copy paste from the varimu, use a common class.
				float sidechainInput1 = fabs(in1);
				float sidechainInput2 = fabs(in2);
				float absSidechainInput;
				if (sidechainInput1 > sidechainInput2) {
					absSidechainInput = sidechainInput1;
				}
				else {
					absSidechainInput = sidechainInput2;
				}
				densityDetection = absSidechainInput; // (localDensityAttack * densityDetection) + ((1. - localDensityAttack) * absSidechainInput);

				/*
				 * Pull up envelope follower if required.
				 */
				if (densityDetection > densityEnvFollow) {
					/*
					 *  Sidechain is hotter? Pull up.
					 */
					densityEnvFollow = (localDensityAttack * densityEnvFollow) + ((1.0 - localDensityAttack) * densityDetection);
				}
				else {
					/*
					 * Side chain is softer? Release
					 */
					densityEnvFollow = (localDensityRelease * densityEnvFollow) /* + (0.0001 * 0.0) */;
				}

				/*
				 * This is the standard hard-knee curve.
				 */
				if (densityEnvFollow > localDensityThreshold) {
					/*
					* Output Level is dbLevelThreshold at dbLevelThreshold.
					* At dbLevelThreshold+1.0 it is dbLevelThreshold + 0.3.
					* Amplification is Output Level / inputLevel.
					*/
					float outputDensity;

					if (localDensityRatio < (DENSITY_RATIO_MAX - 0.001)) {
						outputDensity = localDensityThreshold
							+ (densityEnvFollow - localDensityThreshold) / localDensityRatio;
					}
					else {
						// Limit.
						outputDensity = localDensityThreshold;
					}
					if (densityEnvFollow > 0.000001) {
						// Which it must be.
						densityAmplification = outputDensity / densityEnvFollow;
					}
					else {
						densityAmplification = 1.0;
					}
				}
				else {
					densityAmplification = 1.0;
				}

				float out1 = in1 * densityAmplification;
				float out2 = in2 * densityAmplification;

				*pOut1++ = out1;
				*pOut2++ = out2;
				dsGainAccumulator += out1 * out1 + out2 * out2;
				if (densityAmplification < dsAmplification) {
					dsAmplification = densityAmplification;
				}
			}

			currDenseReduction = dsAmplification;

			dsGainAccumulator = sqrt(dsGainAccumulator / (2.*l));
			dsOutputGain = dsGainAccumulator;

			_densityDetection = densityDetection;
			_densityEnvFollow = densityEnvFollow;
			_densitySideChain = densitySideChain;
			_densityAmplification = densityAmplification;
		}
#endif
	}

}

//==============================================================================
bool MangroveAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MangroveAudioProcessor::createEditor()
{
    return new MangroveAudioProcessorEditor (*this, _parameters);
}

//==============================================================================
void MangroveAudioProcessor::getStateInformation (MemoryBlock& destData)
{
	auto state = _parameters.copyState();
	std::unique_ptr<XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void MangroveAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(_parameters.state.getType()))
			_parameters.replaceState(ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MangroveAudioProcessor();
}
