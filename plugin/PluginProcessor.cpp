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

    // This first shell deliberately uses a simple oscillator so the AU can be
    // verified in a DAW before any proprietary Morpheus firmware or ROM exists.
    for (const auto metadata : midiMessages) {
        const auto message = metadata.getMessage();
        if (message.isNoteOn()) {
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
            ? amplitude * static_cast<float>(std::sin(phase))
            : 0.0f;
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

void MorpheusLabAudioProcessor::getStateInformation(juce::MemoryBlock& destinationData) {
    const juce::ValueTree state("MorpheusLabState");
    juce::MemoryOutputStream stream(destinationData, false);
    state.writeToStream(stream);
}

void MorpheusLabAudioProcessor::setStateInformation(const void*, int) {
    // No user parameters are stored in this initial test shell.
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new MorpheusLabAudioProcessor();
}
