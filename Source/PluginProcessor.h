/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope {
    slope_12,
    slope_24,
    slope_36,
    slope_48
};

struct ChainSettings {
    float bell_freq{0}, bell_gain_in_db{0}, bell_q{1.f};
    float lp_freq{ 0 }, hp_freq{ 0 };
    int   lp_slope{ Slope::slope_12 }, hp_slope{ Slope::slope_12 };
};

ChainSettings GetChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class SimpleEQAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SimpleEQAudioProcessor();
    ~SimpleEQAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

    static juce::AudioProcessorValueTreeState::ParameterLayout CreateParameterLayout();
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", CreateParameterLayout() };

private:
    using Filter = juce::dsp::IIR::Filter<float>;

    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

    MonoChain left_chain, right_chain;

    enum ChainPositions {
        HighPass,
        Bell,
        LowPass
    };

    void UpdateBellFilter(const ChainSettings& chain_settings);
    void UpdateCoefficients(Filter::CoefficientsPtr& old_coef, const Filter::CoefficientsPtr& new_coefs);

    template <int Index, typename ChainType, typename CoefficientType>
    void Update(ChainType& chain, const CoefficientType& coefficients) {
        UpdateCoefficients(chain.get<Index>().coefficients, coefficients[Index]);
        chain.setBypassed<Index>(false);
    }

    template <typename ChainType, typename CoefficientType>
    void UpdatePassFilter(ChainType& left_high_pass, 
                          const CoefficientType& cut_coefficients,
                          const Slope& slope) {

        left_high_pass.setBypassed<0>(true);
        left_high_pass.setBypassed<1>(true);
        left_high_pass.setBypassed<2>(true);
        left_high_pass.setBypassed<3>(true);

        switch (slope) {
            case slope_48:
            {
                Update<3>(left_high_pass, cut_coefficients);
            }
            case slope_36:
            {
                Update<2>(left_high_pass, cut_coefficients);
            }
            case slope_24:
            {
                Update<1>(left_high_pass, cut_coefficients);
            }
            case slope_12:
            {
                Update<0>(left_high_pass, cut_coefficients);
            }
        }
    }

    void UpdateHighpassFilter(const ChainSettings& chain_settings);
    void UpdateLowpassFilter(const ChainSettings& chain_settings);

    void UpdateFilters();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEQAudioProcessor)
};
