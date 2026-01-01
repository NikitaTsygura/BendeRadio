/*
 * BendeRadio BT - Audio Gain Limiter Implementation
 * 
 * Applies digital gain reduction and clipping protection to prevent
 * DAC and amplifier overload when receiving high-level audio from PCs.
 */

#include "AudioGainLimiter.h"

AudioGainLimiter::AudioGainLimiter()
    : gainFactor(0.3f)  // Default: 30% of full scale (prevents PC audio clipping)
{
}

void AudioGainLimiter::init(float gain) {
    setGainFactor(gain);
}

void AudioGainLimiter::setGainFactor(float gain) {
    // Constrain gain to safe range (0.0 to 1.0)
    gainFactor = constrain(gain, 0.0f, 1.0f);
}

audio_sample_t AudioGainLimiter::processSample(audio_sample_t sample) {
    // Apply gain reduction
    float scaled = (float)sample * gainFactor;
    
    // Convert back to integer and constrain to prevent clipping
    audio_sample_t result = (audio_sample_t)scaled;
    result = constrain(result, DAC_MAX_NEGATIVE, DAC_MAX_POSITIVE);
    
    return result;
}

void AudioGainLimiter::processStereoPair(audio_sample_t& left, audio_sample_t& right) {
    left = processSample(left);
    right = processSample(right);
}

size_t AudioGainLimiter::processBuffer(uint8_t* buffer, size_t len) {
    // Process as 16-bit samples (stereo = interleaved L, R)
    // len should be multiple of 4 (2 bytes per sample × 2 channels)
    size_t sampleCount = len / 2;  // Number of 16-bit samples
    size_t pairCount = sampleCount / 2;  // Number of stereo pairs
    
    audio_sample_t* samples = (audio_sample_t*)buffer;
    
    // Process each stereo pair
    for (size_t i = 0; i < pairCount; i++) {
        size_t leftIdx = i * 2;
        size_t rightIdx = leftIdx + 1;
        
        samples[leftIdx] = processSample(samples[leftIdx]);
        samples[rightIdx] = processSample(samples[rightIdx]);
    }
    
    // Return processed length (same as input)
    return len;
}

