#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include <memory>
#include "dsp/filters/filter_coeffs.h"
#include "dsp/filters/filter_engine.h"
#include "dsp/filters/filter_iir.h"
#include "dsp/filters/filter_utils.h"

using namespace MarsDSP::Filters;

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor,
                                        private juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

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

    juce::AudioProcessorValueTreeState apvts;

private:
    using MonoChain = BiquadCascade::cascade<float, 1>;

    struct FilterParams
    {
        Biquadratic::FilterType type { Biquadratic::FilterType::LowPass };
        float cutoffHz { 1000.0f };
        float q { 0.707f };
        float bandCenterHz { 5000.0f };
        float bandWidthControlHz { 1000.0f };
        float gainDb { 0.0f };
    };

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    FilterParams readParams() const;
    Biquadratic::biquad<float> makeStage(const FilterParams& params) const;
    MonoChain buildChain(const FilterParams& params) const;
    void rebuildChains();
    void parameterChanged(const juce::String&, float) override;

    std::array<MonoChain, 2> chains;
    double currentSampleRate { 44100.0 };
    std::atomic<bool> chainDirty { true };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
