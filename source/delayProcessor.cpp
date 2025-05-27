//
// Created by smoke on 5/26/2025.
//

#include "delayProcessor.h"

delayProcessor::delayProcessor(){}

void delayProcessor::prepare(double sampleRate, int numChannels, float maxDelaySeconds)
{
    int bufferSize = static_cast<int>(sampleRate * maxDelaySeconds);
    delayBuffer.setSize(numChannels, bufferSize);
    delayBuffer.clear();
    writePosition = 0;
    previousDelaySeconds = 1.0f;
}

void delayProcessor::process(juce::AudioBuffer<float>& buffer,
    float delaySeconds, float feedback, float wetDry,
    float gainBegin, float gainEnd, double sampleRate,
    bool granularMode,
    float grainSize, float grainDensity,
    float grainPitch, float grainSpread)
{
    if (granularMode) {
        processGranularDelay(buffer, delaySeconds, feedback, wetDry,
                           gainBegin, gainEnd, sampleRate,
                           grainSize, grainDensity, grainPitch, grainSpread);
    } else {
        processStandardDelay(buffer, delaySeconds, feedback, wetDry,
                           gainBegin, gainEnd, sampleRate);
    }
}

void delayProcessor::fillBuffer(int channel, int bufferSize, int delayBufferSize, float* channelData)
{
    if (delayBufferSize > bufferSize + writePosition)
    {
        delayBuffer.copyFrom(channel, writePosition, channelData, bufferSize);
    }
    else
    {
        auto numSamplesToEnd = delayBufferSize - writePosition;
        delayBuffer.copyFrom(channel, writePosition, channelData, numSamplesToEnd);
        auto numSamplesAtStart = bufferSize - numSamplesToEnd;
        channelData += numSamplesToEnd;
        delayBuffer.copyFrom(channel, 0, channelData, numSamplesAtStart);
    }
}

void delayProcessor::processStandardDelay(juce::AudioBuffer<float>& buffer,
    float delaySeconds, float feedback, float wetDry,
    float gainBegin, float gainEnd, double sampleRate)
{
    int bufferSize = buffer.getNumSamples();
    int totalNumInputChannels = buffer.getNumChannels();
    int delayBufferSize = delayBuffer.getNumSamples();

    delaySeconds = std::clamp(delaySeconds, 0.01f, 10.0f);
    int newDelayBufferSize = static_cast<int>(sampleRate * delaySeconds);

    if (newDelayBufferSize != bufferSize)
    {
        delayBuffer.setSize(totalNumInputChannels, newDelayBufferSize, true, false, false);
        writePosition =  writePosition % newDelayBufferSize;
        delayBufferSize = newDelayBufferSize;
        previousDelaySeconds = delaySeconds;
    }

    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto* dryChannelData = dryBuffer.getReadPointer(channel);

        auto readPosition = writePosition - static_cast<int>(delaySeconds * sampleRate);
        if (readPosition < 0)
        {
            readPosition += delayBufferSize;
        }

        juce::AudioBuffer<float> delayedBuffer(2, bufferSize);
        auto* delayedData = delayedBuffer.getWritePointer(0);

        if (readPosition + bufferSize <= delayBufferSize)
        {
            delayedBuffer.copyFromWithRamp(0, 0, delayBuffer.getReadPointer(channel, readPosition),
                bufferSize, gainBegin, gainEnd);
        }
        else
        {
            auto numSamplesToEnd = delayBufferSize - readPosition;
            auto numSamplesAtStart = bufferSize - numSamplesToEnd;

            delayedBuffer.copyFromWithRamp(0, 0, delayBuffer.getReadPointer(channel, readPosition),
                numSamplesToEnd, gainBegin, gainEnd);
            delayedBuffer.copyFromWithRamp(0, numSamplesToEnd, delayBuffer.getReadPointer(channel, 0),
                numSamplesAtStart, gainBegin, gainEnd);
        }
        for (int sample = 0; sample < bufferSize; ++sample)
        {
            float wetSignal = delayedData[sample];
            float drySignal = dryChannelData[sample];
            channelData[sample] = drySignal * (1.0f - wetDry) + wetSignal * wetDry;
        }

        juce::AudioBuffer<float> feedbackBuffer(2, bufferSize);
        feedbackBuffer.copyFrom(0, 0, dryChannelData, bufferSize);
        feedbackBuffer.addFromWithRamp(0, 0, delayedData, bufferSize, feedback, feedback);

        fillBuffer(channel, bufferSize, delayBufferSize, feedbackBuffer.getWritePointer(0));
    }
    writePosition += bufferSize;
    writePosition %= delayBufferSize;
}

void delayProcessor::processGranularDelay(juce::AudioBuffer<float>& buffer,
    float delaySeconds, float feedback, float wetDry,
    float gainBegin, float gainEnd, double sampleRate,
    float grainSize, float grainDensity, float grainPitch, float grainSpread)
{
    int bufferSize = buffer.getNumSamples();
    int totalNumInputChannels = buffer.getNumChannels();
    int delayBufferSize = delayBuffer.getNumSamples();

    delaySeconds = std::clamp(delaySeconds, 0.01f, 10.0f);
    int newDelayBufferSize = static_cast<int>(sampleRate * delaySeconds);

    if (newDelayBufferSize != bufferSize)
    {
        delayBuffer.setSize(totalNumInputChannels, newDelayBufferSize, true, false, false);
        writePosition = writePosition % newDelayBufferSize;
        delayBufferSize = newDelayBufferSize;
        previousDelaySeconds = delaySeconds;
        grainProcessor.prepare(sampleRate, totalNumInputChannels, delaySeconds);
    }

    // Store dry signal
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Fill delay buffer with input + feedback first
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto* dryChannelData = dryBuffer.getReadPointer(channel);

        // Create feedback buffer (input + delayed signal with feedback)
        juce::AudioBuffer<float> feedbackBuffer(2, bufferSize);
        feedbackBuffer.copyFrom(0, 0, dryChannelData, bufferSize);

        // Add feedback from delay buffer if we have enough history
        if (writePosition >= bufferSize) {
            auto readPos = writePosition - bufferSize;
            feedbackBuffer.addFromWithRamp(0, 0, delayBuffer.getReadPointer(channel, readPos),
                                         bufferSize, feedback, feedback);
        }

        fillBuffer(channel, bufferSize, delayBufferSize, feedbackBuffer.getWritePointer(0));
    }

    // Process granular delay
    grainProcessor.process(buffer, delayBuffer, writePosition,
                         grainSize, grainDensity, grainPitch, grainSpread, wetDry);

    // Mix dry signal back in (grain processor handles wet/dry internally, but we can adjust)
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        auto* dryChannelData = dryBuffer.getReadPointer(channel);

        for (int sample = 0; sample < bufferSize; ++sample)
        {
            // Apply gain ramping to the final output
            float gainRamp = gainBegin + (gainEnd - gainBegin) * (static_cast<float>(sample) / bufferSize);
            channelData[sample] *= gainRamp;
        }
    }

    writePosition += bufferSize;
    writePosition %= delayBufferSize;
}
