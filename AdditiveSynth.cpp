#include "AdditiveSynth.h"
#include <stdio.h>

// Constructor
AdditiveSynth::AdditiveSynth() {
    initializeADSR(); // Initialize ADSR settings

    // 🎛 Initialize Low-Pass Filter for smoother sound
    lowPassFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    lowPassFilter.setCutoffFrequency(2000.0f); // Default 2kHz cutoff
    lowPassFilter.setResonance(0.7f);
}


// Initialize ADSR Envelope
void AdditiveSynth::initializeADSR() {
    adsrParams.attack = 0.8f;   // Slow fade-in (pad-like)
    adsrParams.decay = 0.3f;    // Moderate decay
    adsrParams.sustain = 0.6f;  // Sustain level at 60%
    adsrParams.release = 1.2f;  // Long fade-out
    adsr.setParameters(adsrParams);
    adsr.noteOn(); // Start envelope
}

// Set Pentatonic Chord Harmonics
void AdditiveSynth::setPentatonicChord(float baseFreq) {
    harmonics.clear();
    float ratios[] = {1.0f, 9.0f/8.0f, 5.0f/4.0f, 3.0f/2.0f, 5.0f/3.0f, 2.0f}; 

    for (float r : ratios) {
        float detuneFactor = 1.0f + (juce::Random::getSystemRandom().nextFloat() * 0.02f - 0.01f); // ±1% detune
        harmonics.push_back({baseFreq * r * detuneFactor, 0.2f, 0.0f});
    }
}

// 🎵 Process Audio (Generate Next Sample)
float AdditiveSynth::process(float sampleRate) {
    float sample = 0.0f;

    // 🌊 LFO for slow pitch modulation
    float lfo = std::sin(lfoPhase * juce::MathConstants<float>::twoPi) * lfoDepth;
    lfoPhase += lfoSpeed / sampleRate;
    if (lfoPhase >= 1.0f) lfoPhase -= 1.0f; // Keep LFO phase in range

    for (auto& h : harmonics) {
        // ✅ Ensure each harmonic has its own phase
        h.phase += h.frequency / sampleRate;
        if (h.phase >= 1.0f) h.phase -= 1.0f;

        float modulatedFreq = h.frequency * (1.0f + lfo);
        float harmonicPhase = h.phase;
        h.phase += modulatedFreq / sampleRate;
        if (h.phase >= 1.0f) h.phase -= 1.0f;
        
        float sineWave = std::sin(h.phase * juce::MathConstants<float>::twoPi);
        float sawWave = 2.0f * (h.phase - std::floor(h.phase + 0.5f));
        float triWave = std::abs(4.0f * (h.phase - std::floor(h.phase + 0.5f)) - 1.0f);


        // 🏗️ Improved Mixing Strategy
        //float mixedWave = (0.3f * sineWave) + (0.5f * sawWave) + (0.2f * triWave);
        float mixedWave = (sineMix * 0.33 * sineWave) + (sawMix * 0.33 * sawWave) + (triMix * 0.33 * triWave);

        sample += mixedWave * h.amplitude;
    }

    // Normalize Sample to Prevent Clipping
    sample *= 0.5f;

    // 🎛 Apply ADSR Envelope
    sample *= adsr.getNextSample();

    // ✅ Apply Low-Pass Filter (Ensure Smoother Sound)
    sample = lowPassFilter.processSample(0, sample);

    
    

    sample *= 3.0f;

    
    //std::cout << sample << std::endl;

    if (sample >= 1.0) {
        sample = 1.0;
    }
    if (sample <= -1.0) {
        sample = -1.0;
    }

 

    return sample;
}


void AdditiveSynth::setMixingRatios(float sine, float saw, float tri) {
    sineMix = sine;
    sawMix = saw;
    triMix = tri;
}

void AdditiveSynth::setFilterCutoff(float cutoff) {
    filterCutoff = cutoff;
    lowPassFilter.setCutoffFrequency(filterCutoff);
}

void AdditiveSynth::setLfoDepth(float depth) {
    lfoDepth = depth;
}


