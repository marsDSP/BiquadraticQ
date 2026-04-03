#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    setSize (400, 400);

    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& name)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        addAndMakeVisible(slider);

        label.setText(name, juce::dontSendNotification);
        label.attachToComponent(&slider, false);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    setupSlider(cutoffSlider, cutoffLabel, "Cutoff");
    setupSlider(qSlider, qLabel, "Q");
    setupSlider(gainSlider, gainLabel, "Gain");

    typeBox.addItemList({"LowPass", "HighPass", "BandPass", "AllPass"}, 1);
    addAndMakeVisible(typeBox);
    typeLabel.setText("Type", juce::dontSendNotification);
    typeLabel.attachToComponent(&typeBox, false);
    addAndMakeVisible(typeLabel);

    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "cutoff", cutoffSlider);
    qAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "q", qSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "gain", gainSlider);
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, "type", typeBox);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    auto rowArea = area.removeFromTop(area.getHeight() / 2);
    
    auto sliderWidth = rowArea.getWidth() / 3;
    cutoffSlider.setBounds(rowArea.removeFromLeft(sliderWidth).reduced(10));
    qSlider.setBounds(rowArea.removeFromLeft(sliderWidth).reduced(10));
    gainSlider.setBounds(rowArea.reduced(10));

    area.removeFromTop(20); // space for labels
    typeBox.setBounds(area.removeFromTop(30).withSizeKeepingCentre(150, 30));
}
