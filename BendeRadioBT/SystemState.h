/*
 * BendeRadio BT - System State Management
 * 
 * Manages device state, settings, and EEPROM persistence
 */

#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"

class SystemState {
public:
    SystemState();
    
    // Initialization
    void init();
    void save();
    void load();
    
    // Power state
    bool isPowerOn() const { return powerOn; }
    void setPowerOn(bool state) { powerOn = state; }
    void togglePower() { powerOn = !powerOn; }
    
    // Volume (0-21)
    uint8_t getVolume() const { return volume; }
    void setVolume(uint8_t vol) { volume = constrain(vol, 0, MAX_VOLUME); }
    
    // Brightness (0-15)
    uint8_t getMouthBrightness() const { return mouthBrightness; }
    void setMouthBrightness(uint8_t bright) { mouthBrightness = constrain(bright, 0, MAX_BRIGHTNESS); }
    
    uint8_t getEyeBrightness() const { return eyeBrightness; }
    void setEyeBrightness(uint8_t bright) { eyeBrightness = constrain(bright, 0, MAX_BRIGHTNESS); }
    
    // Audio settings
    uint16_t getAudioThreshold() const { return audioThreshold; }
    void setAudioThreshold(uint16_t threshold) { audioThreshold = threshold; }
    
    uint8_t getAudioMode() const { return audioMode; }
    void setAudioMode(uint8_t mode) { audioMode = constrain(mode, 0, MAX_AUDIO_MODES - 1); }
    
    uint8_t getVisualEffect() const { return visualEffect; }
    void setVisualEffect(uint8_t effect) { visualEffect = constrain(effect, 0, MAX_VISUAL_EFFECTS - 1); }
    
    // Time calibration
    int32_t getCurrentTime() const { return currentTimeSeconds; }
    void setCurrentTime(int32_t seconds) { currentTimeSeconds = seconds; }
    void setTimeCalibration(int32_t offset) { timeCalibrationOffset = offset; }
    int32_t getTimeCalibrationOffset() const { return timeCalibrationOffset; }
    
private:
    // System state
    bool powerOn;
    uint8_t volume;
    uint8_t mouthBrightness;
    uint8_t eyeBrightness;
    uint16_t audioThreshold;
    uint8_t audioMode;
    uint8_t visualEffect;
    
    // Timekeeping
    int32_t currentTimeSeconds;      // Current time in seconds since calibration
    int32_t timeCalibrationOffset;   // Offset from millis() to actual time
    unsigned long lastTimeUpdate;    // Last millis() when time was updated
    
    // EEPROM management
    bool isValidEEPROM();
    void writeDefaults();
};

