#pragma once
#include <JuceHeader.h>

class WaveFormSettings
{
public:
    enum class WaveForms
    {
        sine = 0,
        square,
        triangle,
        sawtooth
    };

    // Processor-owned: pass APVTS so we can read parameters safely
    explicit WaveFormSettings (juce::AudioProcessorValueTreeState& apvts);

    WaveForms getSelectedWaveForm() const noexcept;

    // Linear gain (not dB) - same meaning as your old getVelocity()
    float getVelocity() const noexcept;

    float getCutoffLowFrequency() const noexcept;
    float getCutoffHighFrequency() const noexcept;

    float getAttackValue() const noexcept;
    float getDecayValue() const noexcept;
    float getReleaseValue() const noexcept;
    float getSustainValue() const noexcept;

	bool getLfoOnValue() const noexcept;
	WaveForms getLfoWaveValue() const noexcept;
	float getLfoFreqValue() const noexcept;
	float getLfoDepthValue() const noexcept;

private:
    std::atomic<float>* waveParam      = nullptr; // choice stored as float index
    std::atomic<float>* gainDbParam    = nullptr; // -24..24
    std::atomic<float>* cutoffLowParam = nullptr; // Hz
    std::atomic<float>* cutoffHighParam= nullptr; // Hz

	std::atomic<float>* attackParam = nullptr; // miliseconds
	std::atomic<float>* decayParam = nullptr; // miliseconds
	std::atomic<float>* sustainParam = nullptr; // 0 - 1 
	std::atomic<float>* releaseParam = nullptr; // miliseconds

	std::atomic<float>* lfoOnParam = nullptr; // on or off
	std::atomic<float>* lfoWaveParam = nullptr; // choice stored as float index
	std::atomic<float>* lfoFreqParam = nullptr; // Hz
	std::atomic<float>* lfoDepthParam = nullptr; // 0 - 1
};

