#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    for (auto* parameterId : { "cutoff", "q", "freq", "gain", "type" })
        apvts.addParameterListener (parameterId, this);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    for (auto* parameterId : { "cutoff", "q", "freq", "gain", "type" })
        apvts.removeParameterListener (parameterId, this);
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
    currentSampleRate = sampleRate;
    juce::ignoreUnused (samplesPerBlock);

    rebuildChains();
    chainDirty.store (false, std::memory_order_release);

    for (auto& chain : chains)
        chain.reset();
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

    if (chainDirty.exchange (false, std::memory_order_acq_rel))
        rebuildChains();

    const auto channelsToProcess = std::min<int> (buffer.getNumChannels(), static_cast<int> (chains.size()));

    for (int channel = 0; channel < channelsToProcess; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto& chain = chains[static_cast<size_t> (channel)];

        for (int i = 0; i < buffer.getNumSamples(); ++i)
            channelData[i] = chain.tick (channelData[i]);
    }

    for (int channel = channelsToProcess; channel < buffer.getNumChannels(); ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());
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

    chainDirty.store (true, std::memory_order_release);
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

AudioPluginAudioProcessor::FilterParams AudioPluginAudioProcessor::readParams() const
{
    FilterParams params;

    params.cutoffHz = apvts.getRawParameterValue ("cutoff")->load();
    params.q = apvts.getRawParameterValue ("q")->load();
    params.bandCenterHz = apvts.getRawParameterValue ("freq")->load();
    params.bandWidthControlHz = apvts.getRawParameterValue ("cutoff")->load();
    params.gainDb = apvts.getRawParameterValue ("gain")->load();

    switch (static_cast<int> (apvts.getRawParameterValue ("type")->load()))
    {
        case 0: params.type = Biquadratic::FilterType::LowPass; break;
        case 1: params.type = Biquadratic::FilterType::HighPass; break;
        case 2: params.type = Biquadratic::FilterType::BandPass; break;
        case 3: params.type = Biquadratic::FilterType::AllPass; break;
        default: break;
    }

    return params;
}

Biquadratic::biquad<float> AudioPluginAudioProcessor::makeStage (const FilterParams& params) const
{
    const auto sr = static_cast<float>(currentSampleRate);

    switch (params.type)
    {
        case Biquadratic::FilterType::LowPass:
            return Coeffs::CoeffCalc<float, Biquadratic::FilterType::LowPass>{} (sr, params.cutoffHz, params.q, params.gainDb);
        case Biquadratic::FilterType::HighPass:
            return Coeffs::CoeffCalc<float, Biquadratic::FilterType::HighPass>{} (sr, params.cutoffHz, params.q, params.gainDb);
        case Biquadratic::FilterType::BandPass:
            return Coeffs::CoeffCalc<float, Biquadratic::FilterType::BandPass>{} (sr, params.bandCenterHz, params.bandWidthControlHz, params.gainDb);
        case Biquadratic::FilterType::AllPass:
            return Coeffs::CoeffCalc<float, Biquadratic::FilterType::AllPass>{} (sr, params.cutoffHz, params.q, params.gainDb);
        default:
            return {};
    }
}

AudioPluginAudioProcessor::MonoChain AudioPluginAudioProcessor::buildChain (const FilterParams& params) const
{
    MonoChain chain;
    chain.clear();
    chain.emplace_back (makeStage (params));
    return chain;
}

void AudioPluginAudioProcessor::rebuildChains()
{
    const auto params = readParams();

    for (auto& chain : chains)
        chain = buildChain (params);
}

void AudioPluginAudioProcessor::parameterChanged (const juce::String&, float)
{
    chainDirty.store (true, std::memory_order_release);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
