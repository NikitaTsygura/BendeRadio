/*
 * BendeRadio BT - Audio Processing Module
 * 
 * Handles audio level analysis, beat detection, and audio data processing
 * from the ADC input connected to the amplifier output
 */

#pragma once

#include <Arduino.h>
#include "config.h"

class AudioProcessor {
public:
    AudioProcessor();
    
    // Initialization (call once in setup)
    void init(uint8_t bclk, uint8_t lrc, uint8_t dout, uint8_t adcPin, uint16_t threshold);
    
    // Main update function (call in loop)
    void tick();
    
    // Check if new audio data is available
    bool hasNewData() const { return newDataAvailable; }
    
    // Get processed audio level (0-100)
    uint8_t getAudioLevel() const { return smoothedLevel; }
    
    // Get raw ADC value
    uint16_t getRawLevel() const { return rawLevel; }
    
    // Get peak level for calibration
    uint16_t getPeakLevel() const { return peakLevel; }
    
    // Check if beat is detected
    bool isBeatDetected() const { return beatDetected; }
    
    // Set threshold for silence detection
    void setThreshold(uint16_t threshold) { audioThreshold = threshold; }
    
    // Get current threshold
    uint16_t getThreshold() const { return audioThreshold; }
    
private:
    uint8_t bclkPin;
    uint8_t lrcPin;
    uint8_t doutPin;
    uint8_t adcPin;
    
    uint16_t audioThreshold;
    uint16_t rawLevel;
    uint16_t smoothedLevel;
    uint16_t peakLevel;
    bool newDataAvailable;
    bool beatDetected;
    
    // Beat detection
    uint16_t beatHistory[BEAT_DETECTION_WINDOW];
    uint8_t beatHistoryIndex;
    uint16_t beatAverage;
    unsigned long lastBeatTime;
    
    // Smoothing
    float smoothingFactor;
    
    void updateAudioLevel();
    void detectBeat();
    uint16_t calculateAverage(uint16_t* array, uint8_t size);
};

