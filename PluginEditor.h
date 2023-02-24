/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class FinalMultiEffectEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener, public juce::ComboBox::Listener
{
public:
    FinalMultiEffectEditor (FinalMultiEffect&);
    ~FinalMultiEffectEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    FinalMultiEffect& audioProcessor;

    juce::Slider mModFreqSlider;
    juce::Slider mOverdriveSlider;
    juce::Slider mPulserFreqSlider;
    
    juce::ComboBox mModTypeComboBox;
    juce::ComboBox mDistTypeComboBox;

    juce::Label mModFreqLabel;
    juce::Label mOverdriveLabel;
    juce::Label mPulserFreqLabel;
    juce::Label mModTypeLabel;
    juce::Label mDistTypeLabel;

    void sliderValueChanged (juce::Slider* slider) override;
    void comboBoxChanged (juce::ComboBox* comboBox) override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FinalMultiEffectEditor)
};
