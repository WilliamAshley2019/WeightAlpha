#pragma once
#include <JuceHeader.h>

// Forward-declare the processor class to avoid circular includes
class WeightAlphaProcessor;

// A modern, clean LookAndFeel for the "Weight Alpha" plugin
class WeightAlphaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    WeightAlphaLookAndFeel()
    {
        const auto baseColour = juce::Colour(0xff2b303b);
        const auto accentColour = juce::Colour(0xff8ab4f8);
        const auto textColour = juce::Colour(0xffc2c8d7);
        const auto outlineColour = baseColour.brighter(0.2f);
        const auto thumbColour = textColour.brighter(0.2f);

        setColour(juce::Slider::thumbColourId, thumbColour);
        setColour(juce::Slider::rotarySliderFillColourId, accentColour);
        setColour(juce::Slider::rotarySliderOutlineColourId, outlineColour);
        setColour(juce::Label::textColourId, textColour);
        setColour(juce::ToggleButton::textColourId, textColour);
        setColour(juce::ToggleButton::tickColourId, accentColour);
        setColour(juce::ToggleButton::tickDisabledColourId, outlineColour);
        setColour(juce::ComboBox::backgroundColourId, baseColour.brighter(0.1f));
        setColour(juce::ComboBox::outlineColourId, outlineColour);
        setColour(juce::ComboBox::arrowColourId, accentColour);
        setColour(juce::ComboBox::textColourId, textColour);
        setColour(juce::PopupMenu::backgroundColourId, baseColour);
        setColour(juce::PopupMenu::textColourId, textColour);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, accentColour.withAlpha(0.3f));
        setColour(juce::PopupMenu::highlightedTextColourId, textColour.brighter());
        setDefaultSansSerifTypefaceName("Inter");
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        juce::ignoreUnused(slider);
        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)).reduced(10.0f);
        if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0) return; // Prevent rendering if bounds are invalid
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = radius * 0.15f;
        auto arcRadius = radius - lineW * 0.5f;

        juce::Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        auto pointerLength = radius * 0.8f;
        auto pointerThickness = lineW * 0.5f;
        juce::Path pointer;
        pointer.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        pointer.applyTransform(juce::AffineTransform::rotation(toAngle).translated(bounds.getCentre()));
        g.setColour(findColour(juce::Slider::thumbColourId));
        g.fillPath(pointer);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto colour = findColour(button.getToggleState() ? juce::ToggleButton::tickColourId : juce::ToggleButton::tickDisabledColourId);
        g.setColour(colour.withAlpha(shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown ? 1.0f : 0.8f));

        if (button.getName() == "Bypass")
        {
            bounds.reduce(5.0f, 5.0f);
            auto size = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.7f;
            auto r = bounds.withSizeKeepingCentre(size, size);
            float ang = juce::MathConstants<float>::pi * 0.25f;

            juce::Path powerSymbol;
            powerSymbol.addArc(r.getX(), r.getY(), r.getWidth(), r.getHeight(), ang, juce::MathConstants<float>::twoPi - ang, true);
            powerSymbol.startNewSubPath(r.getCentreX(), r.getY());
            powerSymbol.lineTo(r.getCentreX(), r.getCentreY());

            auto strokeType = juce::PathStrokeType(size * 0.15f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
            g.strokePath(powerSymbol, strokeType);
        }
        else
        {
            g.fillRoundedRectangle(bounds.reduced(5.0f), 4.0f);
        }
    }
};

// Custom listener class for parameter changes
class ParameterListener : public juce::AudioProcessorParameter::Listener
{
public:
    ParameterListener(std::function<void(float, float)> callback) : onParameterChange(callback) {}

    void parameterValueChanged(int, float) override
    {
        if (onParameterChange)
            onParameterChange(0.0f, 0.0f);
    }

    void parameterGestureChanged(int, bool) override {}

private:
    std::function<void(float, float)> onParameterChange;
};

// The main editor component for the plugin, with a Timer for UI updates
class WeightAlphaEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit WeightAlphaEditor(WeightAlphaProcessor&);
    ~WeightAlphaEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);
    void updateFrequencyDisplay();

    WeightAlphaProcessor& audioProcessor;
    WeightAlphaLookAndFeel lookAndFeel;

    juce::Slider freqKnob, weightKnob, strengthKnob;
    juce::Label freqLabel, weightLabel, strengthLabel, freqValueLabel, titleLabel, bypassLabel;

    juce::ToggleButton bypassButton, freqRangeButton;
    juce::ComboBox presetSelector;

    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;
    using ButtonAttachment = APVTS::ButtonAttachment;
    using ComboBoxAttachment = APVTS::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> freqAttach, weightAttach, strengthAttach;
    std::unique_ptr<ButtonAttachment> bypassAttach, freqRangeAttach;
    std::unique_ptr<ComboBoxAttachment> presetAttach;

    ParameterListener freqListener;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WeightAlphaEditor)
};