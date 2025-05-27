//
// Created by smoke on 5/26/2025.
//
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#ifndef DELAYPROCESSOR_H
#define DELAYPROCESSOR_H



class delayProcessor {
public:
    delayProcessor();
    void prepare(double sampleRate, int numChannels, float maxDelaySeconds);
    void process(juce::AudioBuffer<float>& buffer,
        float delaySeconds, float feedback, float wetDry,
        float gainBegin, float gainEnd, double sampleRate);

private:
    juce::AudioBuffer<float> delayBuffer;
    int writePosition { 0 };
    float previousDelaySeconds = 1.0f;
    void fillBuffer(int channel, int bufferSize, int delayBufferSize, float* channelData);
};



#endif //DELAYPROCESSOR_H
