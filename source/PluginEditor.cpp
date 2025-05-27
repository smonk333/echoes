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

    auto setupToggle = [this](juce::ToggleButton& t, const juce::String& text) {
        t.setButtonText(text);
        t.setClickingTogglesState(true);
        addAndMakeVisible(&t);
    };

    setupSlider(delaySlider);
    setupSlider(feedbackSlider);
    setupSlider(wetDrySlider);
    setupSlider(gainBeginSlider);
    setupSlider(gainEndSlider);

    // set up granular controls
    setupSlider(grainSizeSlider);
    setupSlider(grainDensitySlider);
    setupSlider(grainPitchSlider);
    setupSlider(grainSpreadSlider);
    setupToggle(granularModeToggle, "Granular Mode");


    delaySliderAttach = std::make_unique<SliderAttachment>(params, "delaySize", delaySlider);
    feedbackSliderAttach = std::make_unique<SliderAttachment>(params, "feedback", feedbackSlider);
    wetDrySliderAttach = std::make_unique<SliderAttachment>(params, "wetDry", wetDrySlider);
    gainBeginSliderAttach = std::make_unique<SliderAttachment>(params, "gainBegin", gainBeginSlider);
    gainEndSliderAttach = std::make_unique<SliderAttachment>(params, "gainEnd", gainEndSlider);

    // granular delay attachments
    granularModeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(params, "granularMode", granularModeToggle);
    grainSizeSliderAttach = std::make_unique<SliderAttachment>(params, "grainSize", grainSizeSlider);
    grainDensitySliderAttach = std::make_unique<SliderAttachment>(params, "grainDensity", grainDensitySlider);
    grainPitchSliderAttach = std::make_unique<SliderAttachment>(params, "grainPitch", grainPitchSlider);
    grainSpreadSliderAttach = std::make_unique<SliderAttachment>(params, "grainSpread", grainSpreadSlider);

    setupLabel(delayLabel, "delay");
    setupLabel(feedbackLabel, "feedback");
    setupLabel(wetDryLabel, "wet/dry");
    setupLabel(gainBeginLabel, "gain begin");
    setupLabel(gainEndLabel, "gain end");

    setupLabel(granularModeLabel, "Mode");
    setupLabel(grainSizeLabel, "Size (ms)");
    setupLabel(grainDensityLabel, "Density (Hz)");
    setupLabel(grainPitchLabel, "Pitch");
    setupLabel(grainSpreadLabel, "Spread (ms)");

    granularModeToggle.onStateChange = [this]() { granularModeChanged(); };

    // set granular control visibility
    granularModeChanged();

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
    setResizable (true, true);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Draw section dividers
    g.setColour(juce::Colours::grey);
    g.drawLine(20, 120, getWidth() - 20, 120, 1.0f);

    // Draw section titles
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);
    g.drawText("Standard Delay", 20, 10, 200, 20, juce::Justification::left);

    if (granularModeToggle.getToggleState())
    {
        g.drawText("Granular Parameters", 20, 130, 200, 20, juce::Justification::left);
    }
}

void PluginEditor::granularModeChanged()
{
    bool granularMode = granularModeToggle.getToggleState();

    // Show/hide granular controls based on mode
    grainSizeSlider.setVisible(granularMode);
    grainDensitySlider.setVisible(granularMode);
    grainPitchSlider.setVisible(granularMode);
    grainSpreadSlider.setVisible(granularMode);

    grainSizeLabel.setVisible(granularMode);
    grainDensityLabel.setVisible(granularMode);
    grainPitchLabel.setVisible(granularMode);
    grainSpreadLabel.setVisible(granularMode);

    repaint();
}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    // Standard delay section
    auto standardSection = area.removeFromTop(100);
    auto standardRow = standardSection.removeFromTop(80);
    auto standardLabelRow = standardSection;

    auto sliderWidth = standardRow.getWidth() / 6; // 6 controls including mode button

    // Standard delay controls
    delaySlider.setBounds(standardRow.removeFromLeft(sliderWidth));
    feedbackSlider.setBounds(standardRow.removeFromLeft(sliderWidth));
    wetDrySlider.setBounds(standardRow.removeFromLeft(sliderWidth));
    gainBeginSlider.setBounds(standardRow.removeFromLeft(sliderWidth));
    gainEndSlider.setBounds(standardRow.removeFromLeft(sliderWidth));
    granularModeToggle.setBounds(standardRow.removeFromLeft(sliderWidth).reduced(10, 25));

    // Standard delay labels
    auto labelWidth = standardLabelRow.getWidth() / 6;
    delayLabel.setBounds(standardLabelRow.removeFromLeft(labelWidth));
    feedbackLabel.setBounds(standardLabelRow.removeFromLeft(labelWidth));
    wetDryLabel.setBounds(standardLabelRow.removeFromLeft(labelWidth));
    gainBeginLabel.setBounds(standardLabelRow.removeFromLeft(labelWidth));
    gainEndLabel.setBounds(standardLabelRow.removeFromLeft(labelWidth));
    granularModeLabel.setBounds(standardLabelRow.removeFromLeft(labelWidth));

    // Granular section - ALWAYS position these controls, visibility is handled separately
    area.removeFromTop(20); // spacing
    auto granularSection = area.removeFromTop(100);
    auto granularRow = granularSection.removeFromTop(80);
    auto granularLabelRow = granularSection;

    auto granularSliderWidth = granularRow.getWidth() / 4;

    // Position granular controls regardless of visibility
    grainSizeSlider.setBounds(granularRow.removeFromLeft(granularSliderWidth));
    grainDensitySlider.setBounds(granularRow.removeFromLeft(granularSliderWidth));
    grainPitchSlider.setBounds(granularRow.removeFromLeft(granularSliderWidth));
    grainSpreadSlider.setBounds(granularRow.removeFromLeft(granularSliderWidth));

    auto granularLabelWidth = granularLabelRow.getWidth() / 4;
    grainSizeLabel.setBounds(granularLabelRow.removeFromLeft(granularLabelWidth));
    grainDensityLabel.setBounds(granularLabelRow.removeFromLeft(granularLabelWidth));
    grainPitchLabel.setBounds(granularLabelRow.removeFromLeft(granularLabelWidth));
    grainSpreadLabel.setBounds(granularLabelRow.removeFromLeft(granularLabelWidth));
}
