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
    setupSlider(freqSlider, freqLabel, "Freq");
    setupSlider(gainSlider, gainLabel, "Gain");

    typeBox.addItemList({"LowPass", "HighPass", "BandPass", "AllPass"}, 1);
    addAndMakeVisible(typeBox);
    typeLabel.setText("Type", juce::dontSendNotification);
    typeLabel.attachToComponent(&typeBox, false);
    addAndMakeVisible(typeLabel);

    juce::StringArray slopes { "6 dB/oct", "12 dB/oct", "24 dB/oct", "48 dB/oct", "96 dB/oct" };
    slopeBox.addItemList(slopes, 1);
    addAndMakeVisible(slopeBox);
    slopeLabel.setText("Slope", juce::dontSendNotification);
    slopeLabel.attachToComponent(&slopeBox, false);
    addAndMakeVisible(slopeLabel);

    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "cutoff", cutoffSlider);
    qAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "q", qSlider);
    freqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "freq", freqSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processorRef.apvts, "gain", gainSlider);
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, "type", typeBox);
    slopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processorRef.apvts, "slope", slopeBox);

    typeBox.addListener(this);
    updateSliderVisibility();
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    typeBox.removeListener(this);
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
    auto qFreqArea = rowArea.removeFromLeft(sliderWidth).reduced(10);
    qSlider.setBounds(qFreqArea);
    freqSlider.setBounds(qFreqArea);
    gainSlider.setBounds(rowArea.reduced(10));

    area.removeFromTop(20); // space for labels
    auto bottomArea = area.removeFromTop(30);
    typeBox.setBounds(bottomArea.removeFromLeft(bottomArea.getWidth() / 2).reduced(10).withSizeKeepingCentre(150, 30));
    slopeBox.setBounds(bottomArea.reduced(10).withSizeKeepingCentre(150, 30));
}

void AudioPluginAudioProcessorEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &typeBox)
        updateSliderVisibility();
}

void AudioPluginAudioProcessorEditor::updateSliderVisibility()
{
    const bool isBandPass = (typeBox.getSelectedId() == 3); // BandPass is 3rd item (1-indexed)
    const bool isAllPass = (typeBox.getSelectedId() == 4);
    
    qSlider.setVisible(!isBandPass);
    qLabel.setVisible(!isBandPass);
    freqSlider.setVisible(isBandPass);
    freqLabel.setVisible(isBandPass);
    
    const bool showSlope = (typeBox.getSelectedId() <= 2); // LP or HP
    slopeBox.setVisible(showSlope);
    slopeLabel.setVisible(showSlope);
}
