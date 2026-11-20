#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MorpheusLabAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    explicit MorpheusLabAudioProcessorEditor(MorpheusLabAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int slotCount = 6;
    void chooseFileForSlot(int slot);
    void refreshFileStatus();
    void refreshPresetList();

    MorpheusLabAudioProcessor& processor;
    juce::Label title, subtitle, footer, presetLabel;
    juce::ComboBox presetSelector;
    juce::OwnedArray<juce::Label> slotNames, slotDescriptions, slotPaths, slotStatuses;
    juce::OwnedArray<juce::TextButton> slotButtons;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorpheusLabAudioProcessorEditor)
};
