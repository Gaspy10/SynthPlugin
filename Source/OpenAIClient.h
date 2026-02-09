#pragma once
#include <JuceHeader.h>

class OpenAIClient
{
public:
    struct Config
    {
        juce::String apiKey = "";                 // Delete before pushing
        juce::String model = "gpt-4o-mini";
        int timeoutMs = 15000;
        int maxOutputTokens = 500;
        bool store = false;
    };

    explicit OpenAIClient(Config cfg);

    juce::Result createTextResponse(const juce::String& userPrompt,
                                   juce::String& outText,
                                   const juce::String& instructions = {}) const;

    juce::Result createJsonSchemaResponse(const juce::String& userPrompt,
                                         const juce::var& jsonSchema, // var holding schema object
                                         juce::String& outJsonText,
                                         const juce::String& schemaName = "SynthPreset",
                                         const juce::String& instructions = {}) const;

private:
    Config cfg;

    juce::Result postResponses(const juce::String& bodyJson, juce::String& outResponseJson) const;

    static juce::String extractOutputText(const juce::String& responseJson);
    static juce::String sanitizeAuthHeaderValue(const juce::String& apiKey);
};
