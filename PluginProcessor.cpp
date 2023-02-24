/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FinalMultiEffect::FinalMultiEffect()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    DBG ("Processor constructor called");

    mModFreqSliderValue = MOD_FREQ_INIT;
    mOverdriveSliderValue = OVERDRIVE_INIT;
    mPulserFreqSliderValue = PULSER_FREQ_INIT;

    mModAngleDelta = 0.0;
    mModCurrentAngle[0] = 0.0;
    mModCurrentAngle[1] = 0.0;

    mPulserAngleDelta = 0.0;
    mPulserCurrentAngle[0] = 0.0;
    mPulserCurrentAngle[1] = 0.0;

    mAmFlag = false;
    mSoftClipFlag = false;
}

FinalMultiEffect::~FinalMultiEffect()
{
}


//======== GET/SET FUNCTIONS =====================================================
modType FinalMultiEffect::getModType()
{
    if (mAmFlag)
        return am;
    else
        return rm;
}

void FinalMultiEffect::setModType (modType type)
{
    switch (type)
    {
        case rm:
            mAmFlag = false;
            break;
        
        case am:
            mAmFlag = true;
            break;
        
        // if an unexpected value comes in for "type" argument, default to RM
        default:
            mAmFlag = false;
            break;
    }
    
    if (mAmFlag)
        DBG ("mAmFlag: ON");
    else
        DBG ("mAmFlag: OFF");

}

distType FinalMultiEffect::getDistType()
{
    if (mSoftClipFlag)
        return soft;
    else
        return hard;
}

void FinalMultiEffect::setDistType (distType type)
{
    switch (type)
    {
        case soft:
            mSoftClipFlag = true;
            break;
        
        case hard:
            mSoftClipFlag = false;
            break;
        
        // if an unexpected value comes in for "type" argument, default to hard clipping
        default:
            mSoftClipFlag = false;
            break;
    }
    
    if (mSoftClipFlag)
        DBG ("mSoftClipFlag: ON");
    else
        DBG ("mSoftClipFlag: OFF");
}

double FinalMultiEffect::getModFreq()
{
    return mModFreqSliderValue;
}

void FinalMultiEffect::setModFreq (double freq)
{
    // limit the modulation frequency to 0 - MOD_FREQ_LIMIT kHz
    freq = (freq < 0.0) ? 0.0 : freq;
    freq = (freq > MOD_FREQ_LIMIT) ? MOD_FREQ_LIMIT : freq;
    
    mModFreqSliderValue = freq;
    
    auto cyclesPerSample = mModFreqSliderValue / mSampleRate;
    
    mModAngleDelta = cyclesPerSample * juce::MathConstants<double>::twoPi;
    
    DBG ("mModFreqSliderValue: " + juce::String (mModFreqSliderValue) + ", mModAngleDelta: " + juce::String (mModAngleDelta));
}

double FinalMultiEffect::getOverdrive()
{
    return mOverdriveSliderValue;
}

void FinalMultiEffect::setOverdrive (double value)
{
    // limit the overdrive gain factor to 1 - OVERDRIVE_LIMIT
    value = (value <= 1.0) ? 1.0 : value;
    value = (value > OVERDRIVE_LIMIT) ? OVERDRIVE_LIMIT : value;
    
    mOverdriveSliderValue = value;
    DBG ("mOverdriveSliderValue: " + juce::String (mOverdriveSliderValue));
}

double FinalMultiEffect::getPulserFreq()
{
    return mPulserFreqSliderValue;
}

void FinalMultiEffect::setPulserFreq (double freq)
{
    // limit the pulser frequency to 0 - PULSER_FREQ_LIMIT Hz
    freq = (freq < 0.0) ? 0.0 : freq;
    freq = (freq > PULSER_FREQ_LIMIT) ? PULSER_FREQ_LIMIT : freq;
    
    mPulserFreqSliderValue = freq;
    
    auto cyclesPerSample = mPulserFreqSliderValue / mSampleRate;
    
    mPulserAngleDelta = cyclesPerSample * juce::MathConstants<double>::twoPi;
    
    DBG ("mPulserFreqSliderValue: " + juce::String (mPulserFreqSliderValue) + ", mPulserAngleDelta: " + juce::String (mPulserAngleDelta));
}

//======== CUSTOM MEMBER FUNCTIONS =====================================================
double FinalMultiEffect::getLfoSample (double angle)
{
    double sample = std::sin (angle);
    return sample;
    
}

void FinalMultiEffect::advancedLfoPhase (double* angle, double delta)
{
    *angle += delta;
    *angle = std::fmod(*angle, juce::MathConstants<double>::twoPi);
}

double FinalMultiEffect::reRangeLfoSample (double sample)
{
    sample += 1.0;
    sample *= 0.5;

    return sample;
}

void FinalMultiEffect::doModulation (juce::AudioBuffer<float>& buffer, int channel)
{    
    auto* channelData = buffer.getWritePointer (channel);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        if (mModFreqSliderValue > 0.0)
        {
            auto lfoSample = getLfoSample (mModCurrentAngle[channel]);

            advancedLfoPhase (&mModCurrentAngle[channel], mModAngleDelta);

            if (mAmFlag)
                lfoSample = reRangeLfoSample (lfoSample);

            channelData[sample] *= lfoSample;
        }
    }
}

void FinalMultiEffect::doDistortion (juce::AudioBuffer<float>& buffer, int channel, float inputPeak)
{
    auto* channelData = buffer.getWritePointer (channel);
    float postDistPeak = buffer.getMagnitude(channel, 0, buffer.getNumSamples());

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        if (mOverdriveSliderValue > 1.0)
        {
            // step 1: overdrive
            channelData[sample] *= mOverdriveSliderValue;
            
            // step 2: limit the signal
            if (mSoftClipFlag)
            {
                // do soft clipping
                channelData[sample] = std::tanh (channelData[sample]);
            }
            else
            {
                // do hard clipping
                if (channelData[sample] > 1.0)
                    channelData[sample] = 1.0;
                else if (channelData[sample] < -1.0)
                    channelData[sample] = -1.0;
            }
        }
    }
    
    postDistPeak = buffer.getMagnitude (channel, 0, buffer.getNumSamples());

    // protect against division by zero when there's silence input
    if (postDistPeak == 0.0)
        postDistPeak = 1.0;
    
    // set the SmoothedValue target to be the ratio of the pre/post peak samples so that the output of the distortion is at the same level as the original signal
    mDistGainFactor.setTargetValue (inputPeak / postDistPeak);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        channelData[sample] *= mDistGainFactor.getNextValue();
}

void FinalMultiEffect::doPulsing (juce::AudioBuffer<float>& buffer, int channel)
{
    auto* channelData = buffer.getWritePointer (channel);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        if (mPulserFreqSliderValue > 0.0)
        {
            auto lfoSample = getLfoSample (mPulserCurrentAngle[channel]);

            advancedLfoPhase (&mPulserCurrentAngle[channel], mPulserAngleDelta);

            lfoSample = reRangeLfoSample (lfoSample);

            channelData[sample] *= lfoSample;
        }
    }
}

//==============================================================================
const juce::String FinalMultiEffect::getName() const
{
    return JucePlugin_Name;
}

bool FinalMultiEffect::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FinalMultiEffect::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FinalMultiEffect::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double FinalMultiEffect::getTailLengthSeconds() const
{
    return 0.0;
}

int FinalMultiEffect::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FinalMultiEffect::getCurrentProgram()
{
    return 0;
}

void FinalMultiEffect::setCurrentProgram (int index)
{
}

const juce::String FinalMultiEffect::getProgramName (int index)
{
    return {};
}

void FinalMultiEffect::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void FinalMultiEffect::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    DBG ("prepareToPlay() called");

    mSampleRate = sampleRate;
    
    // set param values before playback
    setModFreq(mModFreqSliderValue);
    setOverdrive(mOverdriveSliderValue);
    setPulserFreq(mPulserFreqSliderValue);

    // 100ms smoothing on automatic gain adjustment for distortion DSP
    mDistGainFactor.reset (mSampleRate, 0.1);
    mDistGainFactor.setCurrentAndTargetValue (1.0);
}

void FinalMultiEffect::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FinalMultiEffect::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void FinalMultiEffect::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        // get the peak amplitude of the input signal before processing so we can do automatic gain matching after the distortion DSP
        double inputPeak = buffer.getMagnitude (channel, 0, buffer.getNumSamples());
                
        doModulation (buffer, channel);
        doDistortion (buffer, channel, inputPeak);
        doPulsing (buffer, channel);
    }
}

//==============================================================================
bool FinalMultiEffect::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FinalMultiEffect::createEditor()
{
    return new FinalMultiEffectEditor (*this);
}

//==============================================================================
void FinalMultiEffect::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FinalMultiEffect::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FinalMultiEffect();
}
