#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <vector>

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

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram.load(); }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destinationData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void setRomPath(int slot, const juce::String& path);
    juce::String getRomPath(int slot) const;
    int getPresetCount() const;
    int getProgramNumber(int index) const;
    juce::String getPresetBankStatus() const;

private:
    bool parsePresetBank(const juce::String& path);

    double sampleRateHz = 44100.0;
    double phase = 0.0;
    float amplitude = 0.12f;
    int activeNote = -1;
    std::atomic<int> currentProgram { 0 };
    std::array<juce::String, 6> romPaths {};
    juce::StringArray presetNames;
    std::vector<int> presetNumbers;
    juce::String presetBankStatus { "No preset bank loaded" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MorpheusLabAudioProcessor)
};
