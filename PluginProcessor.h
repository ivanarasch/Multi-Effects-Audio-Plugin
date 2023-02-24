/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define MOD_FREQ_INIT 100.0
#define MOD_FREQ_LIMIT 5000.0

#define OVERDRIVE_INIT 1.0
#define OVERDRIVE_LIMIT 50.0

#define PULSER_FREQ_INIT 2.0
#define PULSER_FREQ_LIMIT 10.0

// declaring enums outside of class definition so that both the Editor and Processor can use them
enum modType
{
    rm = 1,
    am
};

enum distType
{
    soft = 1,
    hard
};


//==============================================================================
/**
*/
class FinalMultiEffect  : public juce::AudioProcessor
{
public:
    //==============================================================================
    FinalMultiEffect();
    ~FinalMultiEffect() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    modType getModType();
    void setModType (modType type);
    
    distType getDistType();
    void setDistType (distType type);

    double getModFreq();
    void setModFreq (double freq);

    double getOverdrive();
    void setOverdrive (double value);
    
    double getPulserFreq();
    void setPulserFreq (double freq);
    
private:

    double mSampleRate;

    double mModFreqSliderValue;
    double mOverdriveSliderValue;
    double mPulserFreqSliderValue;

    double mModAngleDelta;
    // use an array so we have a dedicated angle value for both the L and R channels
    double mModCurrentAngle[2];
    
    double mPulserAngleDelta;
    double mPulserCurrentAngle[2];

    bool mAmFlag;
    bool mSoftClipFlag;
    
    juce::SmoothedValue<double> mDistGainFactor;

    double getLfoSample (double angle);
    void advancedLfoPhase (double* angle, double delta);
    double reRangeLfoSample (double sample);

    void doModulation (juce::AudioBuffer<float>& buffer, int channel);
    void doDistortion (juce::AudioBuffer<float>& buffer, int channel, float peakSample);
    void doPulsing (juce::AudioBuffer<float>& buffer, int channel);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FinalMultiEffect)
};
