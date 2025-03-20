#include <JuceHeader.h>

#include <iostream>

int main(int argc, char* argv[]) {
    juce::dsp::Convolution convolution;
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = 48000;
    spec.maximumBlockSize = 1024;
    spec.numChannels = 1;
    convolution.prepare(spec);
    
    
    juce::File file = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("IR.wav");


}
