#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final
    : public juce::AudioProcessorEditor, private juce::Timer {
 public:
  explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
  ~AudioPluginAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;
  void openFile();

 private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  AudioPluginAudioProcessor& processorRef;
  juce::Slider gainSlider;
  juce::Slider frequencySlider;
  juce::Slider chordRateSlider;
  juce::Slider sineMixSlider;
  juce::Slider sawMixSlider;
  juce::Slider triMixSlider;
  juce::Slider cutoffSlider;
  juce::Slider lfoDepthSlider;
  juce::Slider reverbMixSlider;
  juce::ComboBox irSelectBox;

  juce::Image churchImage, caveImage, roomImage;
  juce::ImageComponent imageDisplay;

  void timerCallback() override;


  std::vector<
      std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>
      attachment;
  std::vector<
      std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> 
      buttonAttachments;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> irAttachment;
    

  juce::TextButton openButton;
  std::unique_ptr<juce::FileChooser> chooser;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
