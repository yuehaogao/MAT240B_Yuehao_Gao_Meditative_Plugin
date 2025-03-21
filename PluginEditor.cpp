#include "PluginEditor.h"

#include "PluginProcessor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  setSize(660, 722);

  churchImage = juce::ImageFileFormat::loadFrom(
      juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("church.png"));

  caveImage = juce::ImageFileFormat::loadFrom(
      juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("cave.png"));

  roomImage = juce::ImageFileFormat::loadFrom(
      juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("room.png")); 

  imageDisplay.setImage(churchImage, juce::RectanglePlacement::centred);
      
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "gain", gainSlider));
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "frequency", frequencySlider));
  attachment.push_back(
      std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "chordRate", chordRateSlider));
  attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "sineMix", sineMixSlider));
  attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "sawMix", sawMixSlider));
  attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "triMix", triMixSlider));
  attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "cutoff", cutoffSlider));
  attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "lfoDepth", lfoDepthSlider));
  attachment.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
          processorRef.apvts, "reverbMix", reverbMixSlider));

  irSelectBox.addItem("Church", 1);
  irSelectBox.addItem("Cave", 2);
  irSelectBox.addItem("Room", 3);

  addAndMakeVisible(gainSlider);
  gainSlider.setTextValueSuffix(" dB (gain)");
  addAndMakeVisible(frequencySlider);
  frequencySlider.setTextValueSuffix(" (freq ratio)");
  addAndMakeVisible(chordRateSlider);
  chordRateSlider.setTextValueSuffix(" s (chord rate)");
  addAndMakeVisible(sineMixSlider);
  sineMixSlider.setTextValueSuffix("(sine mix)");
  addAndMakeVisible(sawMixSlider);
  sawMixSlider.setTextValueSuffix("(saw mix)");
  addAndMakeVisible(triMixSlider);
  triMixSlider.setTextValueSuffix("(tri mix)");
  addAndMakeVisible(cutoffSlider);
  cutoffSlider.setTextValueSuffix(" Hz (cutoff)");
  addAndMakeVisible(lfoDepthSlider);
  lfoDepthSlider.setTextValueSuffix(" (LFO depth)");
  addAndMakeVisible(reverbMixSlider);
  reverbMixSlider.setTextValueSuffix(" (dry|wet)");

  addAndMakeVisible(irSelectBox);
  addAndMakeVisible(imageDisplay);

  // ✅ Start the timer after everything is set up
  startTimerHz(10);

  irAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
    processorRef.apvts, "irChoice", irSelectBox);


  chooser = std::make_unique<juce::FileChooser>(
      "Select a file to open...",
      juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
      "*.wav;*.mp3;*.aif;*.flac");
  openButton.setButtonText("select audio file");
  openButton.onClick = [this] {
    auto fn = [this](const juce::FileChooser& c) {
      juce::File file = c.getResult();
      if (!file.exists()) return;

      juce::AudioFormatManager formatManager;
      formatManager.registerBasicFormats();
      std::unique_ptr<juce::AudioFormatReader> reader(
          formatManager.createReaderFor(file));
      if (reader == nullptr) return;

      auto buffer = std::make_unique<juce::AudioBuffer<float>>(
          static_cast<int>(reader->numChannels),
          static_cast<int>(reader->lengthInSamples));

      reader->read(buffer.get(), 0, static_cast<int>(reader->lengthInSamples),
                   0, true, true);

      processorRef.setBuffer(std::move(buffer));
    };

    chooser->launchAsync(juce::FileBrowserComponent::canSelectFiles, fn);
  };

  addAndMakeVisible(openButton);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized() {
  auto area = getLocalBounds();
  auto height = 40;
  openButton.setBounds(area.removeFromTop(height));
  gainSlider.setBounds(area.removeFromTop(height));
  frequencySlider.setBounds(area.removeFromTop(height));
  chordRateSlider.setBounds(area.removeFromTop(height));
  sineMixSlider.setBounds(area.removeFromTop(height));
  sawMixSlider.setBounds(area.removeFromTop(height));
  triMixSlider.setBounds(area.removeFromTop(height));
  cutoffSlider.setBounds(area.removeFromTop(height));
  lfoDepthSlider.setBounds(area.removeFromTop(height));
  reverbMixSlider.setBounds(area.removeFromTop(height));
  irSelectBox.setBounds(area.removeFromTop(height));

  //imageDisplay.setBounds(getWidth() - 200, 0, 200, 200);
  imageDisplay.setBounds(area.removeFromTop(260));
}


void AudioPluginAudioProcessorEditor::timerCallback()
{
    // 👇 Example: switch the displayed image based on IR selection
    auto irChoice = processorRef.apvts.getParameter("irChoice")->getValue(); // 0, 1, or 2

    std::cout << irChoice << std::endl;

    if (irChoice == 0)
        imageDisplay.setImage(churchImage, juce::RectanglePlacement::centred);
    else if (irChoice == 0.5)
        imageDisplay.setImage(caveImage, juce::RectanglePlacement::centred);
    else if (irChoice == 1)
        imageDisplay.setImage(roomImage, juce::RectanglePlacement::centred);
}
