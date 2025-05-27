//
// Created by smoke on 5/26/2025.
//
#pragma once

#include "grainProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

#ifndef DELAYPROCESSOR_H
#define DELAYPROCESSOR_H



class delayProcessor {
public:
    delayProcessor();
    void prepare(double sampleRate, int numChannels, float maxDelaySeconds);
    void process(juce::AudioBuffer<float>& buffer,
        float delaySeconds, float feedback, float wetDry,
        float gainBegin, float gainEnd, double sampleRate,
        bool granularMode = false, float grainSize = 100.0f, float grainDensity = 10.0f,
        float grainPitch = 1.0f, float grainSpread = 50.0f);

private:
    juce::AudioBuffer<float> delayBuffer;
    int writePosition { 0 };
    float previousDelaySeconds = 1.0f;
    grainProcessor grainProcessor;

    void fillBuffer(int channel, int bufferSize, int delayBufferSize, float* channelData);
    void processStandardDelay(juce::AudioBuffer<float>& buffer,
        float delaySeconds, float feedback, float wetDry, float gainBegin, float gainEnd,
        double sampleRate);
    void processGranularDelay(juce::AudioBuffer<float>& buffer, float delaySeconds,
        float feedback, float wetDry, float gainBegin, float gainEnd,
        double sampleRate, float grainSize, float grainDensity, float grainPitch,
        float grainSpread);
};

#endif //DELAYPROCESSOR_H
