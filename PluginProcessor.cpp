#include "PluginProcessor.h"

#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout parameters() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameter_list;

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(     // using
      ParameterID{"gain", 1}, "Gain", -60.0, 0.0, -60.0));  

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
      ParameterID{"frequency", 1}, "Frequency", 0.75, 1.25, 1.0));

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(     // using
      ParameterID{"chordRate", 1}, "Chord Change Rate", 3.0f, 9.0f, 5.0f));
      
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterID{"sineMix", 1}, "Sine Mix", 0.0f, 1.0f, 0.3f));
    
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterID{"sawMix", 1}, "Saw Mix", 0.0f, 1.0f, 0.5f));
    
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterID{"triMix", 1}, "Tri Mix", 0.0f, 1.0f, 0.2f));
  
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterID{"cutoff", 1}, "Cutoff Frequency", 100.0f, 10000.0f, 2000.0f));
  
  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterID{"lfoDepth", 1}, "LFO Depth", 0.0f, 1.0f, 0.2f));

  parameter_list.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParameterID{"reverbMix", 1}, "Reverb Mix", 0.0f, 1.0f, 0.5f));
  
  parameter_list.push_back(std::make_unique<juce::AudioParameterChoice>(
        ParameterID{"irChoice", 1}, "IR Choice",
        juce::StringArray{"Church", "Cave", "Room"},
        0  // Default to Church
  ));
  


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

  ky::setPlaybackRate(static_cast<float>(sampleRate));

  // ✅ Prepare convolution reverb
  juce::dsp::ProcessSpec spec;
  spec.sampleRate = sampleRate;
  spec.maximumBlockSize = samplesPerBlock;
  spec.numChannels = getTotalNumOutputChannels();

  convolution.reset();
  convolution.prepare(spec);

  // ✅ Load user-selected IR from dropdown
  loadSelectedImpulseResponse();  // 🪄 This is your new function that uses irChoice

  // ✅ Reset synth
  synth.setPentatonicChord(220.0f); // A2 pentatonic to start
  chordChangeTimer = 0;


  // initialisation that you need..
  // juce::ignoreUnused(sampleRate, samplesPerBlock);

  // ky::setPlaybackRate(static_cast<float>(getSampleRate()));

  // // Configure convolution reverb (keep this if needed)
  // convolution.reset();
  // juce::dsp::ProcessSpec spec;
  // spec.sampleRate = sampleRate;
  // spec.maximumBlockSize = samplesPerBlock;
  // spec.numChannels = getTotalNumOutputChannels();
  // convolution.prepare(spec);

  // // 🎵 Load your impulse response file
  // juce::File irFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
  // .getChildFile("church_ir.wav");

  // if (!irFile.existsAsFile()) {
  //   DBG("❌ IR file not found! Please check path.");
  // } else {
  //   convolution.loadImpulseResponse(irFile,
  //     juce::dsp::Convolution::Stereo::yes,
  //     juce::dsp::Convolution::Trim::no,
  //     0);  // zero latency
  //   DBG("✅ IR file loaded successfully.");
  // }

  



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

  
  float gainValue = apvts.getParameter("gain")->getValue();
  float freq = apvts.getParameter("frequency")->getValue();
  float chordChangingRate = apvts.getParameter("chordRate")->getValue();
  float sineMix = apvts.getParameter("sineMix")->getValue();
  float sawMix = apvts.getParameter("sawMix")->getValue();
  float triMix = apvts.getParameter("triMix")->getValue();
  float cutoff = apvts.getParameter("cutoff")->getValue();
  float lfoDepth = *apvts.getRawParameterValue("lfoDepth");
  float reverbMix = apvts.getParameter("reverbMix")->getValue();


  synth.setMixingRatios(sineMix, sawMix, triMix);
  synth.setFilterCutoff(cutoff * 9500 + 500);
  synth.setLfoDepth(lfoDepth * 0.05);
  

  // auto thing = this->buffer.exchange(nullptr, std::memory_order_acq_rel);

  // ramp.frequency(ky::mtof(f * 127));
  //timer.frequency(7 * r);
  // ramp.frequency(0.3f);

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
          synth.setPentatonicChord(110.0f * (0.75 + 0.5 * freq)); // A2 pentatonic
      } else if (currentChordIndex == 1) {
          synth.setPentatonicChord(82.4f * (0.75 + 0.5 * freq)); // E2 pentatonic
      } else if (currentChordIndex == 2) {
          synth.setPentatonicChord(73.4f * (0.75 + 0.5 * freq)); // D2 pentatonic
      } else {
          synth.setPentatonicChord(98.0f * (0.75 + 0.5 * freq)); // G2 pentatonic
      }
  }


  // ✅ Keep existing sample buffers
  auto* leftChannel = buffer.getWritePointer(0);
  auto* rightChannel = buffer.getWritePointer(1);
  
  for (int i = 0; i < buffer.getNumSamples(); ++i) {

    float sample = synth.process(getSampleRate());
    // ✅ Apply gain (volume control)
    
    // sample *= v; 
    sample *= gainValue; 

    // ✅ Send to Left & Right Channels
    leftChannel[i] = sample;
    rightChannel[i] = sample;
  }

  // Store dry buffer first (before reverb)
  juce::AudioBuffer<float> dryBuffer;
  dryBuffer.makeCopyOf(buffer);

  // ✅ Keep Convolution Reverb (if active)
  juce::dsp::AudioBlock<float> block(buffer);
  juce::dsp::ProcessContextReplacing<float> context(block);
  convolution.process(context);

  // Now blend dry and wet buffers
  for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
    float* wet = buffer.getWritePointer(channel);
    float* dry = dryBuffer.getWritePointer(channel);
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        wet[i] = (1.0f - reverbMix) * dry[i] + reverbMix * wet[i];
    }
  }
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

void AudioPluginAudioProcessor::loadSelectedImpulseResponse() {
  int selectedIR = static_cast<int>(*apvts.getRawParameterValue("irChoice"));

  if (selectedIR == lastLoadedIR)
      return;  // Skip if already loaded

  juce::File irFile;
  auto desktop = juce::File::getSpecialLocation(juce::File::userDesktopDirectory);

  if (selectedIR == 0)
      irFile = desktop.getChildFile("church_ir.wav");
  else if (selectedIR == 0.5)
      irFile = desktop.getChildFile("cave_ir.wav");
  else if (selectedIR == 1)
      irFile = desktop.getChildFile("room_ir.wav");

  if (irFile.existsAsFile()) {
      convolution.loadImpulseResponse(irFile,
          juce::dsp::Convolution::Stereo::yes,
          juce::dsp::Convolution::Trim::no,
          0);
      lastLoadedIR = selectedIR;
      std::cout << "Loaded IR: " << irFile.getFileName() << std::endl;
  } else {
      std::cout << "IR file not found: " << irFile.getFullPathName() << std::endl;
  }
}


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new AudioPluginAudioProcessor();
}
