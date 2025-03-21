#pragma once
#include <vector>
#include <cmath>
#include <JuceHeader.h>



class AdditiveSynth {
    public:
        AdditiveSynth();
        
        void setPentatonicChord(float baseFreq);
        float process(float sampleRate);
        void initializeADSR(); // Add this function to initialize ADSR
        void setMixingRatios(float sine, float saw, float tri);
        void setFilterCutoff(float cutoff);
        void setLfoDepth(float depth);
    
    private:
        struct Harmonic {
            float frequency;
            float amplitude;
            float phase;
        };
    
        std::vector<Harmonic> harmonics;
    
        float phase = 0.0f; // Global phase accumulator

        // 🎚️ ADSR Envelope (Correct declaration)
        juce::ADSR adsr;
        juce::ADSR::Parameters adsrParams;
    
        // 🎛 Sound Texture Enhancements
        juce::dsp::StateVariableTPTFilter<float> lowPassFilter; // Smooth filter
        float lfoPhase = 0.0f; // For pitch modulation
        float lfoSpeed = 0.1f; // Slow movement speed
        float lfoDepth = 0.002f; // Subtle detuning effect

        float sineMix = 0.3f;
        float sawMix = 0.5f;
        float triMix = 0.2f;

        float filterCutoff = 2000.0f;
};


