#pragma once

#include <JuceHeader.h>

#include "AdditiveSynth.h"
#include "Library.h"


//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor {
 public:
  //==============================================================================
  AudioPluginAudioProcessor();
  ~AudioPluginAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  //==============================================================================
  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  void setBuffer(std::unique_ptr<juce::AudioBuffer<float>> buffer);

  juce::AudioProcessorValueTreeState apvts;
  // std::atomic<juce::AudioBuffer<float>*> buffer;

 private:
  ky::Ramp ramp;
  ky::Timer timer;
  ky::SchroederReverb reverb, reverb2;
  ky::AttackDecay env;

  std::unique_ptr<ky::ClipPlayer> player;
  juce::dsp::Convolution convolution;

  void loadSelectedImpulseResponse();
  int lastLoadedIR = -1;
  

  AdditiveSynth synth;
  int currentChordIndex = 0;  // Keeps track of which chord is playing
  float chordChangeTimer = 0; // Timer to track when to switch chords
  float chordChangeInterval = 5.0f; // Default: Change every 5 seconds

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};

juce::AudioProcessorValueTreeState::ParameterLayout parameters();

