/*
 * BendeRadio BT - Audio Processing Implementation
 * 
 * Note: This module reads audio levels from an ADC pin connected to the amplifier output.
 * The actual Bluetooth audio playback is handled by the BluetoothA2DP library in the main sketch.
 */

#include "AudioProcessor.h"

AudioProcessor::AudioProcessor()
    : bclkPin(0)
    , lrcPin(0)
    , doutPin(0)
    , adcPin(0)
    , audioThreshold(DEFAULT_AUDIO_THRESHOLD)
    , rawLevel(0)
    , smoothedLevel(0)
    , peakLevel(0)
    , newDataAvailable(false)
    , beatDetected(false)
    , beatHistoryIndex(0)
    , beatAverage(0)
    , lastBeatTime(0)
    , smoothingFactor(AUDIO_SMOOTHING_FACTOR)
{
    // Initialize beat detection history
    for (uint8_t i = 0; i < BEAT_DETECTION_WINDOW; i++) {
        beatHistory[i] = 0;
    }
}

void AudioProcessor::init(uint8_t bclk, uint8_t lrc, uint8_t dout, uint8_t adc, uint16_t threshold) {
    bclkPin = bclk;
    lrcPin = lrc;
    doutPin = dout;
    adcPin = adc;
    audioThreshold = threshold;
    
    // Configure ADC pin
    pinMode(adcPin, INPUT);
    
    // Note: I2S pins are configured by the BluetoothA2DP library
    // We only need the ADC pin for level analysis
    
    rawLevel = 0;
    smoothedLevel = 0;
    peakLevel = 0;
    newDataAvailable = false;
    beatDetected = false;
}

void AudioProcessor::tick() {
    updateAudioLevel();
    detectBeat();
}

void AudioProcessor::updateAudioLevel() {
    // Read raw ADC value (0-4095 for ESP32 ADC)
    uint16_t newRaw = analogRead(adcPin);
    
    // Apply smoothing to reduce noise
    rawLevel = (uint16_t)(smoothingFactor * rawLevel + (1.0f - smoothingFactor) * newRaw);
    
    // Convert to 0-100 range, subtracting threshold
    int32_t adjustedLevel = (int32_t)rawLevel - (int32_t)audioThreshold;
    if (adjustedLevel < 0) adjustedLevel = 0;
    
    // Scale to 0-100 (assuming max ADC value of 4095, but adjust based on your amplifier output)
    // This scaling factor may need calibration based on your hardware
    uint16_t scaledLevel = map(adjustedLevel, 0, 4095 - audioThreshold, 0, 100);
    scaledLevel = constrain(scaledLevel, 0, 100);
    
    smoothedLevel = scaledLevel;
    
    // Track peak level for calibration
    if (rawLevel > peakLevel) {
        peakLevel = rawLevel;
    } else {
        // Decay peak slowly
        peakLevel = peakLevel * 0.99f;
    }
    
    newDataAvailable = true;
}

void AudioProcessor::detectBeat() {
    // Update beat detection history
    beatHistory[beatHistoryIndex] = smoothedLevel;
    beatHistoryIndex = (beatHistoryIndex + 1) % BEAT_DETECTION_WINDOW;
    
    // Calculate average
    beatAverage = calculateAverage(beatHistory, BEAT_DETECTION_WINDOW);
    
    // Detect beat: current level significantly above average
    // Beat detected if level is 1.5x the average and above threshold
    uint16_t beatThreshold = beatAverage + (beatAverage / 2);
    
    if (smoothedLevel > beatThreshold && smoothedLevel > 20) {
        // Prevent beat detection too frequently (min 100ms between beats)
        unsigned long now = millis();
        if (now - lastBeatTime > 100) {
            beatDetected = true;
            lastBeatTime = now;
        } else {
            beatDetected = false;
        }
    } else {
        beatDetected = false;
    }
}

uint16_t AudioProcessor::calculateAverage(uint16_t* array, uint8_t size) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum / size;
}

