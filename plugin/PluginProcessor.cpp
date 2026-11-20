#include "PluginProcessor.h"
#include "PluginEditor.h"

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
        if (message.isNoteOn()) activeNote = message.getNoteNumber();
        else if (message.isNoteOff()
                 && (activeNote == message.getNoteNumber() || message.isAllNotesOff()))
            activeNote = -1;
        else if (message.isAllNotesOff() || message.isAllSoundOff()) activeNote = -1;
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

void MorpheusLabAudioProcessor::setRomPath(int slot, const juce::String& path) {
    if (slot >= 0 && slot < static_cast<int>(romPaths.size())) {
        romPaths[static_cast<size_t>(slot)] = path;
    }
}

juce::String MorpheusLabAudioProcessor::getRomPath(int slot) const {
    if (slot < 0 || slot >= static_cast<int>(romPaths.size()))
        return {};
    return romPaths[static_cast<size_t>(slot)];
}

void MorpheusLabAudioProcessor::getStateInformation(juce::MemoryBlock& destinationData) {
    juce::XmlElement state("MorpheusLabState");
    for (size_t i = 0; i < romPaths.size(); ++i) {
        auto* slot = state.createNewChildElement("RomSlot");
        slot->setAttribute("index", static_cast<int>(i));
        slot->setAttribute("path", romPaths[i]);
    }
    if (auto xml = state.toString(); xml.isNotEmpty())
        copyXmlToBinary(state, destinationData);
}

void MorpheusLabAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto state = getXmlFromBinary(data, sizeInBytes)) {
        if (state->hasTagName("MorpheusLabState")) {
            forEachXmlChildElementWithTagName(*state, slot, "RomSlot") {
                const int index = slot->getIntAttribute("index", -1);
                if (index >= 0 && index < static_cast<int>(romPaths.size()))
                    romPaths[static_cast<size_t>(index)] = slot->getStringAttribute("path");
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new MorpheusLabAudioProcessor();
}
