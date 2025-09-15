#include "PluginProcessor.h"
#include "PluginEditor.h"

// WeightProcessor implementation
WeightProcessor::WeightProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    apvts.state.setProperty("currentProgram", 0, nullptr); // Initialize preset index
    freqParamPtr = apvts.getRawParameterValue("freq");
    weightParamPtr = apvts.getRawParameterValue("weight");
    strengthParamPtr = apvts.getRawParameterValue("strength");
    bypassParamPtr = apvts.getRawParameterValue("bypass");
    freqRangeParamPtr = apvts.getRawParameterValue("freqRange");
}

juce::AudioProcessorValueTreeState::ParameterLayout WeightProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<CustomParameter>(
        juce::ParameterID("freq", 1), "Freq",
        juce::NormalisableRange<float>(0.0f, 1.0f),
        juce::mapFromLog10(120.0f, 20.0f, 20000.0f), "Hz", false));
    params.push_back(std::make_unique<CustomParameter>(
        juce::ParameterID("weight", 2), "Weight",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f, "%"));
    params.push_back(std::make_unique<CustomParameter>(
        juce::ParameterID("strength", 3), "Strength",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f, "%"));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("bypass", 4), "Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("freqRange", 5), "Freq Range", false,
        juce::AudioParameterBoolAttributes()
        .withStringFromValueFunction([](float val, int) {
            return val > 0.5f ? "Narrow (20–120 Hz)" : "Full (20–20k Hz)";
            })));
    return { params.begin(), params.end() };
}

void WeightProcessor::prepareToPlay(double, int)
{
    precisionProcessingFloat.prepare();
    precisionProcessingDouble.prepare();
}

bool WeightProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet())
        return true;

    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono();
}

template<typename T>
void WeightProcessor::processBlockT(juce::AudioBuffer<T>& buffer)
{
    juce::ScopedNoDenormals noDenormals;

    const bool bypass = bypassParamPtr->load(std::memory_order_relaxed) > 0.5f;
    const float weightVal = weightParamPtr->load(std::memory_order_relaxed);
    const float strengthVal = strengthParamPtr->load(std::memory_order_relaxed);

    if (bypass || weightVal == 0.0f)
        return;

    const float freqVal = freqParamPtr->load(std::memory_order_relaxed);

    auto& st = getPrecisionDependantProcessing<T>();
    auto* channelDataL = buffer.getWritePointer(0);
    auto* channelDataR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    double overallscale = getSampleRate() / 44100.0;
    double targetFreq = juce::mapToLog10(freqVal, 20.0f, 20000.0f) / getSampleRate();
    targetFreq = ((targetFreq + 0.53) * 0.2) / std::sqrt(overallscale);

    double alpha = std::pow(targetFreq, 4);
    double resControl = (weightVal * (0.05 + strengthVal * 0.1)) + (0.2 + strengthVal * 0.3);
    double beta = alpha * (resControl * resControl);
    alpha += (1.0 - beta) * std::pow(targetFreq, 3);

    for (int n = 0; n < buffer.getNumSamples(); ++n)
    {
        T dryL = channelDataL[n];
        T dryR = channelDataR ? channelDataR[n] : dryL;
        T xL = dryL;
        T xR = dryR;

        for (int i = 0; i < 8; ++i)
        {
            T trend = static_cast<T>(beta * (xL - st.prevL[i]) + (0.999 - beta) * st.trendL[i]);
            T forecast = static_cast<T>(st.prevL[i] + st.trendL[i]);
            xL = static_cast<T>(alpha * xL + (0.999 - alpha) * forecast);
            st.prevL[i] = xL;
            st.trendL[i] = trend;

            trend = static_cast<T>(beta * (xR - st.prevR[i]) + (0.999 - beta) * st.trendR[i]);
            forecast = static_cast<T>(st.prevR[i] + st.trendR[i]);
            xR = static_cast<T>(alpha * xR + (0.999 - alpha) * forecast);
            st.prevR[i] = xR;
            st.trendR[i] = trend;
        }

        xL = (xL * weightVal) + (dryL * (1.0f - weightVal));
        xR = (xR * weightVal) + (dryR * (1.0f - weightVal));

        if constexpr (std::is_same_v<T, float>)
        {
            int expon;
            frexpf(xL, &expon);
            st.fpdL ^= st.fpdL << 13; st.fpdL ^= st.fpdL >> 17; st.fpdL ^= st.fpdL << 5;
            xL += static_cast<T>((static_cast<int32_t>(st.fpdL)) * 5.5e-36l * std::pow(2, expon + 62));

            frexpf(xR, &expon);
            st.fpdR ^= st.fpdR << 13; st.fpdR ^= st.fpdR >> 17; st.fpdR ^= st.fpdR << 5;
            xR += static_cast<T>((static_cast<int32_t>(st.fpdR)) * 5.5e-36l * std::pow(2, expon + 62));
        }

        channelDataL[n] = xL;
        if (channelDataR) channelDataR[n] = xR;
    }
}

void WeightProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midi*/)
{
    processBlockT(buffer);
}

void WeightProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& /*midi*/)
{
    processBlockT(buffer);
}

const juce::String WeightProcessor::getName() const { return "Weight"; }
bool WeightProcessor::acceptsMidi() const { return false; }
bool WeightProcessor::producesMidi() const { return false; }
bool WeightProcessor::isMidiEffect() const { return false; }
double WeightProcessor::getTailLengthSeconds() const { return 0.0; }

int WeightProcessor::getNumPrograms() { return 3; }
int WeightProcessor::getCurrentProgram() { return apvts.state.getProperty("currentProgram", 0); }
void WeightProcessor::setCurrentProgram(int index)
{
    juce::ValueTree state = apvts.copyState();
    state.setProperty("currentProgram", index, nullptr);
    if (index == 0) // Default
    {
        state.setProperty("freq", juce::mapFromLog10(120.0f, 20.0f, 20000.0f), nullptr);
        state.setProperty("weight", 0.5f, nullptr);
        state.setProperty("strength", 0.5f, nullptr);
        state.setProperty("bypass", 0.0f, nullptr);
        state.setProperty("freqRange", 0.0f, nullptr);
    }
    else if (index == 1) // Bass Boost
    {
        state.setProperty("freq", juce::mapFromLog10(80.0f, 20.0f, 20000.0f), nullptr);
        state.setProperty("weight", 0.7f, nullptr);
        state.setProperty("strength", 0.6f, nullptr);
        state.setProperty("bypass", 0.0f, nullptr);
        state.setProperty("freqRange", 1.0f, nullptr);
    }
    else if (index == 2) // Vocal Warmth
    {
        state.setProperty("freq", juce::mapFromLog10(500.0f, 20.0f, 20000.0f), nullptr);
        state.setProperty("weight", 0.4f, nullptr);
        state.setProperty("strength", 0.4f, nullptr);
        state.setProperty("bypass", 0.0f, nullptr);
        state.setProperty("freqRange", 0.0f, nullptr);
    }
    apvts.replaceState(state);
}

const juce::String WeightProcessor::getProgramName(int index)
{
    switch (index)
    {
    case 0: return "Default";
    case 1: return "Bass Boost";
    case 2: return "Vocal Warmth";
    default: return "Unknown";
    }
}

void WeightProcessor::changeProgramName(int, const juce::String&) {}

void WeightProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void WeightProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WeightProcessor();
}

juce::AudioProcessorEditor* WeightProcessor::createEditor()
{
    return new WeightEditor(*this);
}