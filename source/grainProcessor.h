//
// Created by smoke on 5/27/2025.
//

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <random>

using random_engine = std::mt19937;

#ifndef GRAINPROCESSOR_H
#define GRAINPROCESSOR_H

// struct to hold the grains
struct Grain
{
    int startPosition;
    int currentPosition;
    int grainSize;
    float amplitude;
    bool isActive;
    int channel;

    Grain() : startPosition(0), currentPosition (0), grainSize(0),
    amplitude(0.0f), isActive(false), channel(0) {}
};

class grainProcessor {
public:
    grainProcessor();
    ~grainProcessor();

    void prepare(double sampleRate, int numChannels, float maxDelaySeconds);
    void process(juce::AudioBuffer<float>& buffer,
        const juce::AudioBuffer<float>& delayBuffer,
        int writePosition, float grainSize, float grainDensity,
        float grainPitch, float grainSpread, float wetDry);

    void setGrainParameters(float size, float density,
        float pitch, float spread);
    void reset();

private:
    static constexpr int MAX_GRAINS = 1000;
    std::vector<Grain> grains;

    double sampleRate;
    int numChannels;
    int delayBufferSize;

    // grain scheduling
    float grainTriggerCounter;
    float samplesPerGrain;

    random_engine randomEngine;
    std::uniform_real_distribution<float> randomDist;

    // parameters
    float grainSizeMs;
    float grainDensityHz;
    float grainPitchRatio;
    float grainSpreadMs;

    // helper methods
    void triggerGrain(int channel, int delayBufferWritePos);
    float getGrainEnvelope(const Grain& grain) const;
    int getRandomDelayPosition(int writePosition);
    void processGrain(Grain& grain, juce::AudioBuffer<float>& outputBuffer,
        const juce::AudioBuffer<float>& delayBuffer, int bufferSize);
};

#endif //GRAINPROCESSOR_H
