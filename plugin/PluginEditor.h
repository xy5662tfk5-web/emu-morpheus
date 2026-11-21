#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MorpheusLabAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                               private juce::Timer {
public:
    explicit MorpheusLabAudioProcessorEditor(MorpheusLabAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void chooseFileForSlot(int slot);
    void refreshPresetList();
    void refreshDisplay();
    void timerCallback() override;
    void selectRelativePreset(int delta);

    MorpheusLabAudioProcessor& processor;
    std::unique_ptr<juce::Drawable> frontPanel;

    juce::ComboBox presetSelector;
    juce::TextButton masterButton, cursorButton, midiMapButton, hyperButton,
                     compareButton, dataCardButton, dataEncoderButton;
    juce::Label lcdLine1, lcdLine2;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorpheusLabAudioProcessorEditor)
};
