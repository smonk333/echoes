#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "delayProcessor.h"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==========================parameter setup=================================

    // standard delay parameters
    std::atomic<float>* delaySizeParam;
    std::atomic<float>* gainBeginParam;
    std::atomic<float>* gainEndParam;
    std::atomic<float>* feedbackParam;
    std::atomic<float>* wetDryParam;

    // granular parameters
    std::atomic<float>* granularModeParam;
    std::atomic<float>* grainSizeParam;
    std::atomic<float>* grainDensityParam;
    std::atomic<float>* grainPitchParam;
    std::atomic<float>* grainSpreadParam;

    juce::AudioProcessorValueTreeState apvts;

private:

    delayProcessor delay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
