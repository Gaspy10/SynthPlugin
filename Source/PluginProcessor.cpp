#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
JuceSynthPluginAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using APC = juce::AudioParameterChoice;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<APC>(
        "wave", "Wave",
        juce::StringArray{ "Sine", "Square", "Triangle", "Sawtooth" }, 0));

    layout.add(std::make_unique<APC>(
        "tremoloWave", "TremoloWave",
        juce::StringArray{ "Sine", "Square", "Triangle", "Sawtooth" }, 0));

    layout.add(std::make_unique<APF>(
        "gain", "Gain (dB)", -24.0f, 24.0f, 0.0f));

    layout.add(std::make_unique<APF>(
        "cutoffLow", "Cutoff Low", 20.0f, 20000.0f, 1000.0f));

    layout.add(std::make_unique<APF>(
        "cutoffHigh", "Cutoff High", 20.0f, 20000.0f, 8000.0f));

    // ================= ADSR =================
    layout.add(std::make_unique<APF>(
        "attack", "Attack (ms)",
        juce::NormalisableRange<float>(1.0f, 5000.0f, 0.0f, 0.5f),
        100.0f));

    layout.add(std::make_unique<APF>(
        "decay", "Decay (ms)",
        juce::NormalisableRange<float>(1.0f, 5000.0f, 0.0f, 0.5f),
        500.0f));

    layout.add(std::make_unique<APF>(
        "sustain", "Sustain",
        juce::NormalisableRange<float>(0.001f, 1.0f),
        0.8f));

    layout.add(std::make_unique<APF>(
        "release", "Release (ms)",
        juce::NormalisableRange<float>(1.0f, 5000.0f, 0.0f, 0.5f),
        100.0f));

	layout.add(std::make_unique<APF>(
		"tremoloFreq", "Tremolo Frequency",
		juce::NormalisableRange<float>(0.1f, 20.0f),
		5.0f));

	layout.add(std::make_unique<APF>(
		"tremoloDepth", "Tremolo Depth",
		juce::NormalisableRange<float>(0.0f, 1.0f),
		0.5f));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        "tremoloOn",
        "Tremolo On",
        false
    ));


    return layout;
}

JuceSynthPluginAudioProcessor::JuceSynthPluginAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "PARAMS", createParameterLayout())
    , waveFormSettings(apvts)
{
    for (int i = 0; i < 10; ++i)
        synth.addVoice(new WavetableVoice(waveFormSettings));

    synth.addSound(new WavetableSound());
}

JuceSynthPluginAudioProcessor::~JuceSynthPluginAudioProcessor() = default;

//==============================================================================
const juce::String JuceSynthPluginAudioProcessor::getName() const { return JucePlugin_Name; }
bool JuceSynthPluginAudioProcessor::acceptsMidi() const { return true; }
bool JuceSynthPluginAudioProcessor::producesMidi() const { return false; }
bool JuceSynthPluginAudioProcessor::isMidiEffect() const { return false; }
double JuceSynthPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int JuceSynthPluginAudioProcessor::getNumPrograms() { return 1; }
int JuceSynthPluginAudioProcessor::getCurrentProgram() { return 0; }
void JuceSynthPluginAudioProcessor::setCurrentProgram(int) {}
const juce::String JuceSynthPluginAudioProcessor::getProgramName(int) { return {}; }
void JuceSynthPluginAudioProcessor::changeProgramName(int, const juce::String&) {}

void JuceSynthPluginAudioProcessor::prepareToPlay(double sampleRate, int _samplesPerBlock)
{
	samplesPerBlock = _samplesPerBlock;
    lfoBuffer.setSize(1, samplesPerBlock);

    synth.setCurrentPlaybackSampleRate(sampleRate);

    // Ported from SynthAudioSource::prepareToPlay:
    maxiSettings::setup(sampleRate, 2, 1024);

    // If voices/filters need reset, do it here (safe, not realtime).
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<WavetableVoice*>(synth.getVoice(i)))
            v->prepare(sampleRate);
}

void JuceSynthPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear any extra output channels
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    // Merge on-screen keyboard MIDI into the host MIDI buffer
    keyboardState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true);

    buffer.clear();

    for (int i = 0; i < samplesPerBlock; ++i) {
        auto c = waveFormSettings.getLfoWaveValue();
        auto freq = waveFormSettings.getLfoFreqValue();
        auto depth = waveFormSettings.getLfoDepthValue();

        if (!waveFormSettings.getLfoOnValue()) {
            lfoBuffer.setSample(0, i, 1.0);
            continue;
        }

        switch (c) {
            case WaveFormSettings::WaveForms::sine: {
                lfoBuffer.setSample(0, i, 1 - 0.5 * depth + 0.5 * depth * tremoloOsc.sinewave(freq));
                break;
            }
            case WaveFormSettings::WaveForms::square: {
                lfoBuffer.setSample(0, i, 1 - 0.5 * depth + 0.5 * depth * tremoloOsc.square(freq));
                break;
            }
            case WaveFormSettings::WaveForms::triangle: {
                lfoBuffer.setSample(0, i, 1 - 0.5 * depth + 0.5 * depth * tremoloOsc.triangle(freq));
                break;
            }
            case WaveFormSettings::WaveForms::sawtooth: {
                lfoBuffer.setSample(0, i, 1 - 0.5 * depth + 0.5 * depth * tremoloOsc.saw(freq));
                break;
            }
        }
    }

	for (int i = 0; i < synth.getNumVoices(); ++i)
	{
		if (auto* voice = dynamic_cast<WavetableVoice*>(synth.getVoice(i)))
		{
			voice->setGlobalLfo(lfoBuffer.getReadPointer(0));
		}
	}

    // Render
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply gain parameter (optional; you can also apply inside voices)
    const float gainDb = apvts.getRawParameterValue("gain")->load();
    buffer.applyGain(juce::Decibels::decibelsToGain(gainDb));
}

//==============================================================================
void JuceSynthPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void JuceSynthPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

//==============================================================================
juce::AudioProcessorEditor* JuceSynthPluginAudioProcessor::createEditor()
{
    return new PluginEditor(*this);
}


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JuceSynthPluginAudioProcessor();
}

