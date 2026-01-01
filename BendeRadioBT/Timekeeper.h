/*
 * BendeRadio BT - Timekeeping Module
 * 
 * Maintains time using millis() with user calibration
 * No RTC module required - time is kept in software
 */

#pragma once

#include <Arduino.h>

class Timekeeper {
public:
    Timekeeper();
    
    // Initialization (call once in setup)
    void init(int32_t initialTimeSeconds);
    
    // Main update function (call in loop)
    void tick();
    
    // Set time (hours: 0-23, minutes: 0-59)
    void setTime(uint8_t hours, uint8_t minutes);
    
    // Get current time
    uint8_t getHours() const { return currentHours; }
    uint8_t getMinutes() const { return currentMinutes; }
    
    // Get calibration offset (for EEPROM storage)
    int32_t getCalibrationOffset() const { return calibrationOffset; }
    
    // Set calibration offset (from EEPROM)
    void setCalibrationOffset(int32_t offset);
    
private:
    int32_t calibrationOffset;  // Offset in seconds from millis() to actual time
    unsigned long lastUpdateMillis;
    uint8_t currentHours;
    uint8_t currentMinutes;
    int32_t currentTimeSeconds;  // Time in seconds since midnight
    
    void updateTime();
    int32_t secondsSinceMidnight(uint8_t hours, uint8_t minutes);
    void secondsToTime(int32_t seconds, uint8_t& hours, uint8_t& minutes);
};

