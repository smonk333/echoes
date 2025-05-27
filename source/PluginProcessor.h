#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

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
    float previousDelaySeconds = 1.0f; // start off with default delay set to 1 second

    //==========================parameter setup=================================

    std::atomic<float>* delaySizeParam;
    std::atomic<float>* gainBeginParam;
    std::atomic<float>* gainEndParam;
    std::atomic<float>* feedbackParam;
    std::atomic<float>* wetDryParam;
    juce::AudioProcessorValueTreeState apvts;

private:
    void fillBuffer (int channel, int bufferSize, int delayBufferSize, float* channelData);
    juce::AudioBuffer<float> delayBuffer;
    int writePosition { 0 }; // for keeping track of where to write in the circular buffer
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
