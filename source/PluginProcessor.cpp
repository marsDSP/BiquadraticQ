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

    for (auto& stage : filters)
    {
        for (auto& filter : stage)
            filter.reset();
    }

    updateCoefficients(0);
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

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    updateCoefficients(numSamples);

    // collect channel ptrs
    float *channelPtrs[2] = { nullptr, nullptr };
    for (int ch = 0; ch < numChannels && ch < 2; ++ch)
        channelPtrs[ch] = buffer.getWritePointer(ch);

    // process all all channels simultaneously with SIMD
    for (int i = 0; i < activeStages; ++i)
        tickSIMD<double, float, 2>(filters[i], channelPtrs, numSamples);
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
    std::unique_ptr xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID ("cutoff", 1), "Cutoff", juce::NormalisableRange (20.0f, 20000.0f, 0.1f, 0.5f), 1000.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID ("q", 1), "Q", juce::NormalisableRange (0.1f, 10.0f, 0.01f), 0.707f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID ("freq", 1), "Freq", juce::NormalisableRange (20.0f, 20000.0f, 0.1f, 0.5f), 5000.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID ("gain", 1), "Gain", juce::NormalisableRange (-24.0f, 24.0f, 0.1f), 0.0f));
    
    juce::StringArray slopes { "6 dB/oct", "12 dB/oct", "24 dB/oct", "48 dB/oct", "96 dB/oct" };
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID ("slope", 1), "Slope", slopes, 1));

    juce::StringArray types;
    types.add ("LowPass");
    types.add ("HighPass");
    types.add ("BandPass");
    types.add ("AllPass");
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID ("type", 1), "Filter Type", types, 0));

    return layout;
}

void AudioPluginAudioProcessor::updateCoefficients(int numSamples)
{
    const auto cutoff = apvts.getRawParameterValue ("cutoff")->load();
    const auto q = apvts.getRawParameterValue ("q")->load();
    const auto freq = apvts.getRawParameterValue ("freq")->load();
    const auto gain = apvts.getRawParameterValue ("gain")->load();
    const auto typeIndex = static_cast<int>(apvts.getRawParameterValue ("type")->load());
    const auto slopeIndex = static_cast<int>(apvts.getRawParameterValue ("slope")->load());

    const bool typeChanged = (typeIndex != lastType);
    const bool slopeChanged = (slopeIndex != lastSlope);

    // skip recalc if nothing changed
    if (cutoff == lastCutoff && q == lastQ && freq == lastFreq &&
        gain == lastGain && !typeChanged && !slopeChanged)
        return;

    lastCutoff = cutoff;
    lastQ = q;
    lastFreq = freq;
    lastGain = gain;
    lastType = typeIndex;
    lastSlope = slopeIndex;

    const auto sr = static_cast<float>(currentSampleRate);

    // if topology changes, reset state to avoid transients/explosions
    // and use immediate update instead of smoothing
    const bool immediateUpdate = typeChanged || slopeChanged;
    if (immediateUpdate)
    {
        for (auto& stage : filters)
        {
            for (auto& filter : stage)
                filter.reset();
        }
    }

    if (typeIndex == 0 || typeIndex == 1) // LowPass or HighPass
    {
        const int order = 1 << lastSlope;
        activeStages = (order + 1) / 2;

        Base::LayoutBase<double, 16> digital;

        // clamp cutoff to avoid singularities at Nyquist
        const double sr_d = static_cast<double>(sr);
        const double cutoff_d = static_cast<double>(cutoff);
        const double clampedCutoff = std::clamp(cutoff_d, 0.1, sr_d * 0.49);
        const double normalizedW = (2.0 * std::numbers::pi_v<double> * clampedCutoff) / sr_d;
        const double prewarpedW = 2.0 * std::tan(normalizedW * 0.5);

        // Scale at DC for LP, at Nyquist for HP
        digital.set_w(typeIndex == 0 ? 0.0 : std::numbers::pi_v<double>);
        digital.set_gain(1.0);

        for (int k = 1; k <= order / 2; ++k)
        {
            const double angle = std::numbers::pi_v<double> * (0.5 + (2.0 * static_cast<double>(k) + static_cast<double>(order) - 1.0) / (2.0 * static_cast<double>(order)));
            std::complex<double> s(std::cos(angle), std::sin(angle));
            std::complex<double> spole = s * (prewarpedW * 0.5);
            std::complex<double> zpole = (1.0 + spole) / (1.0 - spole);

            if (typeIndex == 0) digital.insert_conjugate(zpole, -1.0);
            else digital.insert_conjugate(zpole, 1.0);
        }

        if (order % 2 == 1)
        {
            double spole = -1.0 * (prewarpedW * 0.5);
            double zpole = (1.0 + spole) / (1.0 - spole);
            if (typeIndex == 0) digital.insert(zpole, -1.0);
            else digital.insert(zpole, 1.0);
        }

        auto casc = Engine::make_cascade(digital);
        for (int i = 0; i < (int)casc.size() && i < 8; ++i)
        {
            if (immediateUpdate)
                Engine::update_all(filters[i], casc[i]);
            else
                Engine::smooth_all(filters[i], casc[i], numSamples);
        }
    }
    else
    {
        const double sr_d = static_cast<double>(sr);
        const double freq_d = static_cast<double>(freq);
        const double cutoff_d = static_cast<double>(cutoff);
        const double q_d = static_cast<double>(q);
        const double gain_d = static_cast<double>(gain);

        activeStages = 1;
        biquad<double> newCoeffs;
        switch (typeIndex)
        {
            case 2: newCoeffs = Engine::calculate_coeffs<double,
                                FilterType::BandPass>(sr_d, freq_d, cutoff_d, gain_d);
                                break;

            case 3: newCoeffs = Engine::calculate_coeffs<double,
                                FilterType::AllPass>(sr_d, cutoff_d, q_d, gain_d);
                                break;
            default: break;
        }

        if (immediateUpdate)
            Engine::update_all(filters[0], newCoeffs);
        else
            Engine::smooth_all(filters[0], newCoeffs, numSamples);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
