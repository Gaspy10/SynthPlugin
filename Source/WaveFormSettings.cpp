#include "WaveFormSettings.h"

WaveFormSettings::WaveFormSettings (juce::AudioProcessorValueTreeState& apvts)
{
    // These IDs must match your APVTS layout:
    // "wave", "gain", "cutoffLow", "cutoffHigh"
    waveParam       = apvts.getRawParameterValue ("wave");
    gainDbParam     = apvts.getRawParameterValue ("gain");
    cutoffLowParam  = apvts.getRawParameterValue ("cutoffLow");
    cutoffHighParam = apvts.getRawParameterValue ("cutoffHigh");
	attackParam = apvts.getRawParameterValue("attack");
    decayParam = apvts.getRawParameterValue ("decay");
    sustainParam = apvts.getRawParameterValue ("sustain");
    releaseParam = apvts.getRawParameterValue ("release");

    jassert (waveParam && gainDbParam && cutoffLowParam && cutoffHighParam);
}

WaveFormSettings::WaveForms WaveFormSettings::getSelectedWaveForm() const noexcept
{
    // In APVTS, AudioParameterChoice value is stored as float index: 0,1,2,3...
    const int idx = (waveParam != nullptr) ? (int) waveParam->load() : 0;

    switch (idx)
    {
        case 0: return WaveForms::sine;
        case 1: return WaveForms::square;
        case 2: return WaveForms::triangle;
        case 3: return WaveForms::sawtooth;
        default: return WaveForms::sine;
    }
}

float WaveFormSettings::getVelocity() const noexcept
{
    const float gainDb = (gainDbParam != nullptr) ? gainDbParam->load() : 0.0f;
    return juce::Decibels::decibelsToGain (gainDb);
}

float WaveFormSettings::getCutoffLowFrequency() const noexcept
{
    return (cutoffLowParam != nullptr) ? cutoffLowParam->load() : 20.0f;
}

float WaveFormSettings::getCutoffHighFrequency() const noexcept
{
    return (cutoffHighParam != nullptr) ? cutoffHighParam->load() : 20000.0f;
}

float WaveFormSettings::getAttackValue() const noexcept
{
    return (attackParam != nullptr) ? attackParam->load() : 100;
}

float WaveFormSettings::getDecayValue() const noexcept
{
    return (decayParam != nullptr) ? decayParam->load() : 500;
}

float WaveFormSettings::getSustainValue() const noexcept
{
    return (sustainParam != nullptr) ? sustainParam->load() : 0.8;
}

float WaveFormSettings::getReleaseValue() const noexcept
{
    return (releaseParam != nullptr) ? releaseParam->load() : 100;
}
