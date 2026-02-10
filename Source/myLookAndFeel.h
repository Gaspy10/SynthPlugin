/*
  ==============================================================================

    Created: 14 Dec 2017 10:16:04am
    Author:  Stefan Remberg

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
using namespace::juce;

//==============================================================================
class myLookAndFeelV1 : public LookAndFeel_V4
{
public:
    myLookAndFeelV1();

    void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
        float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override;

private:
    Image img1;

};

//==============================================================================
class myLookAndFeelV2 : public juce::LookAndFeel_V4
{

public:
    myLookAndFeelV2();

    void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
        float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override;

};

//==============================================================================
class myLookAndFeelV3 : public juce::LookAndFeel_V4
{

public:
    myLookAndFeelV3();

    void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
        float rotaryStartAngle, float rotaryEndAngle, Slider& slider) override;

private:
    Image img2;

};
