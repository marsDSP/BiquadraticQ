#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    currentSampleRate = sampleRate;
    juce::ignoreUnused (samplesPerBlock);

    for (auto& filter : filters)
    {
        filter.reset();
    }

    updateCoefficients();
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    updateCoefficients();

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto& filter = filters[static_cast<size_t>(channel)];
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            channelData[i] = filter.tick (channelData[i]);
        }
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID ("cutoff", 1), "Cutoff", juce::NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.5f), 1000.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID ("q", 1), "Q", juce::NormalisableRange<float> (0.1f, 10.0f, 0.01f), 0.707f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID ("freq", 1), "Freq", juce::NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.5f), 5000.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID ("gain", 1), "Gain", juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f));

    juce::StringArray types;
    types.add ("LowPass");
    types.add ("HighPass");
    types.add ("BandPass");
    types.add ("AllPass");
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID ("type", 1), "Filter Type", types, 0));

    return layout;
}

void AudioPluginAudioProcessor::updateCoefficients()
{
    const auto cutoff = apvts.getRawParameterValue ("cutoff")->load();
    const auto q = apvts.getRawParameterValue ("q")->load();
    const auto freq = apvts.getRawParameterValue ("freq")->load();
    const auto gain = apvts.getRawParameterValue ("gain")->load();
    const auto typeIndex = static_cast<int>(apvts.getRawParameterValue ("type")->load());

    Biquadratic::biquad<float> newCoeffs;
    const auto sr = static_cast<float>(currentSampleRate);

    switch (typeIndex)
    {
        case 0: newCoeffs = Engine::calculate_coeffs<float, Biquadratic::FilterType::LowPass>(sr, cutoff, q, gain); break;
        case 1: newCoeffs = Engine::calculate_coeffs<float, Biquadratic::FilterType::HighPass>(sr, cutoff, q, gain); break;
        case 2: newCoeffs = Engine::calculate_coeffs<float, Biquadratic::FilterType::BandPass>(sr, freq, cutoff, gain); break;
        case 3: newCoeffs = Engine::calculate_coeffs<float, Biquadratic::FilterType::AllPass>(sr, cutoff, q, gain); break;
        default: break;
    }

    Engine::update_all(filters, newCoeffs);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
