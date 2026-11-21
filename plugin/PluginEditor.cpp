#include "PluginEditor.h"
#include <BinaryData.h>

namespace {
const juce::Colour panelColour  { 0xff202833 };
const juce::Colour lcdColour    { 0xffa8b99a };
const juce::Colour lcdInk       { 0xff17251b };
const juce::Colour accentColour { 0xffd6a65f };
}

MorpheusLabAudioProcessorEditor::MorpheusLabAudioProcessorEditor(
    MorpheusLabAudioProcessor& p)
    : juce::AudioProcessorEditor(p), processor(p) {
    auto svgXml = juce::XmlDocument::parse(
        juce::String::fromUTF8(BinaryData::morpheus_front_panel_svg,
                               BinaryData::morpheus_front_panel_svgSize));
    if (svgXml != nullptr)
        frontPanel = juce::Drawable::createFromSVG(*svgXml);

    title.setText("E-MU MORPHEUS LAB", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centredLeft);
    title.setFont(juce::Font(juce::FontOptions(22.0f, juce::Font::bold)));

    subtitle.setText("MORPHEUS PRESET CONTROL  |  ROM FILE MANAGER",
                     juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xff9aa9bc));
    subtitle.setFont(juce::Font(juce::FontOptions(11.0f)));

    presetLabel.setText("PRESET BANK", juce::dontSendNotification);
    presetLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd8e1ed));
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
        button->setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe7edf5));
        addAndMakeVisible(*button);
    }
    previousButton.onClick = [this] { selectRelativePreset(-1); };
    nextButton.onClick = [this] { selectRelativePreset(1); };

    lcdNumber.setColour(juce::Label::textColourId, lcdInk);
    lcdNumber.setFont(juce::Font(juce::FontOptions(15.0f, juce::Font::bold)));
    lcdNumber.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(lcdNumber);

    lcdName.setColour(juce::Label::textColourId, lcdInk);
    lcdName.setFont(juce::Font(juce::FontOptions(23.0f, juce::Font::bold)));
    lcdName.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(lcdName);

    lcdHint.setText("MORPHEUS PRESET DISPLAY", juce::dontSendNotification);
    lcdHint.setColour(juce::Label::textColourId, juce::Colour(0xff435342));
    lcdHint.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    addAndMakeVisible(lcdHint);

    editSectionLabel.setText("Z-PLANE  |  MORPHING FILTER", juce::dontSendNotification);
    editSectionLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe7edf5));
    editSectionLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    addAndMakeVisible(editSectionLabel);

    editNotice.setText("FILTER CATALOG + DSP PENDING VALIDATION  •  UI PREVIEW",
                       juce::dontSendNotification);
    editNotice.setColour(juce::Label::textColourId, juce::Colour(0xffffc477));
    editNotice.setFont(juce::Font(juce::FontOptions(9.0f)));
    addAndMakeVisible(editNotice);

    // UI placeholders only: actual Morpheus parameter names, ranges and filter
    // algorithms must be validated against the hardware SysEx specification
    // before these controls are connected to an audio engine.
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
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 18);
        slider.setRange(controlMins[i], controlMaxs[i], 0.001);
        slider.setValue(controlDefaults[i], juce::dontSendNotification);
        slider.setColour(juce::Slider::rotarySliderFillColourId, accentColour);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff596779));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff0d3a0));
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe7edf5));
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
    effectsSectionLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe7edf5));
    effectsSectionLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    addAndMakeVisible(effectsSectionLabel);

    const std::array<juce::String, 4> effectNames { "REVERB", "CHORUS", "DELAY", "DISTORTION" };
    for (size_t i = 0; i < effectToggles.size(); ++i) {
        effectToggles[i].setButtonText(effectNames[i]);
        effectToggles[i].setColour(juce::ToggleButton::textColourId, juce::Colour(0xffd8e1ed));
        effectToggles[i].setColour(juce::ToggleButton::tickColourId, accentColour);
        addAndMakeVisible(effectToggles[i]);
    }

    const juce::StringArray names {
        "CPU Firmware", "Sound ROM 1", "Sound ROM 2", "Sound ROM 3",
        "Sound ROM 4", "Preset Bank (.syx)"
    };
    const juce::StringArray descriptions {
        "Processor firmware image", "Sound memory image / socket 1",
        "Sound memory image / socket 2", "Sound memory image / socket 3",
        "Sound memory image / socket 4", "Morpheus SysEx preset bank"
    };

    for (int i = 0; i < slotCount; ++i) {
        auto* name = new juce::Label();
        name->setText(names[i], juce::dontSendNotification);
        name->setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
        addAndMakeVisible(name);
        slotNames.add(name);

        auto* info = new juce::Label();
        info->setText(descriptions[i], juce::dontSendNotification);
        info->setColour(juce::Label::textColourId, juce::Colour(0xff9aa9bc));
        info->setFont(juce::Font(juce::FontOptions(10.5f)));
        addAndMakeVisible(info);
        slotDescriptions.add(info);

        auto* path = new juce::Label();
        path->setText("No file selected", juce::dontSendNotification);
        path->setColour(juce::Label::textColourId, juce::Colour(0xffd8e1ed));
        path->setFont(juce::Font(juce::FontOptions(11.0f)));
        path->setMinimumHorizontalScale(0.7f);
        addAndMakeVisible(path);
        slotPaths.add(path);

        auto* status = new juce::Label();
        status->setJustificationType(juce::Justification::centredRight);
        status->setFont(juce::Font(juce::FontOptions(10.5f)));
        addAndMakeVisible(status);
        slotStatuses.add(status);

        auto* button = new juce::TextButton("Choose…");
        button->onClick = [this, i] { chooseFileForSlot(i); };
        addAndMakeVisible(button);
        slotButtons.add(button);
    }

    footer.setText(processor.getPresetBankStatus(), juce::dontSendNotification);
    footer.setColour(juce::Label::textColourId, juce::Colour(0xffffc477));
    footer.setFont(juce::Font(juce::FontOptions(10.5f)));
    addAndMakeVisible(footer);

    refreshFileStatus();
    refreshPresetList();
    refreshDisplay();
    startTimerHz(10);
    setSize(760, 850);
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
            slotStatuses[i]->setColour(juce::Label::textColourId, juce::Colour(0xff8995a5));
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
                               : "NO PRESET BANK", juce::dontSendNotification);
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
    g.fillAll(juce::Colour(0xff171b22));
    g.setColour(juce::Colour(0xff596779));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(8.0f), 10.0f, 1.0f);

    if (frontPanel != nullptr)
        frontPanel->drawWithin(g, juce::Rectangle<float>(18.0f, 10.0f,
            static_cast<float>(getWidth() - 36), 67.6f),
            juce::RectanglePlacement::stretchToFit, 1.0f);

    g.setColour(panelColour);
    g.fillRoundedRectangle(juce::Rectangle<float>(18.0f, 115.0f,
        static_cast<float>(getWidth() - 36), 112.0f), 8.0f);
    g.setColour(lcdColour);
    g.fillRoundedRectangle(juce::Rectangle<float>(28.0f, 125.0f,
        static_cast<float>(getWidth() - 56), 92.0f), 5.0f);
    g.setColour(juce::Colour(0xff252e3a));
    g.drawRoundedRectangle(juce::Rectangle<float>(18.0f, 240.0f,
        static_cast<float>(getWidth() - 36), 278.0f), 8.0f, 1.0f);
    g.setColour(juce::Colour(0xff202833));
    g.fillRoundedRectangle(juce::Rectangle<float>(18.0f, 530.0f,
        static_cast<float>(getWidth() - 36), 300.0f), 8.0f);
    g.setColour(juce::Colour(0xff596779));
    g.drawRoundedRectangle(juce::Rectangle<float>(18.0f, 530.0f,
        static_cast<float>(getWidth() - 36), 300.0f), 8.0f, 1.0f);
}

void MorpheusLabAudioProcessorEditor::resized() {
    title.setBounds(24, 14, getWidth() - 48, 34);
    subtitle.setBounds(25, 48, getWidth() - 50, 18);
    presetLabel.setBounds(28, 81, 90, 24);
    presetSelector.setBounds(122, 80, getWidth() - 146, 28);

    lcdNumber.setBounds(43, 132, 180, 24);
    lcdName.setBounds(43, 158, getWidth() - 86, 34);
    lcdHint.setBounds(getWidth() - 235, 132, 180, 20);
    previousButton.setBounds(30, 190, 105, 24);
    nextButton.setBounds(142, 190, 105, 24);

    editSectionLabel.setBounds(30, 246, 220, 20);
    editNotice.setBounds(getWidth() - 380, 247, 345, 18);
    const int controlWidth = (getWidth() - 76) / 4;
    for (size_t i = 0; i < editControls.size(); ++i) {
        const int row = static_cast<int>(i / 4);
        const int column = static_cast<int>(i % 4);
        const int x = 28 + column * controlWidth;
        const int y = 272 + row * 112;
        editLabels[i].setBounds(x, y, controlWidth - 8, 16);
        editControls[i].setBounds(x + 8, y + 15, controlWidth - 24, 92);
    }
    effectsSectionLabel.setBounds(30, 486, 90, 22);
    for (size_t i = 0; i < effectToggles.size(); ++i)
        effectToggles[i].setBounds(125 + static_cast<int>(i) * 145, 483, 135, 24);

    for (int i = 0; i < slotCount; ++i) {
        const int y = 546 + i * 44;
        slotNames[i]->setBounds(30, y, 145, 19);
        slotDescriptions[i]->setBounds(30, y + 19, 145, 16);
        slotPaths[i]->setBounds(185, y + 1, getWidth() - 370, 23);
        slotStatuses[i]->setBounds(getWidth() - 178, y + 1, 112, 23);
        slotButtons[i]->setBounds(getWidth() - 94, y, 72, 27);
    }
    footer.setBounds(25, 824, getWidth() - 50, 16);
}
