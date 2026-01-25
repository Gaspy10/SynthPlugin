#pragma once
#include <JuceHeader.h>

// Your existing classes:
#include "WavetableVoice.h"
#include "WavetableSound.h"
#include "WaveFormSettings.h"

// If you use Maximilian:
#include "maximilian.h"

class JuceSynthPluginAudioProcessor : public juce::AudioProcessor
{
public:
    JuceSynthPluginAudioProcessor();
    ~JuceSynthPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // ----------------- Things your Editor expects -----------------
    juce::MidiKeyboardState keyboardState;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    // ----------------- Synth moved from SynthAudioSource -----------------
    juce::Synthesiser synth;
	juce::AudioBuffer<double> lfoBuffer;
	maxiOsc tremoloOsc;

    int samplesPerBlock;

    // IMPORTANT: This must be processor-owned and NOT depend on GUI widgets.
    WaveFormSettings waveFormSettings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JuceSynthPluginAudioProcessor)
};
