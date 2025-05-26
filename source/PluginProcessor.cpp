#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                     apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // caching parameter pointers
    delaySizeParam = apvts.getRawParameterValue("delaySize");
    feedbackParam = apvts.getRawParameterValue("feedback");
    wetDryParam = apvts.getRawParameterValue("wetDry");
    gainBeginParam = apvts.getRawParameterValue("gainBegin");
    gainEndParam = apvts.getRawParameterValue("gainEnd");
}

PluginProcessor::~PluginProcessor()
{
}

//======================create parameter layout=================================
juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delaySize", "Delay Size", 0.01f, 10.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("feedback", "Feedback", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("wetDry", "Wet/Dry", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("gainBegin", "Gain Begin", 0.0f, 1.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("gainEnd", "Gain End", 0.0f, 1.0f, 1.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    auto delaySeconds = std::clamp(delaySizeParam->load(), 0.01f, 10.0f); // 10 seconds max
    auto delayBufferSize = static_cast<int>(sampleRate * delaySeconds);
    circularBuffer.setSize(getTotalNumOutputChannels(), delayBufferSize);

}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto bufferSize = buffer.getNumSamples();
    auto delayBufferSize = circularBuffer.getNumSamples();

    const float delaySize = *delaySizeParam;
    const float feedback = *feedbackParam;
    const float wetDry = *wetDryParam;
    const float gainBegin = *gainBeginParam;
    const float gainEnd = *gainEndParam;

    float delaySeconds = std::clamp(delaySize, 0.01f, 10.0f);
    int newDelayBufferSize = static_cast<int>(getSampleRate() * delaySeconds);

    // Fix: Compare with previousDelaySeconds as int, not float
    if (newDelayBufferSize != static_cast<int>(getSampleRate() * previousDelaySeconds))
    {
        circularBuffer.setSize(getTotalNumOutputChannels(), newDelayBufferSize, false, true); // Clear the buffer
        writePosition = writePosition % newDelayBufferSize;
        delayBufferSize = newDelayBufferSize;
        previousDelaySeconds = delaySeconds;
    }
    else
    {
        delayBufferSize = circularBuffer.getNumSamples();
    }

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Store the dry signal before processing
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto* dryChannelData = dryBuffer.getReadPointer(channel);

        // Calculate read position for the delay
        auto readPosition = writePosition - static_cast<int>(delaySeconds * getSampleRate());
        if (readPosition < 0)
        {
            readPosition += delayBufferSize;
        }

        // Read the delayed signal from the circular buffer
        juce::AudioBuffer<float> delayedBuffer(1, bufferSize);
        auto* delayedData = delayedBuffer.getWritePointer(0);

        if (readPosition + bufferSize <= delayBufferSize)
        {
            // Simple case: no wrapping needed
            delayedBuffer.copyFromWithRamp(0, 0, circularBuffer.getReadPointer(channel, readPosition),
                                         bufferSize, gainBegin, gainEnd);
        }
        else
        {
            // Wrapping case: read from end and beginning of circular buffer
            auto numSamplesToEnd = delayBufferSize - readPosition;
            auto numSamplesAtStart = bufferSize - numSamplesToEnd;

            delayedBuffer.copyFromWithRamp(0, 0, circularBuffer.getReadPointer(channel, readPosition),
                                         numSamplesToEnd, gainBegin, gainEnd);
            delayedBuffer.copyFromWithRamp(0, numSamplesToEnd, circularBuffer.getReadPointer(channel, 0),
                                         numSamplesAtStart, gainBegin, gainEnd);
        }

        // Apply wet/dry mixing
        for (int sample = 0; sample < bufferSize; ++sample)
        {
            float wetSignal = delayedData[sample];
            float drySignal = dryChannelData[sample];
            channelData[sample] = drySignal * (1.0f - wetDry) + wetSignal * wetDry;
        }

        // Add feedback: mix the delayed signal back into the input before writing to circular buffer
        juce::AudioBuffer<float> feedbackBuffer(1, bufferSize);
        feedbackBuffer.copyFrom(0, 0, dryChannelData, bufferSize);
        feedbackBuffer.addFromWithRamp(0, 0, delayedData, bufferSize, feedback, feedback);

        // Write the feedback signal to the circular buffer
        fillBuffer(channel, bufferSize, delayBufferSize, feedbackBuffer.getWritePointer(0));
    }

    // Update write position
    writePosition += bufferSize;
    writePosition %= delayBufferSize;
}

void PluginProcessor::fillBuffer(int channel, int bufferSize, int delayBufferSize,
    float* channelData)
{
    // check to see if main buffer copies to circular buffer without needing to wrap
    if (delayBufferSize > bufferSize + writePosition)
    {
        // copy main buffer contents to circular buffer
        circularBuffer.copyFrom(channel, writePosition, channelData, bufferSize);
    }

    else
    {
        // determine how much space is available at the end of the circular buffer
        auto numSamplesToEnd = delayBufferSize - writePosition;

        // copy that amount of data from the main buffer to the circular buffer
        circularBuffer.copyFrom(channel, writePosition, channelData, numSamplesToEnd);

        // calculate how much data is left to copy
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;

        channelData += numSamplesToEnd; // move the pointer forward by the amount copied

        // copy remaining amount of data to the start (index 0) of the circular buffer
        circularBuffer.copyFrom(channel, 0, channelData, numSamplesAtStart);
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
