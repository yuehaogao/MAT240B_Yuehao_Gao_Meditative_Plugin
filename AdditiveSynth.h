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
};

/*
class AdditiveSynth {
public:
    struct Harmonic {
        float frequency;
        float amplitude;
        float phase;
    };

    std::vector<Harmonic> harmonics;


    // THE PENTATONIC SCALE
    void setPentatonicChord(float baseFreq) {
        harmonics.clear();
        float ratios[] = {1.0f, // I
            9.0/8.0f,           // II
            5.0f/4.0f,          // III
            3.0f/2.0f,          // V
            5.0f/3.0f,          // VI
            2.0f};              // I (Octave Higher)
            // Pentatonic

        for (float r : ratios) {
            float detuneFactor = 1.0f + (juce::Random::getSystemRandom().nextFloat() * 0.02f - 0.01f); // ±1% detune
            harmonics.push_back({baseFreq * r * detuneFactor, 0.2f, 0.0f});
        }
    }

    float process(float sampleRate) {
        float sample = 0;
        for (auto& osc : harmonics) {
            osc.phase += osc.frequency / sampleRate;
            if (osc.phase >= 1.0f) osc.phase -= 1.0f;
            sample += std::sin(osc.phase * 2.0f * juce::MathConstants<float>::pi) * osc.amplitude;
        }
        return sample;
    }

private:
    juce::ADSR adsr; // Envelope generator
    juce::ADSR::Parameters adsrParams;

};

*/
