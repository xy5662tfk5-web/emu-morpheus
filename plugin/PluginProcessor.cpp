#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "morpheus/SysExPresetBank.h"

MorpheusLabAudioProcessor::MorpheusLabAudioProcessor()
    : juce::AudioProcessor(BusesProperties().withOutput(
          "Output", juce::AudioChannelSet::stereo(), true)) {}

void MorpheusLabAudioProcessor::prepareToPlay(double sampleRate, int) {
    sampleRateHz = sampleRate > 0.0 ? sampleRate : 44100.0;
    phase = 0.0;
}

void MorpheusLabAudioProcessor::releaseResources() {}

bool MorpheusLabAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto output = layouts.getMainOutputChannelSet();
    return output == juce::AudioChannelSet::mono()
        || output == juce::AudioChannelSet::stereo();
}

void MorpheusLabAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    for (const auto metadata : midiMessages) {
        const auto message = metadata.getMessage();
        if (message.isProgramChange()) {
            setCurrentProgram(message.getProgramChangeNumber());
        } else if (message.isNoteOn()) {
            activeNote = message.getNoteNumber();
        } else if (message.isNoteOff()
                   && (activeNote == message.getNoteNumber() || message.isAllNotesOff())) {
            activeNote = -1;
        } else if (message.isAllNotesOff() || message.isAllSoundOff()) {
            activeNote = -1;
        }
    }
    midiMessages.clear();

    const auto channelCount = buffer.getNumChannels();
    const auto sampleCount = buffer.getNumSamples();
    const double increment = activeNote >= 0
        ? juce::MathConstants<double>::twoPi
            * juce::MidiMessage::getMidiNoteInHertz(activeNote) / sampleRateHz
        : 0.0;

    for (int sample = 0; sample < sampleCount; ++sample) {
        const float value = activeNote >= 0
            ? amplitude * static_cast<float>(std::sin(phase)) : 0.0f;
        phase += increment;
        if (phase >= juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;
        for (int channel = 0; channel < channelCount; ++channel)
            buffer.setSample(channel, sample, value);
    }
}

juce::AudioProcessorEditor* MorpheusLabAudioProcessor::createEditor() {
    return new MorpheusLabAudioProcessorEditor(*this);
}

int MorpheusLabAudioProcessor::getNumPrograms() {
    return juce::jmax(1, presetNames.size());
}

void MorpheusLabAudioProcessor::setCurrentProgram(int index) {
    if (presetNames.isEmpty()) {
        currentProgram.store(0);
        return;
    }
    currentProgram.store(juce::jlimit(0, presetNames.size() - 1, index));
}

const juce::String MorpheusLabAudioProcessor::getProgramName(int index) {
    if (index < 0 || index >= presetNames.size())
        return presetNames.isEmpty() ? "No preset bank loaded" : "Unknown preset";
    return presetNames[index];
}

int MorpheusLabAudioProcessor::getPresetCount() const {
    return presetNames.size();
}

juce::String MorpheusLabAudioProcessor::getPresetBankStatus() const {
    return presetBankStatus;
}

bool MorpheusLabAudioProcessor::parsePresetBank(const juce::String& path) {
    presetNames.clear();
    currentProgram.store(0);

    juce::MemoryBlock fileData;
    const juce::File file(path);
    if (!file.existsAsFile() || !file.loadFileAsData(fileData)) {
        presetBankStatus = "Cannot read preset bank";
        return false;
    }

    const auto* bytes = static_cast<const std::uint8_t*>(fileData.getData());
    const auto parsed = morpheus::parsePresetBankSysEx(bytes, fileData.getSize());

    for (const auto& preset : parsed.presets)
        presetNames.add(juce::String::fromUTF8(preset.name.c_str()));

    if (presetNames.isEmpty()) {
        presetBankStatus = "No valid Morpheus presets ("
            + juce::String(static_cast<int>(parsed.invalidMessages))
            + " invalid, "
            + juce::String(static_cast<int>(parsed.ignoredMessages))
            + " ignored)";
        return false;
    }

    presetBankStatus = "Loaded " + juce::String(presetNames.size())
        + " presets ("
        + juce::String(static_cast<int>(parsed.invalidMessages))
        + " invalid, "
        + juce::String(static_cast<int>(parsed.ignoredMessages))
        + " ignored)";
    return true;
}

void MorpheusLabAudioProcessor::setRomPath(int slot, const juce::String& path) {
    if (slot < 0 || slot >= static_cast<int>(romPaths.size()))
        return;

    romPaths[static_cast<size_t>(slot)] = path;
    if (slot == 5)
        parsePresetBank(path);
}

juce::String MorpheusLabAudioProcessor::getRomPath(int slot) const {
    if (slot < 0 || slot >= static_cast<int>(romPaths.size()))
        return {};
    return romPaths[static_cast<size_t>(slot)];
}

void MorpheusLabAudioProcessor::getStateInformation(juce::MemoryBlock& destinationData) {
    juce::XmlElement state("MorpheusLabState");
    state.setAttribute("currentProgram", currentProgram.load());
    for (size_t i = 0; i < romPaths.size(); ++i) {
        auto* slot = state.createNewChildElement("RomSlot");
        slot->setAttribute("index", static_cast<int>(i));
        slot->setAttribute("path", romPaths[i]);
    }
    copyXmlToBinary(state, destinationData);
}

void MorpheusLabAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto state = getXmlFromBinary(data, sizeInBytes)) {
        if (state->hasTagName("MorpheusLabState")) {
            forEachXmlChildElementWithTagName(*state, slot, "RomSlot") {
                const int index = slot->getIntAttribute("index", -1);
                if (index >= 0 && index < static_cast<int>(romPaths.size())) {
                    const auto path = slot->getStringAttribute("path");
                    romPaths[static_cast<size_t>(index)] = path;
                    if (index == 5 && path.isNotEmpty())
                        parsePresetBank(path);
                }
            }
            setCurrentProgram(state->getIntAttribute("currentProgram", 0));
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new MorpheusLabAudioProcessor();
}
