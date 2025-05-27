//
// Created by smoke on 5/27/2025.
//

#include "grainProcessor.h"

grainProcessor::grainProcessor()
    : sampleRate(44100.0), numChannels(2), delayBufferSize(0),
      grainTriggerCounter(0.0f), samplesPerGrain(0.0f),
      randomEngine(std::random_device{}()), randomDist(0.0f, 1.0f),
      grainSizeMs(100.0f), grainDensityHz(10.0f), grainPitchRatio(1.0f),
      grainSpreadMs(50.0f)
{
    grains.resize(MAX_GRAINS);
}

grainProcessor::~grainProcessor() {}

void grainProcessor::prepare (double sampleRate, int numChannels, float maxDelaySeconds)
{
    delayBufferSize = static_cast<int>(sampleRate * maxDelaySeconds);

    // reset all grains
    for (auto& grain : grains)
    {
        grain.isActive = false;
    }

    grainTriggerCounter = 0.0f;
    samplesPerGrain = static_cast<float>(sampleRate / grainDensityHz);
}

void grainProcessor::process (juce::AudioBuffer<float>& buffer,
    const juce::AudioBuffer<float>& delayBuffer,
    int writePosition, float grainSize, float grainDensity, float grainPitch,
    float grainSpread, float wetDry)
{
    grainSizeMs = grainSize;
    grainDensityHz = grainDensity;
    grainPitchRatio = grainPitch;
    grainSpreadMs = grainSpread;

    samplesPerGrain = static_cast<float>(sampleRate / grainDensityHz);

    int bufferSize = buffer.getNumSamples();

    // make a temporary buffer for grain output
    juce::AudioBuffer<float> grainBuffer(numChannels, bufferSize);
    grainBuffer.clear();

    // process each sample
    for (int sample = 0; sample < bufferSize; ++sample)
    {
        // check if we should trigger new grains
        grainTriggerCounter += 1.0f;
        if (grainTriggerCounter >= samplesPerGrain)
        {
            grainTriggerCounter -= samplesPerGrain;

            for (int ch = 0; ch < numChannels; ++ch)
            {
                triggerGrain(ch, (writePosition + sample) % delayBufferSize);
            }
        }

        // process active grains
        for (auto& grain : grains)
        {
            if (grain.isActive)
            {
                processGrain(grain, grainBuffer, delayBuffer, bufferSize);
            }
        }
    }

    // mix grain output with original buffer
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);
        auto* grainData = grainBuffer.getReadPointer(ch);

        for (int sample = 0; sample < bufferSize; ++sample)
        {
            float drySignal = channelData[sample];
            float wetSignal = grainData[sample];
            channelData[sample] = drySignal * (1.0f - wetDry) + wetSignal * wetDry;
        }
    }
}

void grainProcessor::setGrainParameters (float size, float density, float pitch, float spread)
{
    grainSizeMs = size;
    grainDensityHz = density;
    grainPitchRatio = pitch;
    grainSpreadMs = spread;

    samplesPerGrain = static_cast<float>(sampleRate / grainDensityHz);
}

void grainProcessor::reset()
{
    for (auto& grain : grains)
    {
        grain.isActive = false;
    }
    grainTriggerCounter = 0.0f;
}

void grainProcessor::triggerGrain (int channel, int delayBufferWritePos)
{
    // find an inactive grain
    for (auto& grain : grains)
    {
        if (!grain.isActive)
        {
            grain.isActive = true;
            grain.channel = channel;
            grain.grainSize = static_cast<int>((grainSizeMs / 1000.0f) * sampleRate);
            // set random amplitude variation
            grain.amplitude = 0.5f + (randomDist(randomEngine) * 0.5f);

            // set start position with random spread
            int spreadSamples = static_cast<int>((grainSpreadMs / 1000.0f) * sampleRate);
            int randomOffset = static_cast<int>((randomDist(randomEngine) - 0.5f) * 2.0f * spreadSamples);
            grain.startPosition = getRandomDelayPosition(delayBufferWritePos + randomOffset);
            grain.currentPosition = 0;

            break; // only trigger one grain per call
        }
    }
}

float grainProcessor::getGrainEnvelope (const Grain& grain) const
{
    if (grain.grainSize <= 0)
        return 0.0f;

    float progress = static_cast<float>(grain.currentPosition) / static_cast<float>(grain.grainSize);

    // Hann window envelope
    if (progress >= 1.0f)
    {
        return 0.0f;
    }

    return 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * progress));
}

int grainProcessor::getRandomDelayPosition (int writePosition)
{
    int pos = writePosition - static_cast<int>((randomDist(randomEngine) * 0.8f + 0.1f) * delayBufferSize);
    if (pos < 0)
    {
        pos += delayBufferSize;
    }
    return pos % delayBufferSize;
}

void grainProcessor::processGrain (Grain& grain, juce::AudioBuffer<float>& outputBuffer,
    const juce::AudioBuffer<float>& delayBuffer, int bufferSize)
{
    if (!grain.isActive || grain.channel >= numChannels)
    {
        return;
    }

    auto* outputData = outputBuffer.getWritePointer(grain.channel);
    auto* delayData = delayBuffer.getReadPointer(grain.channel);

    for (int sample = 0; sample < bufferSize && grain.isActive; ++sample)
    {
        if (grain.currentPosition >= grain.grainSize)
        {
            grain.isActive = false;
            break;
        }

        // calculate read position with pitch shifting
        float readPos = grain.startPosition + (grain.currentPosition * grainPitchRatio);
        int readIndex = static_cast<int>(readPos) % delayBufferSize;

        // linear interpolation for fractional positions
        float fraction = readPos - static_cast<int>(readPos);
        int nextIndex = (readIndex + 1) % delayBufferSize;

        float sample1 = delayData[readIndex];
        float sample2 = delayData[nextIndex];
        float interpolatedSample = sample1 + fraction * (sample2 - sample1);

        // apply grain envelope and amplitude
        float envelope = getGrainEnvelope(grain);
        float grainSample = interpolatedSample * envelope * grain.amplitude;

        outputData[sample] += grainSample;
        grain.currentPosition++;
    }
}