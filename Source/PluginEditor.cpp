#include "PluginEditor.h"

static void setUnitFormatting(juce::Slider& s,
    const juce::String& unit,
    int decimals,
    double scale = 1.0) // display = value * scale
{
    s.textFromValueFunction = [=](double v)
        {
            const double shown = v * scale;
            return juce::String(shown, decimals) + " " + unit;
        };

    // Allows typing "123 ms" / "123" etc.
    s.valueFromTextFunction = [=](const juce::String& text)
        {
            auto t = text.trim().upToFirstOccurrenceOf(" ", false, false); // strips " ms"
            const double shown = t.getDoubleValue();
            return shown / scale;
        };
}

PluginEditor::PluginEditor(JuceSynthPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
    processor(p),
    keyboardComponent(processor.keyboardState,
        juce::MidiKeyboardComponent::horizontalKeyboard),
    openAIClient(OpenAIClient::Config{
        /*apiKey*/ Secrets::getOpenAIKey(),
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

    keyboardComponent.setOctaveForMiddleC(5);

    addAndMakeVisible (waveForm);

    setSliderProperties(decibelSlider);
    addAndMakeVisible (decibelSlider);
    addAndMakeVisible (decibelLabel);
    decibelLabel.setJustificationType(juce::Justification::centredLeft);
	
    addAndMakeVisible (cutoffLowSlider);
    addAndMakeVisible (cutoffHighSlider);

    addAndMakeVisible (attackSlider);
    addAndMakeVisible (decaySlider);
    addAndMakeVisible (releaseSlider);
    addAndMakeVisible (sustainSlider);

    promptBox.setFont(juce::Font(20.0f));

    promptBox.setTextToShowWhenEmpty(
        "Describe the sound you want...",
        juce::Colours::grey
    );


    auto initTopLabel = [&](juce::Label& lbl, const juce::String& text)
        {
            lbl.setText(text, juce::dontSendNotification);
            lbl.setJustificationType(juce::Justification::centred);
            lbl.setColour(juce::Label::textColourId, juce::Colours::white);
            lbl.setFont(juce::Font(13.0f));
            addAndMakeVisible(lbl);
        };

    initTopLabel(attackLabel, "Attack");
    initTopLabel(decayLabel, "Decay");
    initTopLabel(sustainLabel, "Sustain");
    initTopLabel(releaseLabel, "Release");
    initTopLabel(decibelLabel, "Gain:");

    initTopLabel(lowPassLabel, "Low-pass filter:");
    initTopLabel(highPassLabel, "High-pass filter:");

    initTopLabel(tremoloFreqLabel, "Tremolo freq:");
    initTopLabel(tremoloDepthLabel, "Tremolo depth:");

    setSliderProperties(attackSlider);
    setSliderProperties(decaySlider);
    setSliderProperties(releaseSlider);
    setSliderProperties(sustainSlider);

    addAndMakeVisible(tremoloLabel);
	addAndMakeVisible (tremoloButton);
    addAndMakeVisible(tremoloWaveForm);
    addAndMakeVisible(tremoloFreqSlider);
	addAndMakeVisible(tremoloDepthSlider);

    addAndMakeVisible(promptBox);
    addAndMakeVisible(generateButton);

    // Label
	tremoloLabel.setText("Tremolo: ", juce::dontSendNotification);

    // Sliders
    decibelSlider.setRange (-24.0, 24.0, 0.1);
    decibelSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);

    cutoffLowSlider.setRange (20.0, 20000.0, 1);
    cutoffHighSlider.setRange (20.0, 20000.0, 1);

	attackSlider.setRange(0, 5000.0, 1);
	sustainSlider.setRange(0.1, 1.0, 0.01);
	decaySlider.setRange(0, 5000.0, 1);
	releaseSlider.setRange(0, 5000.0, 1);

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

    // Gain: dB
    decibelSlider.setTextValueSuffix(" dB");

    attackSlider.setTextValueSuffix(" ms");
    decaySlider.setTextValueSuffix(" ms");
    releaseSlider.setTextValueSuffix(" ms");

    // Filters
    cutoffLowSlider.setTextValueSuffix(" Hz");
    cutoffHighSlider.setTextValueSuffix(" Hz");

    // Tremolo
    tremoloFreqSlider.setTextValueSuffix(" Hz");

    tremoloDepthSlider.setTextValueSuffix(" %");

    // Parameter attachments
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

    setSize (650, 415);

    startTimerHz (30); 
}

void PluginEditor::setSliderProperties(juce::Slider& s) {
    s.setLookAndFeel(&myLookAndFeelV1);
    s.setSliderStyle(Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(Slider::TextBoxBelow, false, 60, 20);
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
    const int rowH = 40;
    const int gap = 8;

    const int leftRowH = 44;

    right.setHeight(3 * rowH + 2 * gap); // there are the knobs so nonsense
    left.setHeight(3 * leftRowH + 2 * gap); 

    auto takeRow = [&](juce::Rectangle<int>& r)
    {
        auto row = r.removeFromTop(rowH);
        r.removeFromTop(gap);
        return row;
    };

    auto leftTakeRow = [&](juce::Rectangle<int>& r)
    {
        auto row = r.removeFromTop(leftRowH);
        r.removeFromTop(gap);
        return row;
    };

    // Waveform + Gain + Cutoffs
    {
        const int labelW = 100;

        // Row 1: Waveform + Gain
        auto row1 = leftTakeRow(left);
        auto waveFormArea = row1.removeFromLeft(100);

        waveForm.setBounds(
            waveFormArea.withSizeKeepingCentre(waveFormArea.getWidth(), 33)
        );

        {
            auto labelArea = row1.removeFromLeft(50);
            auto sliderArea = row1;

            decibelLabel.setBounds(labelArea);
            decibelSlider.setBounds(sliderArea);
        }

        // Row 2: High-pass
        auto row2 = leftTakeRow(left);
        {
            auto labelArea = row2.removeFromLeft(labelW);
            auto sliderArea = row2;

            highPassLabel.setBounds(labelArea);
            cutoffLowSlider.setBounds(sliderArea);
        }

        // Row 3: Low-pass
        auto row3 = leftTakeRow(left);
        {
            auto labelArea = row3.removeFromLeft(labelW);
            auto sliderArea = row3;

            lowPassLabel.setBounds(labelArea);
            cutoffHighSlider.setBounds(sliderArea);
        }
    }


    // ADSR stack
    {
        const int knobW = 75;
        const int labelH = 16;

        auto placeKnobWithLabel = [&](juce::Label& lbl, juce::Slider& s, juce::Rectangle<int>& r)
        {
            auto cell = r.removeFromLeft(knobW);
            lbl.setBounds(cell.removeFromTop(labelH));
            s.setBounds(cell); // remaining area
        };

        placeKnobWithLabel(attackLabel, attackSlider, right);
        placeKnobWithLabel(decayLabel, decaySlider, right);
        placeKnobWithLabel(sustainLabel, sustainSlider, right);
        placeKnobWithLabel(releaseLabel, releaseSlider, right);
    }

    auto tremoloArea = juce::Rectangle<int>(
        left.getX(),
        left.getBottom() + 5,
        getWidth(),
        rowH
    ).reduced(5, 0);

    //tremolo
    {
        const int labelW = 80;

        tremoloLabel.setBounds(tremoloArea.removeFromLeft(60));
        tremoloButton.setBounds(tremoloArea.removeFromLeft(30));

        auto tremoloWaveFormArea = tremoloArea.removeFromLeft(waveForm.getWidth());
        tremoloWaveForm.setBounds(
            tremoloWaveFormArea.withSizeKeepingCentre(tremoloWaveFormArea.getWidth(), 33)
        );
        tremoloArea.removeFromLeft(10); // gap

        // ---- Tremolo frequency ----
        {
            auto freqArea = tremoloArea.removeFromLeft(tremoloArea.getWidth() / 2);
            tremoloFreqLabel.setBounds(freqArea.removeFromLeft(labelW));
            tremoloFreqSlider.setBounds(freqArea);
        }

        tremoloArea.removeFromLeft(10); // gap

        // ---- Tremolo depth ----
        {
            auto depthArea = tremoloArea;
            tremoloDepthLabel.setBounds(depthArea.removeFromLeft(labelW));
            tremoloDepthSlider.setBounds(depthArea);
        }
    }


    auto vibratoArea = juce::Rectangle<int>(
        tremoloLabel.getX(),
        left.getBottom() + rowH + 15,
        getWidth(),
        rowH*1.5
    ).reduced(5, 0);

    {
        promptBox.setBounds(vibratoArea.removeFromLeft(525));
        vibratoArea.removeFromLeft(10);
        generateButton.setBounds(vibratoArea.removeFromLeft(80));
    }
}

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


