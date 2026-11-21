#include "PluginEditor.h"
#include <BinaryData.h>

namespace {
const juce::Colour lcdInk { 0xff182406 };
const juce::Colour lcdFill { 0xff6d902b };
const juce::Colour transparent { 0x00000000 };
}

MorpheusLabAudioProcessorEditor::MorpheusLabAudioProcessorEditor(
    MorpheusLabAudioProcessor& p)
    : juce::AudioProcessorEditor(p), processor(p) {
    auto svgXml = juce::XmlDocument::parse(
        juce::String::fromUTF8(BinaryData::morpheus_front_panel_svg,
                               BinaryData::morpheus_front_panel_svgSize));
    if (svgXml != nullptr)
        frontPanel = juce::Drawable::createFromSVG(*svgXml);

    // Invisible selector sits over the physical PRESET key; its popup is real.
    presetSelector.setAlpha(0.0f);
    presetSelector.setTextWhenNothingSelected({});
    presetSelector.onChange = [this] {
        const int selectedId = presetSelector.getSelectedId();
        if (selectedId > 0)
            processor.setCurrentProgram(selectedId - 1);
        refreshDisplay();
    };
    addAndMakeVisible(presetSelector);

    auto makeInvisibleKey = [this](juce::TextButton& button,
                                   std::function<void()> action) {
        button.setButtonText({});
        button.setColour(juce::TextButton::buttonColourId, transparent);
        button.setColour(juce::TextButton::buttonOnColourId, transparent);
        button.setColour(juce::TextButton::textColourOffId, transparent);
        button.setColour(juce::TextButton::textColourOnId, transparent);
        button.onClick = std::move(action);
        addAndMakeVisible(button);
    };

    makeInvisibleKey(masterButton, [this] { chooseFileForSlot(0); });
    makeInvisibleKey(cursorButton, [this] { selectRelativePreset(-1); });
    makeInvisibleKey(midiMapButton, [this] { chooseFileForSlot(1); });
    makeInvisibleKey(hyperButton, [this] { chooseFileForSlot(2); });
    makeInvisibleKey(compareButton, [this] { selectRelativePreset(1); });
    makeInvisibleKey(dataCardButton, [this] { chooseFileForSlot(5); });
    makeInvisibleKey(dataEncoderButton, [this] { selectRelativePreset(1); });

    lcdLine1.setColour(juce::Label::textColourId, lcdInk);
    lcdLine1.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
    lcdLine1.setJustificationType(juce::Justification::centredLeft);
    lcdLine1.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(lcdLine1);

    lcdLine2.setText("Ch:01 Vol:127", juce::dontSendNotification);
    lcdLine2.setColour(juce::Label::textColourId, lcdInk);
    lcdLine2.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
    lcdLine2.setJustificationType(juce::Justification::centredLeft);
    lcdLine2.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(lcdLine2);

    refreshPresetList();
    refreshDisplay();
    startTimerHz(10);
    setResizable(false, false);
    setSize(1200, 112);
}

void MorpheusLabAudioProcessorEditor::chooseFileForSlot(int slot) {
    const auto titleText = slot == 0 ? "Choose CPU firmware"
        : slot == 5 ? "Choose Morpheus SysEx preset bank"
                    : "Choose sound ROM image";
    const auto pattern = slot == 5 ? "*.syx;*.mid" : "*.bin;*.rom;*.img;*";

    fileChooser = std::make_unique<juce::FileChooser>(
        titleText, juce::File(), pattern);

    auto safeThis = juce::Component::SafePointer<MorpheusLabAudioProcessorEditor>(this);
    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [safeThis, slot](const juce::FileChooser& chooser) {
            if (safeThis == nullptr)
                return;
            const auto selected = chooser.getResult();
            if (selected.existsAsFile()) {
                safeThis->processor.setRomPath(slot, selected.getFullPathName());
                if (slot == 5)
                    safeThis->refreshPresetList();
            }
        });
}

void MorpheusLabAudioProcessorEditor::refreshPresetList() {
    presetSelector.clear(juce::dontSendNotification);
    const int count = processor.getPresetCount();
    for (int i = 0; i < count; ++i)
        presetSelector.addItem(processor.getProgramName(i), i + 1);

    if (count > 0)
        presetSelector.setSelectedId(processor.getCurrentProgram() + 1,
                                     juce::dontSendNotification);
    refreshDisplay();
}

void MorpheusLabAudioProcessorEditor::refreshDisplay() {
    const int count = processor.getPresetCount();
    const int index = processor.getCurrentProgram();
    if (count > 0) {
        const auto number = juce::String(processor.getProgramNumber(index))
                                .paddedLeft('0', 3);
        const auto name = processor.getProgramName(index).substring(0, 11);
        lcdLine1.setText("P" + number + " " + name, juce::dontSendNotification);
        presetSelector.setSelectedId(index + 1, juce::dontSendNotification);
    } else {
        lcdLine1.setText("P--- No Preset", juce::dontSendNotification);
        presetSelector.setSelectedId(0, juce::dontSendNotification);
    }
}

void MorpheusLabAudioProcessorEditor::selectRelativePreset(int delta) {
    const int count = processor.getPresetCount();
    if (count <= 0)
        return;
    const int next = (processor.getCurrentProgram() + delta + count) % count;
    processor.setCurrentProgram(next);
    refreshDisplay();
}

void MorpheusLabAudioProcessorEditor::timerCallback() {
    refreshDisplay();
}

void MorpheusLabAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff111214));
    if (frontPanel != nullptr)
        frontPanel->drawWithin(g, getLocalBounds().toFloat(),
                               juce::RectanglePlacement::stretchToFit, 1.0f);

    // Dynamic text replaces the sample text in the SVG's LCD glass.
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff769c31), 0.0f, 36.0f,
        juce::Colour(0xff5d7d23), 0.0f, 76.0f, false));
    g.fillRect(433, 36, 204, 40);
}

void MorpheusLabAudioProcessorEditor::resized() {
    const float sx = static_cast<float>(getWidth()) / 1200.0f;
    const float sy = static_cast<float>(getHeight()) / 112.0f;
    auto place = [sx, sy](juce::Component& c, float x, float y, float w, float h) {
        c.setBounds(juce::roundToInt(x * sx), juce::roundToInt(y * sy),
                    juce::roundToInt(w * sx), juce::roundToInt(h * sy));
    };

    // Invisible hit targets align with the physical controls in the SVG.
    place(masterButton,       695, 35, 40, 16);
    place(presetSelector,     755, 35, 40, 16);
    place(cursorButton,       815, 35, 40, 16);
    place(midiMapButton,      695, 75, 40, 16);
    place(hyperButton,        755, 75, 40, 16);
    place(compareButton,      815, 75, 40, 16);
    place(dataCardButton,     208, 66, 160, 10);
    place(dataEncoderButton,  887, 28, 56, 56);

    // Two dynamic rows in the 2 x 16 LCD.
    place(lcdLine1, 439, 32, 192, 20);
    place(lcdLine2, 439, 49, 192, 20);
}
