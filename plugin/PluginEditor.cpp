#include "PluginEditor.h"

MorpheusLabAudioProcessorEditor::MorpheusLabAudioProcessorEditor(
    MorpheusLabAudioProcessor& p)
    : juce::AudioProcessorEditor(p), processor(p) {
    title.setText("E-MU MORPHEUS LAB", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centredLeft);
    title.setFont(juce::Font(juce::FontOptions(22.0f, juce::Font::bold)));
    addAndMakeVisible(title);

    subtitle.setText("SYSEX PRESET BANK READER  |  ROM FILE MANAGER",
                     juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xff9aa9bc));
    subtitle.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(subtitle);

    presetLabel.setText("PRESET", juce::dontSendNotification);
    presetLabel.setColour(juce::Label::textColourId, juce::Colour(0xffd8e1ed));
    presetLabel.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    addAndMakeVisible(presetLabel);

    presetSelector.setTextWhenNothingSelected("Choose a preset bank (.syx)");
    presetSelector.onChange = [this] {
        const int selectedId = presetSelector.getSelectedId();
        if (selectedId > 0)
            processor.setCurrentProgram(selectedId - 1);
        footer.setText(processor.getPresetBankStatus(), juce::dontSendNotification);
    };
    addAndMakeVisible(presetSelector);

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
    setSize(680, 530);
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
        presetSelector.setSelectedId(processor.getCurrentProgram() + 1,
                                      juce::dontSendNotification);
        footer.setText(processor.getPresetBankStatus(), juce::dontSendNotification);
    } else {
        presetSelector.setTextWhenNothingSelected("No valid presets loaded");
        footer.setText(processor.getPresetBankStatus(), juce::dontSendNotification);
    }
}

void MorpheusLabAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff171b22));
    g.setColour(juce::Colour(0xff596779));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(8.0f), 10.0f, 1.0f);
    g.setColour(juce::Colour(0xff252e3a));
    g.drawRoundedRectangle(juce::Rectangle<float>(18.0f, 118.0f,
        static_cast<float>(getWidth() - 36), 350.0f), 8.0f, 1.0f);
}

void MorpheusLabAudioProcessorEditor::resized() {
    title.setBounds(24, 16, getWidth() - 48, 34);
    subtitle.setBounds(25, 51, getWidth() - 50, 20);
    presetLabel.setBounds(28, 78, 70, 24);
    presetSelector.setBounds(100, 76, getWidth() - 128, 28);

    for (int i = 0; i < slotCount; ++i) {
        const int y = 132 + i * 53;
        slotNames[i]->setBounds(30, y, 145, 20);
        slotDescriptions[i]->setBounds(30, y + 20, 145, 17);
        slotPaths[i]->setBounds(185, y + 2, getWidth() - 370, 24);
        slotStatuses[i]->setBounds(getWidth() - 178, y + 2, 112, 24);
        slotButtons[i]->setBounds(getWidth() - 94, y, 72, 28);
    }
    footer.setBounds(25, 482, getWidth() - 50, 25);
}
