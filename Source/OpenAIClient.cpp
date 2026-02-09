#include "OpenAIClient.h"

OpenAIClient::OpenAIClient(Config c) : cfg(std::move(c)) {}

juce::String OpenAIClient::sanitizeAuthHeaderValue(const juce::String& apiKey)
{
    return apiKey.trim();
}

juce::Result OpenAIClient::postResponses(const juce::String& bodyJson,
    juce::String& outResponseJson) const
{
    if (cfg.apiKey.isEmpty())
        return juce::Result::fail("OpenAIClient: apiKey is empty.");

    juce::URL url("https://api.openai.com/v1/responses");

    url = url.withPOSTData(bodyJson);

    juce::String headers;
    headers << "Authorization: Bearer " << sanitizeAuthHeaderValue(cfg.apiKey) << "\r\n"
        << "Content-Type: application/json";

    int statusCode = 0;
    juce::StringPairArray responseHeaders;

    auto stream = url.createInputStream(
        true,               // usePostCommand
        nullptr,            // OpenStreamProgressCallback* cb
        nullptr,            // void* context
        headers,            // String headers
        cfg.timeoutMs,      // int timeOutMs
        &responseHeaders,   // StringPairArray* responseHeaders
        &statusCode,        // int* statusCode
        3,                  // int numRedirectsToFollow
        "POST"              // String httpRequestCmd
    );

    if (stream == nullptr)
        return juce::Result::fail("OpenAIClient: failed to open HTTPS connection.");

    const auto responseText = stream->readEntireStreamAsString();

    if (statusCode < 200 || statusCode >= 300)
        return juce::Result::fail("OpenAIClient HTTP " + juce::String(statusCode) + ": " + responseText);

    outResponseJson = responseText;
    return juce::Result::ok();
}



juce::String OpenAIClient::extractOutputText(const juce::String& responseJson)
{
    const auto parsed = juce::JSON::parse(responseJson);
    if (parsed.isVoid() || !parsed.isObject())
        return {};

    const auto* obj = parsed.getDynamicObject();
    if (obj == nullptr)
        return {};

    const auto outputVar = obj->getProperty("output");
    if (!outputVar.isArray())
        return {};

    juce::String result;

    for (const auto& item : *outputVar.getArray())
    {
        if (!item.isObject()) continue;
        const auto* itemObj = item.getDynamicObject();
        if (!itemObj) continue;

        if (itemObj->getProperty("type").toString() != "message")
            continue;

        const auto contentVar = itemObj->getProperty("content");
        if (!contentVar.isArray())
            continue;

        for (const auto& c : *contentVar.getArray())
        {
            if (!c.isObject()) continue;
            const auto* cObj = c.getDynamicObject();
            if (!cObj) continue;

            if (cObj->getProperty("type").toString() == "output_text")
            {
                result << cObj->getProperty("text").toString();
            }
        }
    }

    return result.trim();
}

juce::Result OpenAIClient::createTextResponse(const juce::String& userPrompt,
                                             juce::String& outText,
                                             const juce::String& instructions) const
{
    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty("model", cfg.model);
    root->setProperty("store", cfg.store);
    root->setProperty("max_output_tokens", cfg.maxOutputTokens);

    if (!instructions.isEmpty())
        root->setProperty("instructions", instructions);

    // input can be a string (simple case)
    root->setProperty("input", userPrompt);

    // plain text format
    {
        juce::DynamicObject::Ptr textObj = new juce::DynamicObject();
        juce::DynamicObject::Ptr fmtObj  = new juce::DynamicObject();
        fmtObj->setProperty("type", "text");
        textObj->setProperty("format", juce::var(fmtObj.get()));
        root->setProperty("text", juce::var(textObj.get()));
    }

    const auto body = juce::JSON::toString(juce::var(root.get()));

    juce::String raw;
    auto r = postResponses(body, raw);
    if (r.failed())
        return r;

    outText = extractOutputText(raw);
    if (outText.isEmpty())
        return juce::Result::fail("OpenAIClient: response contained no output_text.");

    return juce::Result::ok();
}

juce::Result OpenAIClient::createJsonSchemaResponse(const juce::String& userPrompt,
                                                   const juce::var& jsonSchema,
                                                   juce::String& outJsonText,
                                                   const juce::String& schemaName,
                                                   const juce::String& instructions) const
{

    juce::DynamicObject::Ptr root = new juce::DynamicObject();
    root->setProperty("model", cfg.model);
    root->setProperty("store", cfg.store);
    root->setProperty("max_output_tokens", cfg.maxOutputTokens);

    if (!instructions.isEmpty())
        root->setProperty("instructions", instructions);

    root->setProperty("input", userPrompt);

    // text.format = { type: "json_schema", name, strict, schema }
    {
        juce::DynamicObject::Ptr fmtObj = new juce::DynamicObject();
        fmtObj->setProperty("type", "json_schema");
        fmtObj->setProperty("name", schemaName);
        fmtObj->setProperty("strict", true);
        fmtObj->setProperty("schema", jsonSchema);

        juce::DynamicObject::Ptr textObj = new juce::DynamicObject();
        textObj->setProperty("format", juce::var(fmtObj.get()));
        root->setProperty("text", juce::var(textObj.get()));
    }

    const auto body = juce::JSON::toString(juce::var(root.get()));

    juce::String raw;
    auto r = postResponses(body, raw);
    if (r.failed())
        return r;

    outJsonText = extractOutputText(raw);
    if (outJsonText.isEmpty())
        return juce::Result::fail("OpenAIClient: response contained no output_text.");

    return juce::Result::ok();
}
