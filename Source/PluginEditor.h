#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PresetGenerationJob.h"
#include "OpenAIClient.h"
#include "myLookAndFeel.h"
#include "Secrets.h"

class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit PluginEditor (JuceSynthPluginAudioProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void setSliderProperties(juce::Slider&);

private:
    juce::ThreadPool threadPool{ 1 };
    OpenAIClient openAIClient;

    void applyPreset(const juce::var& result);

    void generatePreset();
    static bool getParamsObject(const juce::var& root, juce::DynamicObject*& outParamsObj);
    void setParamFromFloat(const juce::String& paramId, float value);
    void setChoiceParamFromComboIndex(const juce::String& paramId, int comboIndex1toN);

    // Reference to the processor (owned by host)
    JuceSynthPluginAudioProcessor& processor;

    // gui components
    juce::MidiKeyboardComponent keyboardComponent;

    juce::ComboBox waveForm;
	juce::ComboBox tremoloWaveForm;
    juce::Slider decibelSlider;
    juce::Slider cutoffLowSlider;
    juce::Slider cutoffHighSlider;
    
    juce::Slider attackSlider;
	juce::Slider decaySlider;
	juce::Slider sustainSlider;
	juce::Slider releaseSlider;

    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel, decibelLabel, lowPassLabel, highPassLabel, tremoloFreqLabel, tremoloDepthLabel;

    juce::Label tremoloLabel;
    juce::ToggleButton tremoloButton;
	juce::Slider tremoloFreqSlider;
	juce::Slider tremoloDepthSlider;

    juce::TextEditor promptBox;
    juce::TextButton generateButton{ "Generate preset" };

    myLookAndFeelV1 myLookAndFeelV1;

    // GUI-only analyser (no audio thread access!)
    //AnalyserComponent analyserComponent;

    // parameter attachments
    using SliderAttachment  = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
	using ToggleButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<SliderAttachment> cutoffLowAttachment;
    std::unique_ptr<SliderAttachment> cutoffHighAttachment;
    std::unique_ptr<ComboBoxAttachment> waveAttachment;

    std::unique_ptr<ComboBoxAttachment> tremoloWaveAttachment;
	std::unique_ptr<ToggleButtonAttachment> tremoloButtonAttachment;
	std::unique_ptr<SliderAttachment> tremoloFreqAttachment;
	std::unique_ptr<SliderAttachment> tremoloDepthAttachment;

    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
