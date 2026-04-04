#pragma once

#include <memory>
#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                      private juce::ComboBox::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;

private:
    AudioPluginAudioProcessor& processorRef;

    juce::Slider cutoffSlider, qSlider, freqSlider, gainSlider;
    juce::ComboBox typeBox, slopeBox;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment, qAttachment, freqAttachment, gainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment, slopeAttachment;

    juce::Label cutoffLabel, qLabel, freqLabel, gainLabel, typeLabel, slopeLabel;

    void updateSliderVisibility();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
