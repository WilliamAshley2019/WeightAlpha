#pragma once
#include <JuceHeader.h>

// Custom parameter class for flexible display and conversion
struct CustomParameter : public juce::AudioParameterFloat
{
    CustomParameter(const juce::ParameterID& id, const juce::String& name,
        juce::NormalisableRange<float> range, float defaultVal,
        const juce::String& label = "",
        bool useNarrowFreqRange = false)
        : juce::AudioParameterFloat(id, name, range, defaultVal,
            juce::AudioParameterFloatAttributes()
            .withLabel(label)
            .withStringFromValueFunction(
                [name, useNarrowFreqRange](float val, int) {
                    if (name == "Freq") {
                        if (useNarrowFreqRange)
                            return juce::String((val * 100.0f) + 20.0f, 1) + " Hz";
                        return juce::String(juce::mapToLog10(val, 20.0f, 20000.0f), 1) + " Hz";
                    }
                    if (name == "Weight" || name == "Strength")
                        return juce::String(val * 100.0f, 1) + " %";
                    return juce::String(val, 2);
                })
            .withValueFromStringFunction(
                [name, useNarrowFreqRange](const juce::String& t) {
                    if (name == "Freq") {
                        if (useNarrowFreqRange)
                            return (t.getFloatValue() - 20.0f) / 100.0f;
                        return juce::mapFromLog10(t.getFloatValue(), 20.0f, 20000.0f);
                    }
                    if (name == "Weight" || name == "Strength")
                        return t.getFloatValue() / 100.0f;
                    return t.getFloatValue();
                }))
    {
    }
};

// Utility functions for logarithmic mapping
namespace juce
{
    inline float mapToLog10(float value, float min, float max)
    {
        return min * std::pow(max / min, value);
    }

    inline float mapFromLog10(float value, float min, float max)
    {
        return std::log(value / min) / std::log(max / min);
    }
}

// The main audio processor for the "Weight Alpha" plugin
class WeightAlphaProcessor : public juce::AudioProcessor
{
public:
    WeightAlphaProcessor();
    ~WeightAlphaProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock(juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    bool supportsDoublePrecisionProcessing() const override { return true; }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Weight Alpha"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 3; }
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTree() { return apvts; }

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<float>* freqParamPtr = nullptr;
    std::atomic<float>* weightParamPtr = nullptr;
    std::atomic<float>* strengthParamPtr = nullptr;
    std::atomic<float>* bypassParamPtr = nullptr;
    std::atomic<float>* freqRangeParamPtr = nullptr;

    template<typename T>
    struct PrecisionDependantProcessing
    {
        std::array<T, 8> prevL{}, prevR{}, trendL{}, trendR{};
        uint32_t fpdL{ 1 }, fpdR{ 1 };

        void prepare()
        {
            juce::Random rng;
            fpdL = static_cast<uint32_t>(rng.nextInt(juce::Range<int>(16386, std::numeric_limits<int>::max())));
            fpdR = static_cast<uint32_t>(rng.nextInt(juce::Range<int>(16386, std::numeric_limits<int>::max())));
        }
    };

    PrecisionDependantProcessing<float> precisionProcessingFloat;
    PrecisionDependantProcessing<double> precisionProcessingDouble;

    template<typename T>
    PrecisionDependantProcessing<T>& getPrecisionDependantProcessing()
    {
        if constexpr (std::is_same_v<T, float>)
            return precisionProcessingFloat;
        else
            return precisionProcessingDouble;
    }

    template<typename T>
    void processBlockT(juce::AudioBuffer<T>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WeightAlphaProcessor)
};

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();