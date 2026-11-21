#pragma once

#include <JuceHeader.h>
#include <array>
#include "PluginProcessor.h"

class MorpheusLabAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                               private juce::Timer {
public:
    explicit MorpheusLabAudioProcessorEditor(MorpheusLabAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int slotCount = 6;
    void chooseFileForSlot(int slot);
    void refreshFileStatus();
    void refreshPresetList();
    void refreshDisplay();
    void timerCallback() override;
    void selectRelativePreset(int delta);

    MorpheusLabAudioProcessor& processor;
    std::unique_ptr<juce::Drawable> frontPanel;
    juce::Label title, subtitle, footer, presetLabel, lcdNumber, lcdName, lcdHint;
    juce::ComboBox presetSelector;
    std::array<juce::Slider, 8> editControls;
    std::array<juce::Label, 8> editLabels;
    std::array<juce::ToggleButton, 4> effectToggles;
    juce::TextButton previousButton { "◀ PREV" }, nextButton { "NEXT ▶" };
    juce::OwnedArray<juce::Label> slotNames, slotDescriptions, slotPaths, slotStatuses;
    juce::OwnedArray<juce::TextButton> slotButtons;
    juce::Label editSectionLabel, effectsSectionLabel, editNotice;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorpheusLabAudioProcessorEditor)
};
