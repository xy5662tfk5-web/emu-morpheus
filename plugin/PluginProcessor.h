#pragma once

#include <JuceHeader.h>
#include <array>

class MorpheusLabAudioProcessor final : public juce::AudioProcessor {
public:
    MorpheusLabAudioProcessor();
    ~MorpheusLabAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "MIDI test tone"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destinationData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void setRomPath(int slot, const juce::String& path);
    juce::String getRomPath(int slot) const;

private:
    double sampleRateHz = 44100.0;
    double phase = 0.0;
    float amplitude = 0.12f;
    int activeNote = -1;
    std::array<juce::String, 6> romPaths {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorpheusLabAudioProcessor)
};
