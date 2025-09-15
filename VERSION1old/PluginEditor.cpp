#include "PluginEditor.h"

WeightEditor::WeightEditor(WeightProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);

    audioProcessor.getValueTree().addParameterListener("freq", this);
    audioProcessor.getValueTree().addParameterListener("freqRange", this);

    auto configureKnob = [&](juce::Slider& knob, juce::Label& label, const juce::String& text) {
        addAndMakeVisible(knob);
        knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        knob.setPopupDisplayEnabled(true, true, this);
        knob.setColour(juce::Slider::thumbColourId, juce::Colour(0xffe6e6e6));
        knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff6b85de));
        knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff3f3f4e));

        addAndMakeVisible(label);
        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(juce::FontOptions().withName("Arial").withHeight(16.0f).withStyle("Regular")));
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.attachToComponent(&knob, false);
        };

    configureKnob(freqKnob, freqLabel, "Freq");
    configureKnob(weightKnob, weightLabel, "Weight");
    configureKnob(strengthKnob, strengthLabel, "Strength");

    addAndMakeVisible(freqValueLabel);
    freqValueLabel.setJustificationType(juce::Justification::centred);
    freqValueLabel.setFont(juce::Font(juce::FontOptions().withName("Arial").withHeight(14.0f).withStyle("Regular")));
    updateFrequencyDisplay();

    addAndMakeVisible(bypassButton);
    bypassButton.setButtonText("");
    bypassButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    bypassAttach = std::make_unique<ButtonAttachment>(p.getValueTree(), "bypass", bypassButton);

    addAndMakeVisible(bypassLabel);
    bypassLabel.setText("Bypass", juce::dontSendNotification);
    bypassLabel.setFont(juce::Font(juce::FontOptions().withName("Arial").withHeight(14.0f).withStyle("Regular")));
    bypassLabel.setJustificationType(juce::Justification::centred);
    bypassLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    bypassLabel.attachToComponent(&bypassButton, false);

    addAndMakeVisible(freqRangeButton);
    freqRangeButton.setButtonText("Narrow Range");
    freqRangeButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    freqRangeAttach = std::make_unique<ButtonAttachment>(p.getValueTree(), "freqRange", freqRangeButton);

    addAndMakeVisible(presetSelector);
    presetSelector.addItem("Default", 1);
    presetSelector.addItem("Bass Boost", 2);
    presetSelector.addItem("Vocal Warmth", 3);
    presetSelector.setSelectedId(1, juce::dontSendNotification);
    presetAttach = std::make_unique<ComboBoxAttachment>(p.getValueTree(), "currentProgram", presetSelector);

    freqAttach = std::make_unique<SliderAttachment>(p.getValueTree(), "freq", freqKnob);
    weightAttach = std::make_unique<SliderAttachment>(p.getValueTree(), "weight", weightKnob);
    strengthAttach = std::make_unique<SliderAttachment>(p.getValueTree(), "strength", strengthKnob);

    setResizable(true, true);
    setResizeLimits(400, 300, 800, 600);
    setSize(500, 350);
}

WeightEditor::~WeightEditor()
{
    audioProcessor.getValueTree().removeParameterListener("freq", this);
    audioProcessor.getValueTree().removeParameterListener("freqRange", this);
    setLookAndFeel(nullptr);
}

void WeightEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient gradient(juce::Colour(0xff1a1a24), 0, 0,
        juce::Colour(0xff24242e), 0, getHeight(), false);
    g.setGradientFill(gradient);
    g.fillAll();

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::FontOptions().withName("Arial").withHeight(24.0f).withStyle("Bold")));
    g.drawFittedText("Weight", getLocalBounds().removeFromTop(50), juce::Justification::centred, 1);
}

void WeightEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(50);
    bounds.removeFromBottom(30);

    auto presetArea = bounds.removeFromBottom(30);
    presetSelector.setBounds(presetArea.reduced(10, 5).withWidth(150));

    auto controlArea = bounds.reduced(20);
    auto topRow = controlArea.removeFromTop(150);
    auto bottomRow = controlArea;

    juce::FlexBox freqBox;
    freqBox.flexDirection = juce::FlexBox::Direction::column;
    freqBox.items.add(juce::FlexItem(freqKnob).withMinWidth(80.0f).withMinHeight(80.0f).withFlex(1.0f));
    freqBox.items.add(juce::FlexItem(freqValueLabel).withHeight(20.0f));

    juce::FlexBox knobBox;
    knobBox.flexDirection = juce::FlexBox::Direction::row;
    knobBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    knobBox.alignItems = juce::FlexBox::AlignItems::center;
    knobBox.items.add(juce::FlexItem(freqBox).withMinWidth(100.0f).withFlex(1.0f).withMargin(10));
    knobBox.items.add(juce::FlexItem(weightKnob).withMinWidth(80.0f).withMinHeight(80.0f).withFlex(1.0f).withMargin(10));
    knobBox.items.add(juce::FlexItem(strengthKnob).withMinWidth(80.0f).withMinHeight(80.0f).withFlex(1.0f).withMargin(10));

    knobBox.performLayout(topRow);

    juce::FlexBox bottomBox;
    bottomBox.flexDirection = juce::FlexBox::Direction::row;
    bottomBox.justifyContent = juce::FlexBox::JustifyContent::spaceAround;
    bottomBox.alignItems = juce::FlexBox::AlignItems::center;
    bottomBox.items.add(juce::FlexItem(bypassButton).withWidth(40.0f).withHeight(40.0f).withMargin(10));
    bottomBox.items.add(juce::FlexItem(freqRangeButton).withWidth(100.0f).withHeight(30.0f).withMargin(10));

    bottomBox.performLayout(bottomRow);
}

void WeightEditor::parameterChanged(const juce::String& parameterID, float)
{
    if (parameterID == "freq" || parameterID == "freqRange")
        updateFrequencyDisplay();
}

void WeightEditor::updateFrequencyDisplay()
{
    bool narrowRange = audioProcessor.getValueTree().getParameter("freqRange")->getValue() > 0.5f;
    float freqVal = audioProcessor.getValueTree().getParameter("freq")->getValue();
    float freqHz = narrowRange ? (freqVal * 100.0f) + 20.0f : juce::mapToLog10(freqVal, 20.0f, 20000.0f);

    juce::String freqText;
    if (freqHz >= 1000.0f)
        freqText = juce::String(freqHz / 1000.0f, 1) + " kHz";
    else
        freqText = juce::String(static_cast<int>(freqHz + 0.5f)) + " Hz";

    freqValueLabel.setText(freqText, juce::dontSendNotification);
}