/*
 * BendeRadio BT - Audio Gain Limiter Module
 * 
 * Prevents audio clipping/distortion by applying digital gain limiting
 * before audio is sent to the DAC. Specifically addresses the issue where
 * Windows PCs send audio at higher digital levels than smartphones.
 */

#pragma once

#include <Arduino.h>

// Audio sample type (16-bit signed integer)
typedef int16_t audio_sample_t;

// DAC limits (16-bit signed integer range)
#define DAC_MAX_POSITIVE  32767
#define DAC_MAX_NEGATIVE -32768
#define DAC_MAX_ABS       32767

class AudioGainLimiter {
public:
    AudioGainLimiter();
    
    // Initialize gain limiter
    // gainFactor: 0.0 to 1.0 (e.g., 0.3 = 30% of full scale)
    void init(float gainFactor = 0.3f);
    
    // Process a single audio sample (16-bit signed)
    // Returns processed sample with gain applied and clipping protection
    audio_sample_t processSample(audio_sample_t sample);
    
    // Process stereo pair (left, right channels)
    void processStereoPair(audio_sample_t& left, audio_sample_t& right);
    
    // Process audio buffer (interleaved stereo: L, R, L, R, ...)
    // buffer: pointer to audio data
    // len: length in bytes
    // Returns: number of bytes processed
    size_t processBuffer(uint8_t* buffer, size_t len);
    
    // Set gain factor (0.0 to 1.0)
    void setGainFactor(float gain);
    
    // Get current gain factor
    float getGainFactor() const { return gainFactor; }
    
private:
    float gainFactor;  // Gain multiplier (0.0-1.0)
};

