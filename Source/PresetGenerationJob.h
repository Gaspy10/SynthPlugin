#pragma once
#include <JuceHeader.h>
#include <functional>

class OpenAIClient;

class PresetGenerationJob : public juce::ThreadPoolJob
{
public:
    PresetGenerationJob(OpenAIClient& client,
        juce::String prompt,
        std::function<void(juce::var)> onFinished);

    JobStatus runJob() override;

private:
    OpenAIClient& client;
    juce::String prompt;
    std::function<void(juce::var)> onFinished;
};
