/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    left_chain.prepare(spec);
    right_chain.prepare(spec);

    ChainSettings chain_settings = GetChainSettings(apvst);

    auto bell_coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 
                                                                                 chain_settings.bell_freq, 
                                                                                 chain_settings.bell_q,
                                                                                 juce::Decibels::decibelsToGain(chain_settings.bell_gain_in_db));
    *left_chain.get<ChainPositions::Bell>().coefficients  = *bell_coefficients;
    *right_chain.get<ChainPositions::Bell>().coefficients = *bell_coefficients;

    auto cut_coefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chain_settings.hp_freq,
                                                                                                        sampleRate,
                                                                                                        2 * (chain_settings.hp_slope + 1));
    auto& left_high_pass = left_chain.get<ChainPositions::HighPass>();

    left_high_pass.setBypassed<0>(true);
    left_high_pass.setBypassed<1>(true);
    left_high_pass.setBypassed<2>(true);
    left_high_pass.setBypassed<3>(true);

    switch (chain_settings.hp_slope) {
        case slope_12:
            *left_high_pass.get<0>().coefficients = *cut_coefficients[0];
            left_high_pass.setBypassed<0>(false);
            break;
        case slope_24:
            *left_high_pass.get<0>().coefficients = *cut_coefficients[0];
            left_high_pass.setBypassed<0>(false);
            *left_high_pass.get<1>().coefficients = *cut_coefficients[1];
            left_high_pass.setBypassed<1>(false);
            break;
        case slope_36:
            *left_high_pass.get<0>().coefficients = *cut_coefficients[0];
            left_high_pass.setBypassed<0>(false);
            *left_high_pass.get<1>().coefficients = *cut_coefficients[1];
            left_high_pass.setBypassed<1>(false);
            *left_high_pass.get<2>().coefficients = *cut_coefficients[2];
            left_high_pass.setBypassed<2>(false);
            break;
        case slope_48:
            *left_high_pass.get<0>().coefficients = *cut_coefficients[0];
            left_high_pass.setBypassed<0>(false);
            *left_high_pass.get<1>().coefficients = *cut_coefficients[1];
            left_high_pass.setBypassed<1>(false);
            *left_high_pass.get<2>().coefficients = *cut_coefficients[2];
            left_high_pass.setBypassed<2>(false);
            *left_high_pass.get<3>().coefficients = *cut_coefficients[3];
            left_high_pass.setBypassed<3>(false);
            break;
    }

    auto& right_high_pass = right_chain.get<ChainPositions::HighPass>();

    right_high_pass.setBypassed<0>(true);
    right_high_pass.setBypassed<1>(true);
    right_high_pass.setBypassed<2>(true);
    right_high_pass.setBypassed<3>(true);

    switch (chain_settings.hp_slope) {
    case slope_12:
        *right_high_pass.get<0>().coefficients = *cut_coefficients[0];
        right_high_pass.setBypassed<0>(false);
        break;
    case slope_24:
        *right_high_pass.get<0>().coefficients = *cut_coefficients[0];
        right_high_pass.setBypassed<0>(false);
        *right_high_pass.get<1>().coefficients = *cut_coefficients[1];
        right_high_pass.setBypassed<1>(false);
        break;
    case slope_36:
        *right_high_pass.get<0>().coefficients = *cut_coefficients[0];
        right_high_pass.setBypassed<0>(false);
        *right_high_pass.get<1>().coefficients = *cut_coefficients[1];
        right_high_pass.setBypassed<1>(false);
        *right_high_pass.get<2>().coefficients = *cut_coefficients[2];
        right_high_pass.setBypassed<2>(false);
        break;
    case slope_48:
        *right_high_pass.get<0>().coefficients = *cut_coefficients[0];
        right_high_pass.setBypassed<0>(false);
        *right_high_pass.get<1>().coefficients = *cut_coefficients[1];
        right_high_pass.setBypassed<1>(false);
        *right_high_pass.get<2>().coefficients = *cut_coefficients[2];
        right_high_pass.setBypassed<2>(false);
        *right_high_pass.get<3>().coefficients = *cut_coefficients[3];
        right_high_pass.setBypassed<3>(false);
        break;
    }
}

void SimpleEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
#endif

void SimpleEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Refactor this 
    ChainSettings chain_settings = GetChainSettings(apvst);

    auto bell_coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                 chain_settings.bell_freq,
                                                                                 chain_settings.bell_q,
                                                                                 juce::Decibels::decibelsToGain(chain_settings.bell_gain_in_db));
    *left_chain.get<ChainPositions::Bell>().coefficients = *bell_coefficients;
    *right_chain.get<ChainPositions::Bell>().coefficients = *bell_coefficients;

    auto cut_coefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chain_settings.hp_freq,
                                                                                                        getSampleRate(),
                                                                                                        2 * (chain_settings.hp_slope + 1));
    auto& left_high_pass = left_chain.get<ChainPositions::HighPass>();

    left_high_pass.setBypassed<0>(true);
    left_high_pass.setBypassed<1>(true);
    left_high_pass.setBypassed<2>(true);
    left_high_pass.setBypassed<3>(true);

    switch (chain_settings.hp_slope) {
    case slope_12:
        *left_high_pass.get<0>().coefficients = *cut_coefficients[0];
        left_high_pass.setBypassed<0>(false);
        break;
    case slope_24:
        *left_high_pass.get<0>().coefficients = *cut_coefficients[0];
        left_high_pass.setBypassed<0>(false);
        *left_high_pass.get<1>().coefficients = *cut_coefficients[1];
        left_high_pass.setBypassed<1>(false);
        break;
    case slope_36:
        *left_high_pass.get<0>().coefficients = *cut_coefficients[0];
        left_high_pass.setBypassed<0>(false);
        *left_high_pass.get<1>().coefficients = *cut_coefficients[1];
        left_high_pass.setBypassed<1>(false);
        *left_high_pass.get<2>().coefficients = *cut_coefficients[2];
        left_high_pass.setBypassed<2>(false);
        break;
    case slope_48:
        *left_high_pass.get<0>().coefficients = *cut_coefficients[0];
        left_high_pass.setBypassed<0>(false);
        *left_high_pass.get<1>().coefficients = *cut_coefficients[1];
        left_high_pass.setBypassed<1>(false);
        *left_high_pass.get<2>().coefficients = *cut_coefficients[2];
        left_high_pass.setBypassed<2>(false);
        *left_high_pass.get<3>().coefficients = *cut_coefficients[3];
        left_high_pass.setBypassed<3>(false);
        break;
    }

    auto& right_high_pass = right_chain.get<ChainPositions::HighPass>();

    right_high_pass.setBypassed<0>(true);
    right_high_pass.setBypassed<1>(true);
    right_high_pass.setBypassed<2>(true);
    right_high_pass.setBypassed<3>(true);

    switch (chain_settings.hp_slope) {
    case slope_12:
        *right_high_pass.get<0>().coefficients = *cut_coefficients[0];
        right_high_pass.setBypassed<0>(false);
        break;
    case slope_24:
        *right_high_pass.get<0>().coefficients = *cut_coefficients[0];
        right_high_pass.setBypassed<0>(false);
        *right_high_pass.get<1>().coefficients = *cut_coefficients[1];
        right_high_pass.setBypassed<1>(false);
        break;
    case slope_36:
        *right_high_pass.get<0>().coefficients = *cut_coefficients[0];
        right_high_pass.setBypassed<0>(false);
        *right_high_pass.get<1>().coefficients = *cut_coefficients[1];
        right_high_pass.setBypassed<1>(false);
        *right_high_pass.get<2>().coefficients = *cut_coefficients[2];
        right_high_pass.setBypassed<2>(false);
        break;
    case slope_48:
        *right_high_pass.get<0>().coefficients = *cut_coefficients[0];
        right_high_pass.setBypassed<0>(false);
        *right_high_pass.get<1>().coefficients = *cut_coefficients[1];
        right_high_pass.setBypassed<1>(false);
        *right_high_pass.get<2>().coefficients = *cut_coefficients[2];
        right_high_pass.setBypassed<2>(false);
        *right_high_pass.get<3>().coefficients = *cut_coefficients[3];
        right_high_pass.setBypassed<3>(false);
        break;
    }

    juce::dsp::AudioBlock<float> audio_block(buffer);

    // --

    auto left_channel  = audio_block.getSingleChannelBlock(0);
    auto right_channel = audio_block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> left_context(left_channel);
    juce::dsp::ProcessContextReplacing<float> right_context(right_channel);

    left_chain.process(left_context);
    right_chain.process(right_context);


    
}

//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
    //return new SimpleEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);

}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings GetChainSettings(juce::AudioProcessorValueTreeState& apvst) {
    ChainSettings chain_settings;

    chain_settings.lp_freq = apvst.getRawParameterValue("LowPass Freq")->load();
    chain_settings.hp_freq = apvst.getRawParameterValue("HighPass Freq")->load();
    chain_settings.bell_freq = apvst.getRawParameterValue("Bell Freq")->load();
    chain_settings.bell_gain_in_db = apvst.getRawParameterValue("Bell Gain")->load();
    chain_settings.bell_q = apvst.getRawParameterValue("Bell Q")->load();
    chain_settings.hp_slope = static_cast<Slope>(apvst.getRawParameterValue("HP Slope")->load());
    chain_settings.lp_slope = static_cast<Slope>(apvst.getRawParameterValue("LP Slope")->load());

    return chain_settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleEQAudioProcessor::CreateParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("HighPass Freq", 
                                                           "HighPass Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 
                                                           20.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("LowPass Freq", 
                                                           "LowPass Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 
                                                           20000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Bell Freq", 
                                                           "Bell Freq",
                                                           juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 
                                                           750.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Bell Gain", 
                                                           "Bell Gain",
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 
                                                           0.0f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Bell Q", 
                                                           "Bell Q",
                                                           juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 
                                                           1.0f));

    juce::StringArray filter_slopes;
    for (size_t i = 0; i < 4; ++i) {
        juce::String str;
        str << (12 + 12 * i);
        str << "dB/oct";
        filter_slopes.add(str);
    }

    layout.add(std::make_unique<juce::AudioParameterChoice>("HP Slope", "HP Slope", filter_slopes, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("LP Slope", "LP Slope", filter_slopes, 0));

    return layout;
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEQAudioProcessor();
}
