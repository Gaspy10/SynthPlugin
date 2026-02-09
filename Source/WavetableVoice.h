/*
  ==============================================================================

    WavetableVoice.h
    Created: 24 Jul 2025 12:03:28pm
    Author:  D

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "WaveFormSettings.h"
#include "FIRfilter.h"
#include "maximilian.h"
#include <juce_dsp/juce_dsp.h>

class WavetableVoice : public juce::SynthesiserVoice
{
public:
    WavetableVoice(WaveFormSettings& w);

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}
    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
    void setCurrentPlaybackSampleRate(double newRate) override;
    void prepare(int sampleRate);
    void setGlobalLfo(const double* data);

private:
    double getNextSample();
    double amplify(double sample) const;

    // These must be declared here:
    double level = 0.0;
    double tailOff = 0.0;
    float frequency = 0;

	const double* globalLfoData = nullptr;

    double envValue = 0.0;

    WaveFormSettings& waveFormSettings;

	maxiOsc osc;
	maxiEnv env;
    FIRFilter filter;
};
