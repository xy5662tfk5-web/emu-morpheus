#include "PluginEditor.h"
#include <BinaryData.h>

namespace {
const juce::Colour backgroundColour { 0xff11161d };
const juce::Colour panelColour      { 0xff202833 };
const juce::Colour panelEdge        { 0xff526174 };
const juce::Colour lcdColour        { 0xffa8b99a };
const juce::Colour lcdInk           { 0xff17251b };
const juce::Colour textColour       { 0xffe7edf5 };
const juce::Colour mutedColour      { 0xff9aa9bc };
const juce::Colour accentColour     { 0xffd6a65f };
}

MorpheusLabAudioProcessorEditor::MorpheusLabAudioProcessorEditor(
    MorpheusLabAudioProcessor& p)
    : juce::AudioProcessorEditor(p), processor(p) {
    auto svgXml = juce::XmlDocument::parse(
        juce::String::fromUTF8(BinaryData::morpheus_front_panel_svg,
                               BinaryData::morpheus_front_panel_svgSize));
    if (svgXml != nullptr)
        frontPanel = juce::Drawable::createFromSVG(*svgXml);

    presetLabel.setText("PRESET BANK", juce::dontSendNotification);
    presetLabel.setColour(juce::Label::textColourId, textColour);
    presetLabel.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
    addAndMakeVisible(presetLabel);

    presetSelector.setTextWhenNothingSelected("Choose a preset bank (.syx)");
    presetSelector.onChange = [this] {
        const int selectedId = presetSelector.getSelectedId();
        if (selectedId > 0)
            processor.setCurrentProgram(selectedId - 1);
        refreshDisplay();
        footer.setText(processor.getPresetBankStatus(), juce::dontSendNotification);
    };
    addAndMakeVisible(presetSelector);

    for (auto* button : { &previousButton, &nextButton }) {
        button->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff354252));
        button->setColour(juce::TextButton::textColourOffId, textColour);
        addAndMakeVisible(*button);
    }
    previousButton.onClick = [this] { selectRelativePreset(-1); };
    nextButton.onClick = [this] { selectRelativePreset(1); };

    lcdNumber.setColour(juce::Label::textColourId, lcdInk);
    lcdNumber.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
    lcdNumber.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(lcdNumber);

    lcdName.setColour(juce::Label::textColourId, lcdInk);
    lcdName.setFont(juce::Font(juce::FontOptions(25.0f, juce::Font::bold)));
    lcdName.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(lcdName);

    lcdHint.setText("MORPHEUS PRESET DISPLAY", juce::dontSendNotification);
    lcdHint.setColour(juce::Label::textColourId, juce::Colour(0xff435342));
    lcdHint.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    addAndMakeVisible(lcdHint);

    editSectionLabel.setText("Z-PLANE  /  SOUND EDIT", juce::dontSendNotification);
    editSectionLabel.setColour(juce::Label::textColourId, textColour);
    editSectionLabel.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
    addAndMakeVisible(editSectionLabel);

    editNotice.setText("CONTROL LAYOUT PREVIEW — PARAMETER MAP / DSP NOT CONNECTED",
                       juce::dontSendNotification);
    editNotice.setColour(juce::Label::textColourId, juce::Colour(0xffffc477));
    editNotice.setFont(juce::Font(juce::FontOptions(9.0f)));
    addAndMakeVisible(editNotice);

    // These are interface placeholders. They are not yet mapped to verified
    // Morpheus SysEx parameters or a Z-Plane DSP implementation.
    const std::array<juce::String, 8> controlNames {
        "MORPH POS", "MORPH MOD", "MORPH RATE", "MORPH DEPTH",
        "AMP ATTACK", "AMP RELEASE", "LFO RATE", "OUTPUT LEVEL"
    };
    const std::array<double, 8> controlDefaults { 0.5, 0.0, 1.0, 0.0, 0.02, 0.35, 2.0, 0.8 };
    const std::array<double, 8> controlMins { 0.0, 0.0, 0.01, 0.0, 0.0, 0.0, 0.01, 0.0 };
    const std::array<double, 8> controlMaxs { 1.0, 1.0, 20.0, 1.0, 10.0, 10.0, 20.0, 1.0 };

    for (size_t i = 0; i < editControls.size(); ++i) {
        auto& slider = editControls[i];
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
        slider.setRange(controlMins[i], controlMaxs[i], 0.001);
        slider.setValue(controlDefaults[i], juce::dontSendNotification);
        slider.setColour(juce::Slider::rotarySliderFillColourId, accentColour);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff596779));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff0d3a0));
        slider.setColour(juce::Slider::textBoxTextColourId, textColour);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff171b22));
        addAndMakeVisible(slider);

        auto& label = editLabels[i];
        label.setText(controlNames[i], juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colour(0xffcbd5e1));
        label.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
        addAndMakeVisible(label);
    }

    effectsSectionLabel.setText("POST-FILTER EFFECTS", juce::dontSendNotification);
    effectsSectionLabel.setColour(juce::Label::textColourId, textColour);
    effectsSectionLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    addAndMakeVisible(effectsSectionLabel);

    const std::array<juce::String, 4> effectNames { "REVERB", "CHORUS", "DELAY", "DISTORTION" };
    for (size_t i = 0; i < effectToggles.size(); ++i) {
        effectToggles[i].setButtonText(effectNames[i]);
        effectToggles[i].setColour(juce::ToggleButton::textColourId, textColour);
        effectToggles[i].setColour(juce::ToggleButton::tickColourId, accentColour);
        addAndMakeVisible(effectToggles[i]);
    }

    const juce::StringArray names {
        "CPU Firmware", "Sound ROM 1", "Sound ROM 2", "Sound ROM 3",
        "Sound ROM 4", "Preset Bank (.syx)"
    };
    const juce::StringArray descriptions {
        "Firmware image — file selection only", "Sound memory image / socket 1",
        "Sound memory image / socket 2", "Sound memory image / socket 3",
        "Sound memory image / socket 4", "Morpheus SysEx preset bank"
    };

    for (int i = 0; i < slotCount; ++i) {
        auto* name = new juce::Label();
        name->setText(names[i], juce::dontSendNotification);
        name->setColour(juce::Label::textColourId, textColour);
        name->setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
        addAndMakeVisible(name);
        slotNames.add(name);

        auto* info = new juce::Label();
        info->setText(descriptions[i], juce::dontSendNotification);
        info->setColour(juce::Label::textColourId, mutedColour);
        info->setFont(juce::Font(juce::FontOptions(9.0f)));
        addAndMakeVisible(info);
        slotDescriptions.add(info);

        auto* path = new juce::Label();
        path->setText("No file selected", juce::dontSendNotification);
        path->setColour(juce::Label::textColourId, textColour);
        path->setFont(juce::Font(juce::FontOptions(10.5f)));
        path->setMinimumHorizontalScale(0.7f);
        addAndMakeVisible(path);
        slotPaths.add(path);

        auto* status = new juce::Label();
        status->setJustificationType(juce::Justification::centredRight);
        status->setFont(juce::Font(juce::FontOptions(9.5f)));
        addAndMakeVisible(status);
        slotStatuses.add(status);

        auto* button = new juce::TextButton("BROWSE");
        button->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff354252));
        button->setColour(juce::TextButton::textColourOffId, textColour);
        button->onClick = [this, i] { chooseFileForSlot(i); };
        addAndMakeVisible(button);
        slotButtons.add(button);
    }

    footer.setText(processor.getPresetBankStatus(), juce::dontSendNotification);
    footer.setColour(juce::Label::textColourId, juce::Colour(0xffffc477));
    footer.setFont(juce::Font(juce::FontOptions(10.0f)));
    addAndMakeVisible(footer);

    refreshFileStatus();
    refreshPresetList();
    refreshDisplay();
    startTimerHz(10);
    setSize(960, 1020);
}

void MorpheusLabAudioProcessorEditor::chooseFileForSlot(int slot) {
    const auto titleText = slot == 0 ? "Choose CPU firmware"
        : slot == 5 ? "Choose Morpheus SysEx preset bank"
                    : "Choose sound ROM image";

    fileChooser = std::make_unique<juce::FileChooser>(
        titleText, juce::File(), slot == 5 ? "*.syx;*.mid" : "*.bin;*.rom;*.img;*");

    auto safeThis = juce::Component::SafePointer<MorpheusLabAudioProcessorEditor>(this);
    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [safeThis, slot](const juce::FileChooser& chooser) {
            if (safeThis == nullptr)
                return;
            const auto selected = chooser.getResult();
            if (selected.existsAsFile()) {
                safeThis->processor.setRomPath(slot, selected.getFullPathName());
                safeThis->refreshFileStatus();
                if (slot == 5)
                    safeThis->refreshPresetList();
            }
        });
}

void MorpheusLabAudioProcessorEditor::refreshFileStatus() {
    for (int i = 0; i < slotCount; ++i) {
        const auto path = processor.getRomPath(i);
        if (path.isEmpty()) {
            slotPaths[i]->setText("No file selected", juce::dontSendNotification);
            slotStatuses[i]->setText("EMPTY", juce::dontSendNotification);
            slotStatuses[i]->setColour(juce::Label::textColourId, mutedColour);
            continue;
        }

        const juce::File file(path);
        slotPaths[i]->setText(file.getFileName(), juce::dontSendNotification);
        if (file.existsAsFile()) {
            const auto bytes = file.getSize();
            const auto sizeText = bytes >= 1024 * 1024
                ? juce::String(static_cast<double>(bytes) / (1024.0 * 1024.0), 2) + " MB"
                : juce::String(static_cast<double>(bytes) / 1024.0, 1) + " KB";
            slotStatuses[i]->setText("FOUND  |  " + sizeText, juce::dontSendNotification);
            slotStatuses[i]->setColour(juce::Label::textColourId, juce::Colour(0xff8ed6a5));
        } else {
            slotStatuses[i]->setText("FILE MISSING", juce::dontSendNotification);
            slotStatuses[i]->setColour(juce::Label::textColourId, juce::Colour(0xffff8a80));
        }
    }
}

void MorpheusLabAudioProcessorEditor::refreshPresetList() {
    presetSelector.clear(juce::dontSendNotification);
    const int count = processor.getPresetCount();
    for (int i = 0; i < count; ++i)
        presetSelector.addItem(processor.getProgramName(i), i + 1);

    if (count > 0) {
        presetSelector.setTextWhenNothingSelected(juce::String());
        presetSelector.setSelectedId(processor.getCurrentProgram() + 1,
                                     juce::dontSendNotification);
    } else {
        presetSelector.setTextWhenNothingSelected("No valid presets loaded");
    }
    footer.setText(processor.getPresetBankStatus(), juce::dontSendNotification);
    refreshDisplay();
}

void MorpheusLabAudioProcessorEditor::refreshDisplay() {
    const int count = processor.getPresetCount();
    const int index = processor.getCurrentProgram();
    const bool hasPresets = count > 0;
    lcdNumber.setText(hasPresets
        ? "PRESET " + juce::String(processor.getProgramNumber(index)).paddedLeft('0', 3)
        : "PRESET ---", juce::dontSendNotification);
    lcdName.setText(hasPresets ? processor.getProgramName(index)
                               : "NO PRESET BANK LOADED",
                    juce::dontSendNotification);
    presetSelector.setSelectedId(hasPresets ? index + 1 : 0,
                                 juce::dontSendNotification);
    previousButton.setEnabled(hasPresets && index > 0);
    nextButton.setEnabled(hasPresets && index + 1 < count);
}

void MorpheusLabAudioProcessorEditor::selectRelativePreset(int delta) {
    const int count = processor.getPresetCount();
    if (count <= 0)
        return;
    processor.setCurrentProgram(processor.getCurrentProgram() + delta);
    refreshDisplay();
}

void MorpheusLabAudioProcessorEditor::timerCallback() {
    const int expectedId = processor.getCurrentProgram() + 1;
    if (presetSelector.getSelectedId() != expectedId)
        refreshDisplay();
}

void MorpheusLabAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(backgroundColour);

    if (frontPanel != nullptr)
        frontPanel->drawWithin(g, juce::Rectangle<float>(18.0f, 10.0f,
            static_cast<float>(getWidth() - 36), 86.0f),
            juce::RectanglePlacement::stretchToFit, 1.0f);

    const auto panel = [&g](float x, float y, float w, float h, bool filled = true) {
        const juce::Rectangle<float> r(x, y, w, h);
        if (filled)
            g.setColour(panelColour);
        else
            g.setColour(juce::Colours::transparentBlack);
        if (filled)
            g.fillRoundedRectangle(r, 8.0f);
        g.setColour(panelEdge);
        g.drawRoundedRectangle(r, 8.0f, 1.0f);
    };

    panel(18.0f, 110.0f, getWidth() - 36.0f, 52.0f);
    panel(18.0f, 174.0f, getWidth() - 36.0f, 118.0f);
    panel(18.0f, 304.0f, getWidth() - 36.0f, 274.0f);
    panel(18.0f, 590.0f, getWidth() - 36.0f, 84.0f);
    panel(18.0f, 686.0f, getWidth() - 36.0f, 306.0f);

    g.setColour(lcdColour);
    g.fillRoundedRectangle(juce::Rectangle<float>(30.0f, 185.0f,
        static_cast<float>(getWidth() - 60), 96.0f), 5.0f);
    g.setColour(juce::Colour(0xff65775f));
    g.drawRoundedRectangle(juce::Rectangle<float>(30.0f, 185.0f,
        static_cast<float>(getWidth() - 60), 96.0f), 5.0f, 1.0f);

    g.setColour(juce::Colour(0xff344253));
    g.drawHorizontalLine(720, 30.0f, static_cast<float>(getWidth() - 30));
    for (int i = 1; i < slotCount; ++i)
        g.drawHorizontalLine(728 + i * 42, 30.0f, static_cast<float>(getWidth() - 30));
}

void MorpheusLabAudioProcessorEditor::resized() {
    const int width = getWidth();

    presetLabel.setBounds(32, 123, 90, 24);
    presetSelector.setBounds(126, 120, width - 370, 30);
    previousButton.setBounds(width - 232, 120, 96, 30);
    nextButton.setBounds(width - 128, 120, 96, 30);

    lcdNumber.setBounds(50, 194, 200, 24);
    lcdName.setBounds(50, 222, width - 100, 40);
    lcdHint.setBounds(width - 252, 192, 200, 18);

    editSectionLabel.setBounds(32, 314, 250, 22);
    editNotice.setBounds(width - 500, 316, 460, 18);
    const int controlWidth = (width - 100) / 4;
    for (size_t i = 0; i < editControls.size(); ++i) {
        const int row = static_cast<int>(i / 4);
        const int column = static_cast<int>(i % 4);
        const int x = 30 + column * controlWidth;
        const int y = 346 + row * 112;
        editLabels[i].setBounds(x, y, controlWidth - 8, 16);
        editControls[i].setBounds(x + 12, y + 15, controlWidth - 32, 90);
    }

    effectsSectionLabel.setBounds(32, 600, 230, 24);
    for (size_t i = 0; i < effectToggles.size(); ++i)
        effectToggles[i].setBounds(275 + static_cast<int>(i) * 155, 612, 140, 28);

    for (int i = 0; i < slotCount; ++i) {
        const int y = 724 + i * 42;
        slotNames[i]->setBounds(32, y, 145, 18);
        slotDescriptions[i]->setBounds(32, y + 17, 185, 16);
        slotPaths[i]->setBounds(225, y + 4, width - 520, 24);
        slotStatuses[i]->setBounds(width - 282, y + 4, 150, 24);
        slotButtons[i]->setBounds(width - 120, y + 2, 88, 27);
    }
    footer.setBounds(28, 996, width - 56, 18);
}
