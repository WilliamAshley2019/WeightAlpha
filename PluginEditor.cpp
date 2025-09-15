#include "PluginEditor.h"
#include "PluginProcessor.h"

// WeightAlphaEditor implementation
WeightAlphaEditor::WeightAlphaEditor(WeightAlphaProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    freqListener([this](float, float) { updateFrequencyDisplay(); })
{
    setOpaque(true); // Optimize rendering
    setBufferedToImage(true); // Improve rendering stability

    setLookAndFeel(&lookAndFeel);

    // Title Label
    addAndMakeVisible(titleLabel);
    titleLabel.setText("Weight Alpha", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(24.0f).withName("Inter").withStyle("Bold")));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setVisible(true);

    // Knobs and Labels
    setupSlider(freqKnob, freqLabel, "Frequency");
    setupSlider(weightKnob, weightLabel, "Weight");
    setupSlider(strengthKnob, strengthLabel, "Strength");

    addAndMakeVisible(freqValueLabel);
    freqValueLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(14.0f).withName("Inter")));
    freqValueLabel.setJustificationType(juce::Justification::centred);
    freqValueLabel.setVisible(true);

    // Bypass Button
    addAndMakeVisible(bypassButton);
    bypassButton.setName("Bypass");
    bypassButton.setVisible(true);
    addAndMakeVisible(bypassLabel);
    bypassLabel.setFont(juce::Font(juce::FontOptions{}.withHeight(14.0f).withName("Inter")));
    bypassLabel.setText("Bypass", juce::dontSendNotification);
    bypassLabel.attachToComponent(&bypassButton, false);
    bypassLabel.setVisible(true);

    // Frequency Range Button
    addAndMakeVisible(freqRangeButton);
    freqRangeButton.setButtonText("Narrow Range");
    freqRangeButton.setClickingTogglesState(true);
    freqRangeButton.setVisible(true);

    // Preset Selector
    addAndMakeVisible(presetSelector);
    presetSelector.addItemList({ "Default", "Bass Boost", "Vocal Warmth" }, 1);
    presetSelector.setSelectedId(1, juce::dontSendNotification);
    presetSelector.setVisible(true);

    // Parameter Attachments
    auto& apvts = audioProcessor.getValueTree();
    freqAttach = std::make_unique<SliderAttachment>(apvts, "freq", freqKnob);
    weightAttach = std::make_unique<SliderAttachment>(apvts, "weight", weightKnob);
    strengthAttach = std::make_unique<SliderAttachment>(apvts, "strength", strengthKnob);
    bypassAttach = std::make_unique<ButtonAttachment>(apvts, "bypass", bypassButton);
    freqRangeAttach = std::make_unique<ButtonAttachment>(apvts, "freqRange", freqRangeButton);
    presetAttach = std::make_unique<ComboBoxAttachment>(apvts, "currentProgram", presetSelector);

    // Attach listener to parameters
    apvts.getParameter("freq")->addListener(&freqListener);
    apvts.getParameter("freqRange")->addListener(&freqListener);
    updateFrequencyDisplay();

    setResizable(true, true);
    setResizeLimits(450, 280, 800, 600);
    setSize(500, 300);

    // Start the timer to update the UI
    startTimerHz(30);
}

WeightAlphaEditor::~WeightAlphaEditor()
{
    auto& apvts = audioProcessor.getValueTree();
    apvts.getParameter("freq")->removeListener(&freqListener);
    apvts.getParameter("freqRange")->removeListener(&freqListener);
    setLookAndFeel(nullptr);
}

void WeightAlphaEditor::timerCallback()
{
    updateFrequencyDisplay();
}

void WeightAlphaEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    addAndMakeVisible(slider);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setPopupDisplayEnabled(true, true, this);
    slider.setVisible(true);

    addAndMakeVisible(label);
    label.setFont(juce::Font(juce::FontOptions{}.withHeight(15.0f).withName("Inter")));
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.attachToComponent(&slider, false);
    label.setVisible(true);
}

void WeightAlphaEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient gradient(juce::Colour(0xff222731), getLocalBounds().getTopLeft().toFloat(),
        juce::Colour(0xff1a1d24), getLocalBounds().getBottomLeft().toFloat(), false);
    g.setGradientFill(gradient);
    g.fillAll();
}

void WeightAlphaEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    juce::Logger::writeToLog("Total bounds: " + area.toString());

    // Header
    auto headerArea = area.removeFromTop(40);
    titleLabel.setBounds(headerArea.removeFromLeft(headerArea.getWidth() / 2));
    presetSelector.setBounds(headerArea.withTrimmedLeft(headerArea.getWidth() / 2).reduced(10, 0));
    juce::Logger::writeToLog("Title bounds: " + titleLabel.getBounds().toString());
    juce::Logger::writeToLog("Preset selector bounds: " + presetSelector.getBounds().toString());
    area.removeFromTop(20);

    // Main Knobs
    auto knobArea = area.removeFromTop(120);
    juce::Logger::writeToLog("Knob area: " + knobArea.toString());

    // Divide knob area into three columns for Frequency, Weight, and Strength
    auto freqArea = knobArea.removeFromLeft(knobArea.getWidth() / 3).reduced(10);
    freqKnob.setBounds(freqArea.removeFromTop(80));
    freqValueLabel.setBounds(freqArea.removeFromTop(20));
    freqLabel.setBounds(freqArea);
    juce::Logger::writeToLog("Freq knob bounds: " + freqKnob.getBounds().toString());
    juce::Logger::writeToLog("Freq value label bounds: " + freqValueLabel.getBounds().toString());
    juce::Logger::writeToLog("Freq label bounds: " + freqLabel.getBounds().toString());

    auto weightArea = knobArea.removeFromLeft(knobArea.getWidth() / 2).reduced(10);
    weightKnob.setBounds(weightArea.removeFromTop(80));
    weightLabel.setBounds(weightArea);
    juce::Logger::writeToLog("Weight knob bounds: " + weightKnob.getBounds().toString());
    juce::Logger::writeToLog("Weight label bounds: " + weightLabel.getBounds().toString());

    auto strengthArea = knobArea.reduced(10);
    strengthKnob.setBounds(strengthArea.removeFromTop(80));
    strengthLabel.setBounds(strengthArea);
    juce::Logger::writeToLog("Strength knob bounds: " + strengthKnob.getBounds().toString());
    juce::Logger::writeToLog("Strength label bounds: " + strengthLabel.getBounds().toString());

    area.removeFromTop(10);

    // Bottom Controls
    auto bottomArea = area;
    bypassButton.setBounds(bottomArea.removeFromLeft(40).withHeight(40));
    freqRangeButton.setBounds(bottomArea.removeFromRight(120).withHeight(40));
    juce::Logger::writeToLog("Bypass button bounds: " + bypassButton.getBounds().toString());
    juce::Logger::writeToLog("Freq range button bounds: " + freqRangeButton.getBounds().toString());
}

void WeightAlphaEditor::updateFrequencyDisplay()
{
    bool narrowRange = audioProcessor.getValueTree().getParameter("freqRange")->getValue() > 0.5f;
    float freqVal = audioProcessor.getValueTree().getParameter("freq")->getValue();
    float freqHz = narrowRange ? (freqVal * 100.0f) + 20.0f : juce::mapToLog10(freqVal, 20.0f, 20000.0f);

    juce::String freqText;
    if (freqHz >= 1000.0f)
        freqText = juce::String(freqHz / 1000.0f, 1) + " kHz";
    else
        freqText = juce::String(static_cast<int>(freqHz + 0.5f)) + " Hz";

    if (freqValueLabel.getText() != freqText)
        freqValueLabel.setText(freqText, juce::dontSendNotification);
}