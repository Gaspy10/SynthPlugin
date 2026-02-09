#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor (JuceSynthPluginAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      keyboardComponent (processor.keyboardState,
                         juce::MidiKeyboardComponent::horizontalKeyboard),
        openAIClient(OpenAIClient::Config{
        /*apiKey*/   "",
        /*model*/    "gpt-5.2",
        /*timeoutMs*/15000,
        /*maxOutputTokens*/500,
        /*store*/    false
            })
{   
    generateButton.onClick = [this]()
    {
        generatePreset(); // background thread
    };

    addAndMakeVisible (keyboardComponent);

    addAndMakeVisible (waveForm);
    addAndMakeVisible (decibelSlider);
    addAndMakeVisible (decibelLabel);
	
    addAndMakeVisible (cutoffLowSlider);
    addAndMakeVisible (cutoffHighSlider);

    addAndMakeVisible (attackSlider);
    addAndMakeVisible (decaySlider);
    addAndMakeVisible (releaseSlider);
    addAndMakeVisible (sustainSlider);

    addAndMakeVisible(tremoloLabel);
	addAndMakeVisible (tremoloButton);
    addAndMakeVisible(tremoloWaveForm);
    addAndMakeVisible(tremoloFreqSlider);
	addAndMakeVisible(tremoloDepthSlider);

    addAndMakeVisible(promptBox);
    addAndMakeVisible(generateButton);

    // ================= LABEL =================
    decibelLabel.setText ("Noise Level in dB", juce::dontSendNotification);
	tremoloLabel.setText("Tremolo: ", juce::dontSendNotification);

    // ================= SLIDERS =================
    decibelSlider.setRange (-24.0, 24.0);
    decibelSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);

    cutoffLowSlider.setRange (20.0, 20000.0);
    cutoffHighSlider.setRange (20.0, 20000.0);

	attackSlider.setRange(0, 5000.0);
	sustainSlider.setRange(0.1, 1.0);
	decaySlider.setRange(0, 5000.0);
	releaseSlider.setRange(0, 5000.0);

    waveForm.addItem ("Sine", 1);
    waveForm.addItem ("Square", 2);
    waveForm.addItem ("Triangle", 3);
    waveForm.addItem ("Sawtooth", 4);

    tremoloWaveForm.addItem("Sine", 1);
    tremoloWaveForm.addItem("Square", 2);
    tremoloWaveForm.addItem("Triangle", 3);
    tremoloWaveForm.addItem("Sawtooth", 4);
	tremoloFreqSlider.setRange(0.1, 20.0);
	tremoloDepthSlider.setRange(0.0, 1.0);

    // ================= PARAMETER ATTACHMENTS =================
    auto& apvts = processor.apvts;

    gainAttachment = std::make_unique<SliderAttachment> (
        apvts, "gain", decibelSlider);

    cutoffLowAttachment = std::make_unique<SliderAttachment> (
        apvts, "cutoffLow", cutoffLowSlider);

    cutoffHighAttachment = std::make_unique<SliderAttachment> (
        apvts, "cutoffHigh", cutoffHighSlider);

    waveAttachment = std::make_unique<ComboBoxAttachment> (
        apvts, "wave", waveForm);

    attackAttachment = std::make_unique<SliderAttachment>(
        apvts, "attack", attackSlider);

    decayAttachment = std::make_unique<SliderAttachment>(
        apvts, "decay", decaySlider);

    sustainAttachment = std::make_unique<SliderAttachment>(
        apvts, "sustain", sustainSlider);

    releaseAttachment = std::make_unique<SliderAttachment>(
        apvts, "release", releaseSlider);

    tremoloWaveAttachment = std::make_unique<ComboBoxAttachment>(
        apvts, "tremoloWave", tremoloWaveForm);

	tremoloButtonAttachment = std::make_unique<ToggleButtonAttachment>(
		apvts, "tremoloOn", tremoloButton);

	tremoloDepthAttachment = std::make_unique<SliderAttachment>(
		apvts, "tremoloDepth", tremoloDepthSlider);

	tremoloFreqAttachment = std::make_unique<SliderAttachment>(
		apvts, "tremoloFreq", tremoloFreqSlider);

    setSize (600, 415);

    startTimerHz (30); 
}

PluginEditor::~PluginEditor() = default;

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void PluginEditor::generatePreset() {
    auto promptText = promptBox.getText(); // UI thread
    promptBox.setText("");

    auto* job = new PresetGenerationJob(
        openAIClient,
        promptText,
        [this](juce::var result) { applyPreset(result); }
    );
    threadPool.addJob(job, true);

}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    // keyboard
    const int keyboardH = 120;
    keyboardComponent.setBounds(area.removeFromTop(keyboardH));
    area.removeFromTop(10); // gap

    // split into left/right with a gap ---
    const int colGap = 10;
    auto left = area.removeFromLeft(area.getWidth() / 2);
    area.removeFromLeft(colGap);
    auto right = area;

    // Helpers
    const int rowH = 32;
    const int gap = 8;

    right.setHeight(4 * rowH + 4 * gap);
    left.setHeight(4* rowH + 4 * gap);

    auto takeRow = [&](juce::Rectangle<int>& r)
    {
        auto row = r.removeFromTop(rowH);
        r.removeFromTop(gap);
        return row;
    };

    // Waveform + Gain + Cutoffs
    {
        auto row1 = takeRow(left);
        const int waveW = 140;
        waveForm.setBounds(row1.removeFromLeft(waveW));
        decibelSlider.setBounds(row1);

        cutoffLowSlider.setBounds(takeRow(left));
        cutoffHighSlider.setBounds(takeRow(left));
    }

    // ADSR stack
    {
        attackSlider.setBounds(takeRow(right));
        decaySlider.setBounds(takeRow(right));
        sustainSlider.setBounds(takeRow(right));
        releaseSlider.setBounds(takeRow(right));
    }

    auto tremoloArea = juce::Rectangle<int>(
        left.getX(),
        right.getBottom() + 10,
        getWidth(),
        rowH
    ).reduced(5, 0);

    //tremolo
    {
        tremoloLabel.setBounds(tremoloArea.removeFromLeft(60));
        tremoloButton.setBounds(tremoloArea.removeFromLeft(30));
        tremoloWaveForm.setBounds(tremoloArea.removeFromLeft(waveForm.getWidth()));
        tremoloArea.removeFromLeft(5);
        tremoloFreqSlider.setBounds(tremoloArea.removeFromLeft(tremoloArea.getWidth() / 2));
        tremoloArea.removeFromLeft(5);
		tremoloDepthSlider.setBounds(takeRow(tremoloArea));
    }

    auto vibratoArea = juce::Rectangle<int>(
        tremoloLabel.getX(),
        right.getBottom() + rowH + 20,
        getWidth(),
        rowH*1.5
    ).reduced(5, 0);

    {
        promptBox.setBounds(vibratoArea.removeFromLeft(470));
        vibratoArea.removeFromLeft(10);
        generateButton.setBounds(vibratoArea.removeFromLeft(80));
    }
}

//==============================================================================
void PluginEditor::timerCallback()
{
    
}

bool PluginEditor::getParamsObject(const juce::var& root, juce::DynamicObject*& outParamsObj)
{
    outParamsObj = nullptr;

    if (!root.isObject())
        return false;

    // Prefer { params: { ... } }
    if (auto* rootObj = root.getDynamicObject())
    {
        auto paramsVar = rootObj->getProperty("params");
        if (paramsVar.isObject())
        {
            outParamsObj = paramsVar.getDynamicObject();
            return outParamsObj != nullptr;
        }

        // Fallback: treat root itself as params
        outParamsObj = rootObj;
        return true;
    }

    return false;
}

void PluginEditor::setParamFromFloat(const juce::String& paramId, float value)
{
    auto* p = processor.apvts.getParameter(paramId);
    auto* rp = dynamic_cast<juce::RangedAudioParameter*>(p);
    if (rp == nullptr)
        return;

    const auto range = rp->getNormalisableRange();

    // Clamp in "real units"
    const float clamped = juce::jlimit(range.start, range.end, value);
    const float norm = range.convertTo0to1(clamped);

    rp->beginChangeGesture();
    rp->setValueNotifyingHost(norm);
    rp->endChangeGesture();
}

void PluginEditor::setChoiceParamFromComboIndex(const juce::String& paramId, int comboIndex1toN)
{
    // ComboBoxes use 1..4 for items.

    const int zeroBased = juce::jlimit(0, 3, comboIndex1toN - 1);
    setParamFromFloat(paramId, (float)zeroBased);
}

void PluginEditor::applyPreset(const juce::var& result)
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

    // Unwrap the job payload if present
    if (auto* rootObj = result.getDynamicObject())
    {
        if (rootObj->hasProperty("ok"))
        {
            const bool ok = (bool)rootObj->getProperty("ok");
            if (!ok)
            {
                DBG("Preset generation failed: " + rootObj->getProperty("error").toString());
                return;
            }

            // Replace result with the actual preset JSON
            const juce::var data = rootObj->getProperty("data");
            applyPreset(data);
            return;
        }
    }

    // `result` should be either {params:{...}} or {...}
    juce::DynamicObject* paramsObj = nullptr;
    if (!getParamsObject(result, paramsObj) || paramsObj == nullptr)
    {
        DBG("applyPreset: JSON result does not contain params object.");
        return;
    }

    for (const auto& prop : paramsObj->getProperties())
    {
        const juce::String id = prop.name.toString();
        const juce::var v = prop.value;

        if (id == "wave" || id == "tremoloWave")
        {
            int idx = v.isInt() ? (int)v
                : (int)v.toString().getIntValue();

            if (idx >= 1 && idx <= 4)      setChoiceParamFromComboIndex(id, idx);
            else if (idx >= 0 && idx <= 3) setParamFromFloat(id, (float)idx);
            continue;
        }

        float value = v.isDouble() || v.isInt()
            ? (float)v
            : (float)v.toString().getDoubleValue();

        if (id == "tremoloOn" && v.isBool())
            value = v ? 1.0f : 0.0f;

        setParamFromFloat(id, value);
    }

    DBG("applyPreset: applied.");
}


