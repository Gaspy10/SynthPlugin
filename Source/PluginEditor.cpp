#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor (JuceSynthPluginAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      keyboardComponent (processor.keyboardState,
                         juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // ================= ADD COMPONENTS =================
    addAndMakeVisible (keyboardComponent);
    addAndMakeVisible (waveForm);
    addAndMakeVisible (decibelSlider);
    addAndMakeVisible (decibelLabel);
    //addAndMakeVisible (analyserComponent);
    addAndMakeVisible (cutoffLowSlider);
    addAndMakeVisible (cutoffHighSlider);
    addAndMakeVisible (attackSlider);
    addAndMakeVisible (decaySlider);
    addAndMakeVisible (releaseSlider);
    addAndMakeVisible (sustainSlider);

    // ================= LABEL =================
    decibelLabel.setText ("Noise Level in dB", juce::dontSendNotification);

    // ================= SLIDERS =================
    decibelSlider.setRange (-24.0, 24.0);
    decibelSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 100, 20);

    cutoffLowSlider.setRange (20.0, 20000.0);
    cutoffHighSlider.setRange (20.0, 20000.0);

	attackSlider.setRange(0, 5000.0);
	sustainSlider.setRange(0.0, 1.0);
	decaySlider.setRange(0, 5000.0);
	releaseSlider.setRange(0, 5000.0);

    // ================= WAVEFORM COMBO =================
    waveForm.addItem ("Sine", 1);
    waveForm.addItem ("Square", 2);
    waveForm.addItem ("Triangle", 3);
    waveForm.addItem ("Sawtooth", 4);

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

    // ================= EDITOR =================
    setSize (600, 300);

    startTimerHz (30); // GUI refresh only (e.g. analyser repaint)
}

//==============================================================================
PluginEditor::~PluginEditor() = default;

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

//==============================================================================
void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    // --- Top: keyboard ---
    const int keyboardH = 120;
    keyboardComponent.setBounds(area.removeFromTop(keyboardH));
    area.removeFromTop(10); // gap

    // --- Bottom: split into left/right with a gap ---
    const int colGap = 10;
    auto left = area.removeFromLeft(area.getWidth() / 2);
    area.removeFromLeft(colGap);
    auto right = area;

    // Helpers
    const int rowH = 32;
    const int gap = 8;

    auto takeRow = [&](juce::Rectangle<int>& r)
        {
            auto row = r.removeFromTop(rowH);
            r.removeFromTop(gap);
            return row;
        };

    // ----- LEFT: Waveform + Gain + Cutoffs -----
    {
        auto row1 = takeRow(left);
        const int waveW = 140;
        waveForm.setBounds(row1.removeFromLeft(waveW));
        decibelSlider.setBounds(row1);

        cutoffLowSlider.setBounds(takeRow(left));
        cutoffHighSlider.setBounds(takeRow(left));
    }

    // ----- RIGHT: ADSR stack -----
    {
        attackSlider.setBounds(takeRow(right));
        decaySlider.setBounds(takeRow(right));
        sustainSlider.setBounds(takeRow(right));
        releaseSlider.setBounds(takeRow(right));
    }
}

//==============================================================================
void PluginEditor::timerCallback()
{
    // GUI-only updates (no DSP!)
    //analyserComponent.repaint();
}
