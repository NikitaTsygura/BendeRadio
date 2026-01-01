/*
 * BendeRadio BT - LED Matrix Renderer Module
 * 
 * Handles all LED matrix display operations:
 * - Mouth: Real-time audio waveform visualization
 * - Eyes: Animated/pulsing during playback, time display when idle
 */

#pragma once

#include <Arduino.h>
#include <GyverMAX7219.h>
#include <GyverGFX.h>
#include "config.h"

class LEDMatrixRenderer {
public:
    LEDMatrixRenderer();
    
    // Initialization (call once in setup)
    void init();
    
    // Main update function (call in loop)
    void tick();
    
    // Brightness control
    void setMouthBrightness(uint8_t brightness);
    void setEyeBrightness(uint8_t brightness);
    
    // Display functions
    void drawMouthWaveform(uint8_t audioLevel);  // Draw waveform on mouth (0-100)
    void updateEyeAnimation(uint8_t audioLevel, bool isBeat, uint8_t effectMode);
    void showClock(uint8_t hours, uint8_t minutes);
    void showPowerOnState();
    
    // UI display functions
    void showVolume(uint8_t volume);
    void showMode(uint8_t mode);
    void showEffectChange(uint8_t effect);
    void showMouthBrightness(uint8_t brightness);
    void showEyeBrightness(uint8_t brightness);
    void showCalibrationMenu();
    void showCalibrationSelection(uint8_t selection);
    void showCalibrationMessage(const char* msg, uint8_t line);
    void showCalibrationComplete();
    void showTimeEdit(uint8_t hours, uint8_t minutes, bool editingHours);
    
private:
    MAX7219<TOTAL_MATRICES, 1, MTRX_CS, MTRX_DAT, MTRX_CLK> matrix;
    
    uint8_t mouthBrightness;
    uint8_t eyeBrightness;
    
    // Waveform history for smooth visualization
    uint8_t waveformHistory[MOUTH_WIDTH];
    uint8_t waveformIndex;
    
    // Eye animation state
    unsigned long lastEyeUpdate;
    uint8_t eyeAnimationFrame;
    int8_t eyePupilX[2];
    int8_t eyePupilY[2];
    
    // Helper functions
    void updateBrightness();
    void drawEye(uint8_t eyeIndex, uint8_t xOffset);
    void drawEyePupil(uint8_t eyeIndex, uint8_t xOffset, int8_t pupilX, int8_t pupilY, uint8_t size);
    void clearMouth();
    void clearEyes();
    
    // Waveform drawing modes
    void drawWaveformMode0(uint8_t audioLevel);  // Vertical bars
    void drawWaveformMode1(uint8_t audioLevel);  // Centered bars
    void drawWaveformMode2(uint8_t audioLevel);  // Frequency-like
    
    // Eye animation modes
    void animateEyesMode0(uint8_t audioLevel, bool isBeat);  // Pulsing
    void animateEyesMode1(uint8_t audioLevel, bool isBeat);  // Wandering pupils
    void animateEyesMode2(uint8_t audioLevel, bool isBeat);  // Beat-reactive
};

