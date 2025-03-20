#include "PluginProcessor.h"

#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout parameters() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameter_list;

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(     // using
      ParameterID{"gain", 1}, "Gain", -60.0, 0.0, -60.0));  

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"frequency", 1}, "Frequency", 0.0, 127.0, 60.0));

  // parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
  //     ParameterID{"distortion", 1}, "Distortion", 0.0, 1.0, 0.0));

  // parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
  //     ParameterID{"rate", 1}, "Rate", 0.0, 1.0, 0.0));

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(     // using
      ParameterID{"chordRate", 1}, "Chord Change Rate", 3.0f, 9.0f, 5.0f));
      
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterID{"sineMix", 1}, "Sine Mix", 0.0f, 1.0f, 0.3f));
    
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterID{"sawMix", 1}, "Saw Mix", 0.0f, 1.0f, 0.5f));
    
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterID{"triMix", 1}, "Tri Mix", 0.0f, 1.0f, 0.2f));

  return {parameter_list.begin(), parameter_list.end()};
}

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      apvts(*this, nullptr, "Parameters", parameters()) {
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AudioPluginAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram() { return 0; }

void AudioPluginAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index,
                                                  const juce::String& newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  juce::ignoreUnused(sampleRate, samplesPerBlock);

  ky::setPlaybackRate(static_cast<float>(getSampleRate()));

  // Configure convolution reverb (keep this if needed)
  convolution.reset();
  juce::dsp::ProcessSpec spec;
  spec.sampleRate = sampleRate;
  spec.maximumBlockSize = samplesPerBlock;
  spec.numChannels = getTotalNumOutputChannels();
  convolution.prepare(spec);

  // 🎵 Initialize the Additive Synth to play an A2 pentatonic chord
  synth.setPentatonicChord(220.0f); // A2 = 220 Hz
  chordChangeTimer = 0;

  // reverb.configure();

  // convolution.reset();

  //Uncomment this part later, since the file "untitled.wav" is currently not there

  // juce::File file =
  //    juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
  //        .getChildFile("untitled.wav");
  // // juce::File file =
  // //    juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
  // //        .getChildFile("emt_140_bright_3.wav");
  // if (!file.exists()) {
  //   std::cout << "File does not exist" << std::endl;
  //   exit(1);
  // }


  // convolution.loadImpulseResponse(file, juce::dsp::Convolution::Stereo::yes,
  //                                 juce::dsp::Convolution::Trim::no, 0);

  // juce::dsp::ProcessSpec spec;
  // spec.sampleRate = sampleRate;
  // spec.maximumBlockSize = static_cast<uint32>(samplesPerBlock);
  // spec.numChannels = static_cast<uint32>(getTotalNumOutputChannels());
  // convolution.prepare(spec);
  
}

void AudioPluginAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages) {
  juce::ignoreUnused(midiMessages);
  juce::ScopedNoDenormals noDenormals;

  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();
  
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  
  float gainValue = apvts.getParameter("gain")->getValue();                // using
  float f = apvts.getParameter("frequency")->getValue();
  // float t = apvts.getParameter("distortion")->getValue();
  // float r = apvts.getParameter("rate")->getValue();  
  float chordChangingRate = apvts.getParameter("chordRate")->getValue();   // using
  float sineMix = apvts.getParameter("sineMix")->getValue();
  float sawMix = apvts.getParameter("sawMix")->getValue();
  float triMix = apvts.getParameter("triMix")->getValue();

  synth.setMixingRatios(sineMix, sawMix, triMix);
  

  // auto thing = this->buffer.exchange(nullptr, std::memory_order_acq_rel);

  // ramp.frequency(ky::mtof(f * 127));
  //timer.frequency(7 * r);
  ramp.frequency(0.3f);

  // 🎚️ Get user-defined chord change rate from slider
  chordChangeInterval = chordChangingRate + 0.5; 
  //std::cout << "Chord Change Interval: " << chordChangeInterval << " seconds" << std::endl;

  // 🎵 Chord Progression Logic: Switch chords automatically
  chordChangeTimer += (buffer.getNumSamples() / getSampleRate()) / 6.0f;
  //std::cout << "Chord Change Interval: " << chordChangeInterval << " seconds" << std::endl;
  
  
  if (chordChangeTimer >= chordChangeInterval) {
      chordChangeTimer = 0; // Reset timer
      currentChordIndex = (currentChordIndex + 1) % 4; // Cycle through 4 chords

      // Set the new chord based on index
      if (currentChordIndex == 0) {
          synth.setPentatonicChord(220.0f); // A2 pentatonic
      } else if (currentChordIndex == 1) {
          synth.setPentatonicChord(164.8f); // E2 pentatonic
      } else if (currentChordIndex == 2) {
          synth.setPentatonicChord(146.8f); // D2 pentatonic
      } else {
          synth.setPentatonicChord(196.0f); // G2 pentatonic
      }
  }


  // ✅ Keep existing sample buffers
  auto* leftChannel = buffer.getWritePointer(0);
  auto* rightChannel = buffer.getWritePointer(1);
  
  for (int i = 0; i < buffer.getNumSamples(); ++i) {
    // if (timer()) {
    //   env.set(0.05f, 0.3f);
    // }
    // float sample = env() * ky::sin7(ramp()) * ky::dbtoa(-60 * (1 - v));
    //  sample = reverb(sample);

    // float sample = player ? player->operator()(ramp()) : 0;

    // buffer.addSample(0, i, sample);
    // buffer.addSample(1, i, sample);
    float sample = synth.process(getSampleRate());
    // ✅ Apply gain (volume control)
    
    // sample *= v; 
    sample *= gainValue; 

    // ✅ Send to Left & Right Channels
    leftChannel[i] = sample;
    rightChannel[i] = sample;
  }

  // ✅ Keep Convolution Reverb (if active)
  juce::dsp::AudioBlock<float> block(buffer);
  juce::dsp::ProcessContextReplacing<float> context(block);
  convolution.process(context);

}

void AudioPluginAudioProcessor::setBuffer(
    std::unique_ptr<juce::AudioBuffer<float>> buffer) {
  juce::ignoreUnused(buffer);

  auto b = std::make_unique<ky::ClipPlayer>();
  for (int i = 0; i < buffer->getNumSamples(); ++i) {
    if (buffer->getNumChannels() == 2) {
      b->addSample(buffer->getSample(0, i) / 2 + buffer->getSample(1, i) / 2);
    } else {
      b->addSample(buffer->getSample(0, i));
    }
  }

  player = std::move(b);
}

bool AudioPluginAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
  return new AudioPluginAudioProcessorEditor(*this);
}

void AudioPluginAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData) {
  auto state = apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data,
                                                    int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));

  if (xmlState.get() != nullptr)
    if (xmlState->hasTagName(apvts.state.getType()))
      apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new AudioPluginAudioProcessor();
}
