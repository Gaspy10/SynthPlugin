#include "PresetGenerationJob.h"
#include "OpenAIClient.h"

PresetGenerationJob::PresetGenerationJob(OpenAIClient& c,
    juce::String p,
    std::function<void(juce::var)> cb)
    : ThreadPoolJob("PresetGenerationJob"),
    client(c),
    prompt(std::move(p)),
    onFinished(std::move(cb))
{
}

juce::ThreadPoolJob::JobStatus PresetGenerationJob::runJob()
{
    if (shouldExit())
        return jobHasFinished;

    // Call OpenAI (blocking) on this worker thread
    juce::String outText;
    auto r = client.createTextResponse(
        prompt,
        outText,
        R"(
        You are a synthesizer preset generator.

        Return ONLY valid JSON.

        Preset format:
        {
          "name": string,
          "params": {
            "gain": number (-24..24),
            "cutoffLow": number (20..20000),
            "cutoffHigh": number (20..20000),
            "wave": number (0=sine, 1=square, 2=triangle, 3=saw),
            "attack": number (0..5000),
            "decay": number (0..5000),
            "sustain": number (0..1),
            "release": number (0..5000),
            "tremoloOn": number (0 or 1),
            "tremoloWave": number (0=sine, 1=square, 2=triangle, 3=saw),
            "tremoloFreq": number (0.1..20),
            "tremoloDepth": number (0..1)
          }
        }

        Rules:
        - Always include ALL parameters
        - Avoid extreme values unless explicitly requested
        - Values must be realistic for music
        - Always set the gain to 0
        )"
    );

    juce::var payload;

    if (r.failed())
    {
        // Return an error object to the UI
        auto* err = new juce::DynamicObject();
        err->setProperty("ok", false);
        err->setProperty("error", r.getErrorMessage());
        payload = juce::var(err);
    }
    else
    {
        auto parsed = juce::JSON::parse(outText);

        if (parsed.isVoid())
        {
            auto* err = new juce::DynamicObject();
            err->setProperty("ok", false);
            err->setProperty("error", "Model returned invalid JSON");
            err->setProperty("raw", outText);
            payload = juce::var(err);
        }
        else
        {
            auto* ok = new juce::DynamicObject();
            ok->setProperty("ok", true);
            ok->setProperty("data", parsed);
            payload = juce::var(ok);
        }
    }

    juce::MessageManager::callAsync([cb = onFinished, payload]() mutable
        {
            cb(payload);
        });

    return jobHasFinished;
}
