#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MorpheusLabAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    explicit MorpheusLabAudioProcessorEditor(MorpheusLabAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Label title;
    juce::Label status;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorpheusLabAudioProcessorEditor)
};
