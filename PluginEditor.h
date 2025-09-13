#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Custom LookAndFeel for a modern, clean UI
class WeightLookAndFeel : public juce::LookAndFeel_V4
{
public:
    WeightLookAndFeel()
    {
        setDefaultSansSerifTypefaceName("Arial");
        setColour(juce::Slider::thumbColourId, juce::Colour(0xffe6e6e6));
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff6b85de));
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff3f3f4e));
        setColour(juce::ToggleButton::textColourId, juce::Colours::white);
        setColour(juce::Label::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff3f3f4e));
        setColour(juce::ComboBox::outlineColourId, juce::Colours::white);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider&) override
    {
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
            static_cast<float>(width), static_cast<float>(height)).reduced(10.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = radius * 0.1f;
        auto arcRadius = radius - lineW * 0.5f;

        // Background Arc
        juce::Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius,
            0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Value Arc
        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius,
            0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Thumb with glow
        auto thumbPoint = bounds.getCentre().getPointOnCircumference(radius - lineW, toAngle);
        g.setColour(findColour(juce::Slider::thumbColourId).withAlpha(0.3f));
        g.fillEllipse(juce::Rectangle<float>(12.0f, 12.0f).withCentre(thumbPoint));
        g.setColour(findColour(juce::Slider::thumbColourId));
        g.fillEllipse(juce::Rectangle<float>(6.0f, 6.0f).withCentre(thumbPoint));
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool, bool) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(4.0f);
        bool isOn = button.getToggleState();

        auto size = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.6f;
        auto r = bounds.withSizeKeepingCentre(size, size);
        float ang = juce::MathConstants<float>::pi * 0.25f;

        juce::Path powerSymbol;
        powerSymbol.addArc(r.getX(), r.getY(), r.getWidth(), r.getHeight(), ang, juce::MathConstants<float>::twoPi - ang, true);
        powerSymbol.startNewSubPath(r.getCentreX(), r.getY());
        powerSymbol.lineTo(r.getCentreX(), r.getCentreY());

        auto strokeType = juce::PathStrokeType(size * 0.15f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
        g.setColour(isOn ? juce::Colour(0xff6b85de) : juce::Colour(0xff3f3f4e));
        g.strokePath(powerSymbol, strokeType);

        // Bypass indicator
        g.setColour(isOn ? juce::Colours::green : juce::Colours::red);
        g.fillEllipse(juce::Rectangle<float>(10.0f, 10.0f).withCentre(bounds.getTopRight().translated(-10.0f, 10.0f)));
    }
};

class WeightEditor : public juce::AudioProcessorEditor,
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit WeightEditor(WeightProcessor&);
    ~WeightEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void updateFrequencyDisplay();

    WeightProcessor& audioProcessor;
    WeightLookAndFeel lookAndFeel;

    juce::Slider freqKnob, weightKnob, strengthKnob;
    juce::ToggleButton bypassButton;
    juce::ToggleButton freqRangeButton;
    juce::ComboBox presetSelector;

    juce::Label freqLabel, weightLabel, strengthLabel, freqValueLabel, bypassLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> freqAttach, weightAttach, strengthAttach;
    std::unique_ptr<ButtonAttachment> bypassAttach, freqRangeAttach;
    std::unique_ptr<ComboBoxAttachment> presetAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WeightEditor)
};