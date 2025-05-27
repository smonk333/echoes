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

    // standard delay slider and label setup
    juce::Slider delaySlider, feedbackSlider, wetDrySlider, gainBeginSlider, gainEndSlider;
    juce::Label delayLabel, feedbackLabel, wetDryLabel, gainBeginLabel, gainEndLabel;

    // granular controls
    juce::ToggleButton granularModeToggle;
    juce::Slider grainSizeSlider, grainDensitySlider, grainPitchSlider, grainSpreadSlider;
    juce::Label granularModeLabel, grainSizeLabel, grainDensityLabel, grainPitchLabel, grainSpreadLabel;

    // set up delay slider attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delaySliderAttach,
        feedbackSliderAttach, wetDrySliderAttach, gainBeginSliderAttach, gainEndSliderAttach;

    // granular delay attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> granularModeAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> grainSizeSliderAttach,
        grainDensitySliderAttach, grainPitchSliderAttach, grainSpreadSliderAttach;

    void granularModeChanged();
    ;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
