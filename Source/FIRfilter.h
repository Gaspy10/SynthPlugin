 #pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>

class FIRFilter
{
public:
    FIRFilter() = default;

    FIRFilter(int numTaps, double sampleRate)
        : taps(numTaps),
        fs(sampleRate),
        buffer(numTaps, 0.0f),
        coeffs(numTaps, 0.0f),
		coeffsHighPass(numTaps, 0.0f)
    {
        jassert(numTaps >= 3);
		tempBufferA.resize(numTaps, 0.0f);
		tempBufferB.resize(numTaps, 0.0f);
        setCutoff(cutoffLow, cutoffHigh); // default until user sets
    }

    void setCutoff(float cutoffHzLow, float cutoffHzHigh)
    {
        cutoffLow = cutoffHzLow;
		cutoffHigh = cutoffHzHigh;

		generateCoefficients(cutoffHzLow, tempBufferA);
		generateCoefficients(cutoffHzHigh, tempBufferB);

        for (int n = 0; n < taps; n++)
        {
			coeffs[n] = tempBufferB[n] - tempBufferA[n];
        }
    }

    float processSample(float x) noexcept
    {
        buffer[index] = x;

        float y = 0.0f;
        int bufIdx = index;

        // low-pass
        for (int i = 0; i < taps; i++)
        {
            y += coeffs[i] * buffer[bufIdx];
            bufIdx = (bufIdx == 0 ? taps - 1 : bufIdx - 1);
        }

        index = (index + 1) % taps;
        return y;
    }

    void processBlock(juce::AudioBuffer<float>& bufferToProcess)
    {
        const int numSamples = bufferToProcess.getNumSamples();
        const int numChannels = bufferToProcess.getNumChannels();

        for (int ch = 0; ch < numChannels; ch++)
        {
            float* data = bufferToProcess.getWritePointer(ch);

            for (int i = 0; i < numSamples; i++)
                data[i] = processSample(data[i]);
        }
    }

private:
    int taps;
    double fs;
    float cutoffLow = 20000.0f;
	float cutoffHigh = 20.0f;

    std::vector<float> coeffs;
	std::vector<float> coeffsHighPass;
    std::vector<float> buffer;

	std::vector<float> tempBufferA;
    std::vector<float> tempBufferB;

    int index = 0;

    void generateCoefficients(float cutoffHz, std::vector<float> & c)
    {
        const float fc = cutoffHz / fs;  // normalized 0..0.5
        const int M = taps - 1;

        for (int n = 0; n < taps; n++)
        {
            int k = n - M / 2;

            // Ideal sinc low-pass
            float sinc = (k == 0)
                ? 2.0f * fc
                : std::sin(2.0f * juce::MathConstants<float>::pi * fc * k)
                / (juce::MathConstants<float>::pi * k);

            // Hamming window
            float w = 0.54f - 0.46f * std::cos(2.0f * juce::MathConstants<float>::pi * n / M);

            c[n] = sinc * w;
        }
    }
};
