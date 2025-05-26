#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    auto &params = processorRef.apvts;

    auto setupSlider = [this](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(&s);
    };

    auto setupLabel = [this](juce::Label& l, const juce::String& text) {
        l.setText(text, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(&l);
    };

    setupSlider(delaySlider);
    setupSlider(feedbackSlider);
    setupSlider(wetDrySlider);
    setupSlider(gainBeginSlider);
    setupSlider(gainEndSlider);

    delaySliderAttach = std::make_unique<SliderAttachment>(params, "delaySize", delaySlider);
    feedbackSliderAttach = std::make_unique<SliderAttachment>(params, "feedback", feedbackSlider);
    wetDrySliderAttach = std::make_unique<SliderAttachment>(params, "wetDry", wetDrySlider);
    gainBeginSliderAttach = std::make_unique<SliderAttachment>(params, "gainBegin", gainBeginSlider);
    gainEndSliderAttach = std::make_unique<SliderAttachment>(params, "gainEnd", gainEndSlider);

    setupLabel(delayLabel, "delay");
    setupLabel(feedbackLabel, "feedback");
    setupLabel(wetDryLabel, "wet/dry");
    setupLabel(gainBeginLabel, "gain begin");
    setupLabel(gainEndLabel, "gain end");

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    setResizable (true, true);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    // layout the positions of your child components here
    auto area = getLocalBounds().reduced(20);
    auto row = area.removeFromTop(220);

    auto sliderWidth = row.getWidth() / 5;

    delaySlider.setBounds(row.removeFromLeft(sliderWidth));
    feedbackSlider.setBounds(row.removeFromLeft(sliderWidth));
    wetDrySlider.setBounds(row.removeFromLeft(sliderWidth));
    gainBeginSlider.setBounds(row.removeFromLeft(sliderWidth));
    gainEndSlider.setBounds(row.removeFromLeft(sliderWidth));

    auto labelRow = getLocalBounds().reduced(20).removeFromBottom(180 + 0).removeFromBottom(30);
    auto labelWidth = labelRow.getWidth() / 5;

    delayLabel.setBounds(labelRow.removeFromLeft(labelWidth));
    feedbackLabel.setBounds(labelRow.removeFromLeft(labelWidth));
    wetDryLabel.setBounds(labelRow.removeFromLeft(labelWidth));
    gainBeginLabel.setBounds(labelRow.removeFromLeft(labelWidth));
    gainEndLabel.setBounds(labelRow.removeFromLeft(labelWidth));


}
