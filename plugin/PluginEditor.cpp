#include "PluginEditor.h"

MorpheusLabAudioProcessorEditor::MorpheusLabAudioProcessorEditor(
    MorpheusLabAudioProcessor& processor)
    : juce::AudioProcessorEditor(processor) {
    title.setText("E-MU MORPHEUS LAB", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setFont(juce::Font(juce::FontOptions(22.0f, juce::Font::bold)));
    addAndMakeVisible(title);

    status.setText("AU shell is running\nMIDI input: enabled\n"
                   "Sound: temporary sine-wave test tone\n"
                   "Original Morpheus ROM / sound engine: not yet integrated",
                   juce::dontSendNotification);
    status.setJustificationType(juce::Justification::centred);
    status.setFont(juce::Font(juce::FontOptions(13.0f)));
    addAndMakeVisible(status);

    setSize(460, 220);
}

void MorpheusLabAudioProcessorEditor::paint(juce::Graphics& graphics) {
    graphics.fillAll(juce::Colour(0xff171b22));
    graphics.setColour(juce::Colour(0xff596779));
    graphics.drawRoundedRectangle(getLocalBounds().toFloat().reduced(8.0f), 10.0f, 1.0f);
}

void MorpheusLabAudioProcessorEditor::resized() {
    title.setBounds(16, 22, getWidth() - 32, 42);
    status.setBounds(20, 78, getWidth() - 40, 112);
}
