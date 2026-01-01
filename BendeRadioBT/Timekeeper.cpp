/*
 * BendeRadio BT - Timekeeping Implementation
 * 
 * Uses millis() to maintain time, calibrated by user input
 */

#include "Timekeeper.h"

Timekeeper::Timekeeper()
    : calibrationOffset(0)
    , lastUpdateMillis(0)
    , currentHours(0)
    , currentMinutes(0)
    , currentTimeSeconds(0)
{
}

void Timekeeper::init(int32_t initialTimeSeconds) {
    lastUpdateMillis = millis();
    
    if (initialTimeSeconds > 0) {
        // Restore time from saved state
        currentTimeSeconds = initialTimeSeconds;
        secondsToTime(currentTimeSeconds, currentHours, currentMinutes);
    } else {
        // Start at midnight if no saved time
        currentHours = 0;
        currentMinutes = 0;
        currentTimeSeconds = 0;
    }
    
    // Apply calibration offset if set
    if (calibrationOffset != 0) {
        updateTime();
    } else {
        tick();
    }
}

void Timekeeper::tick() {
    updateTime();
}

void Timekeeper::setTime(uint8_t hours, uint8_t minutes) {
    // Set current time
    currentHours = hours % 24;
    currentMinutes = minutes % 60;
    currentTimeSeconds = secondsSinceMidnight(currentHours, currentMinutes);
    
    // Calculate offset: current time in seconds - millis()/1000
    // This offset will be added to millis()/1000 to get actual time
    unsigned long currentMillis = millis();
    int32_t currentMillisSeconds = currentMillis / 1000;
    
    // Calibration offset = desired time - current millis seconds
    calibrationOffset = currentTimeSeconds - currentMillisSeconds;
    
    // Handle wrap-around (if millis seconds > desired time, we need to add a day)
    if (calibrationOffset < 0) {
        calibrationOffset += 86400; // Add 24 hours in seconds
    }
    
    lastUpdateMillis = currentMillis;
}

void Timekeeper::setCalibrationOffset(int32_t offset) {
    calibrationOffset = offset;
    lastUpdateMillis = millis();
    updateTime();
}

void Timekeeper::updateTime() {
    unsigned long currentMillis = millis();
    
    // Calculate time: (millis() / 1000) + calibration offset
    int32_t calculatedSeconds = (currentMillis / 1000) + calibrationOffset;
    
    // Keep within 24-hour range
    calculatedSeconds = calculatedSeconds % 86400;
    if (calculatedSeconds < 0) calculatedSeconds += 86400;
    
    currentTimeSeconds = calculatedSeconds;
    secondsToTime(currentTimeSeconds, currentHours, currentMinutes);
    
    lastUpdateMillis = currentMillis;
}

int32_t Timekeeper::secondsSinceMidnight(uint8_t hours, uint8_t minutes) {
    return (int32_t)hours * 3600 + (int32_t)minutes * 60;
}

void Timekeeper::secondsToTime(int32_t seconds, uint8_t& hours, uint8_t& minutes) {
    seconds = seconds % 86400;
    if (seconds < 0) seconds += 86400;
    
    hours = seconds / 3600;
    minutes = (seconds % 3600) / 60;
}

