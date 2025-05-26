#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect the UI" };

    // set up sliders
    juce::Slider delaySlider, feedbackSlider, wetDrySlider, gainBeginSlider, gainEndSlider;

    // set up labels
    juce::Label delayLabel, feedbackLabel, wetDryLabel, gainBeginLabel, gainEndLabel;

    // set up slider attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delaySliderAttach,
        feedbackSliderAttach, wetDrySliderAttach, gainBeginSliderAttach, gainEndSliderAttach;
    ;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
