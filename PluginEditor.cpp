/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FinalMultiEffectEditor::FinalMultiEffectEditor (FinalMultiEffect& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);

    // SLIDERS
    mModFreqSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    mModFreqSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    mModFreqSlider.setRange (0.0, MOD_FREQ_LIMIT);
    mModFreqSlider.setValue (audioProcessor.getModFreq());
    addAndMakeVisible (&mModFreqSlider);
    mModFreqSlider.addListener (this);
    
    mOverdriveSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    mOverdriveSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    mOverdriveSlider.setRange (0.0, OVERDRIVE_LIMIT);
    mOverdriveSlider.setValue (audioProcessor.getOverdrive());
    addAndMakeVisible (&mOverdriveSlider);
    mOverdriveSlider.addListener (this);

    mPulserFreqSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    mPulserFreqSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    mPulserFreqSlider.setRange (0.0, PULSER_FREQ_LIMIT);
    mPulserFreqSlider.setValue (audioProcessor.getPulserFreq());
    addAndMakeVisible (&mPulserFreqSlider);
    mPulserFreqSlider.addListener (this);
    
    // COMBO-BOXES
    mModTypeComboBox.addItem ("RM", rm);
    mModTypeComboBox.addItem ("AM", am);
    mModTypeComboBox.setSelectedId (audioProcessor.getModType());
    addAndMakeVisible(&mModTypeComboBox);
    mModTypeComboBox.addListener(this);

    mDistTypeComboBox.addItem ("Soft", soft);
    mDistTypeComboBox.addItem ("Hard", hard);
    mDistTypeComboBox.setSelectedId (audioProcessor.getDistType());
    addAndMakeVisible (&mDistTypeComboBox);
    mDistTypeComboBox.addListener (this);
    
    // LABELS
    mModFreqLabel.setText ("Mod Frequency", juce::NotificationType::dontSendNotification);
    mModFreqLabel.attachToComponent (&mModFreqSlider, true);
    addAndMakeVisible (&mModFreqLabel);

    mOverdriveLabel.setText ("Overdrive", juce::NotificationType::dontSendNotification);
    mOverdriveLabel.attachToComponent (&mOverdriveSlider, true);
    addAndMakeVisible (&mOverdriveLabel);
    
    mPulserFreqLabel.setText ("Pulser Freq", juce::NotificationType::dontSendNotification);
    mPulserFreqLabel.attachToComponent (&mPulserFreqSlider, true);
    addAndMakeVisible (&mPulserFreqLabel);
    
    mModTypeLabel.setText ("Mod Type", juce::NotificationType::dontSendNotification);
    mModTypeLabel.attachToComponent (&mModTypeComboBox, true);
    addAndMakeVisible (&mModTypeLabel);

    mDistTypeLabel.setText ("Dist Type", juce::NotificationType::dontSendNotification);
    mDistTypeLabel.attachToComponent (&mDistTypeComboBox, true);
    addAndMakeVisible (&mDistTypeLabel);
}

FinalMultiEffectEditor::~FinalMultiEffectEditor()
{
    mModFreqSlider.removeListener (this);
    mOverdriveSlider.removeListener (this);
    mPulserFreqSlider.removeListener (this);
    mModTypeComboBox.removeListener (this);
    mModFreqSlider.removeListener (this);
}

void FinalMultiEffectEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &mModFreqSlider)
    {
        audioProcessor.setModFreq(mModFreqSlider.getValue());
        //= mModFreqSlider.getValue();
    }
    else if (slider == &mOverdriveSlider)
    {
        audioProcessor.setOverdrive (mOverdriveSlider.getValue());
    }
    else if (slider == &mPulserFreqSlider)
    {
        audioProcessor.setPulserFreq (mPulserFreqSlider.getValue());
    }
}

void FinalMultiEffectEditor::comboBoxChanged (juce::ComboBox* comboBox)
{
    if (comboBox == &mModTypeComboBox)
    {
        switch (comboBox->getSelectedId())
        {
            case rm:
                audioProcessor.setModType (rm);
                break;
            case am:
                audioProcessor.setModType (am);
                break;

            // in case the ComboBox is set to something unknown, default to RM
            default:
                audioProcessor.setModType (rm);
                break;
        }
    }
    else if (comboBox == &mDistTypeComboBox)
    {
        switch (comboBox->getSelectedId())
        {
            case soft:
                audioProcessor.setDistType (soft);
                break;
            case hard:
                audioProcessor.setDistType (hard);
                break;
                
            // in case the ComboBox is set to something unknown, default to hard clipping
            default:
                audioProcessor.setDistType (hard);
                break;
        }
    }
}

//==============================================================================
void FinalMultiEffectEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawFittedText ("Multi-FX Final!", getLocalBounds(), juce::Justification::centredTop, 1);
}

void FinalMultiEffectEditor::resized()
{
    int spacing = 30;
    int sliderWidth = 200;
    int sliderHeight = 50;
    int comboWidth = 100;
    int comboHeight = 40;
    // make the horizontal and vertical starting points 100 pixels back than the halfway point of the window
    float xMargin = getWidth() / 2.0f - 100;
    float yMargin = getHeight() / 2.0f - 100;

    mModFreqSlider.setBounds (xMargin, yMargin, sliderWidth, sliderHeight);
    mOverdriveSlider.setBounds (xMargin, yMargin + spacing, sliderWidth, sliderHeight);
    mPulserFreqSlider.setBounds (xMargin, yMargin + spacing * 2, sliderWidth, sliderHeight);
    
    mDistTypeComboBox.setBounds (xMargin, yMargin + spacing * 6, comboWidth, comboHeight);
    mModTypeComboBox.setBounds (xMargin, yMargin + spacing * 4, comboWidth, comboHeight);
}
