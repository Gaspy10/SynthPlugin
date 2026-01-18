#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class PluginEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit PluginEditor (JuceSynthPluginAudioProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Reference to the processor (owned by host)
    JuceSynthPluginAudioProcessor& processor;

    // ================= GUI COMPONENTS =================
    juce::MidiKeyboardComponent keyboardComponent;

    juce::ComboBox waveForm;
    juce::Slider decibelSlider;
    juce::Slider cutoffLowSlider;
    juce::Slider cutoffHighSlider;

    juce::Label decibelLabel;
    juce::Slider attackSlider;
	juce::Slider decaySlider;
	juce::Slider sustainSlider;
	juce::Slider releaseSlider;

    // GUI-only analyser (no audio thread access!)
    //AnalyserComponent analyserComponent;

    // ================= PARAMETER ATTACHMENTS =================
    using SliderAttachment  = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> gainAttachment;
    std::unique_ptr<SliderAttachment> cutoffLowAttachment;
    std::unique_ptr<SliderAttachment> cutoffHighAttachment;
    std::unique_ptr<ComboBoxAttachment> waveAttachment;

    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
