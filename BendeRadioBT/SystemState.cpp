/*
 * BendeRadio BT - System State Implementation
 */

#include "SystemState.h"

SystemState::SystemState() 
    : powerOn(false)
    , volume(10)
    , mouthBrightness(8)
    , eyeBrightness(10)
    , audioThreshold(DEFAULT_AUDIO_THRESHOLD)
    , audioMode(0)
    , visualEffect(0)
    , currentTimeSeconds(0)
    , timeCalibrationOffset(0)
    , lastTimeUpdate(0)
{
}

void SystemState::init() {
    EEPROM.begin(EEPROM_SIZE);
    delay(100);
    
    if (isValidEEPROM()) {
        load();
    } else {
        writeDefaults();
        save();
    }
    
    // Initialize time calibration
    lastTimeUpdate = millis();
    if (timeCalibrationOffset != 0) {
        currentTimeSeconds = (millis() / 1000) + timeCalibrationOffset;
    }
}

void SystemState::save() {
    uint16_t magic = EEPROM_MAGIC_VALUE;
    uint8_t stateByte = powerOn ? 1 : 0;
    
    // Write magic value
    EEPROM.put(EEPROM_ADDR_MAGIC, magic);
    
    // Write state variables
    EEPROM.put(EEPROM_ADDR_STATE, stateByte);
    EEPROM.put(EEPROM_ADDR_VOLUME, volume);
    EEPROM.put(EEPROM_ADDR_MOUTH_BRIGHT, mouthBrightness);
    EEPROM.put(EEPROM_ADDR_EYE_BRIGHT, eyeBrightness);
    EEPROM.put(EEPROM_ADDR_THRESHOLD, audioThreshold);
    EEPROM.put(EEPROM_ADDR_MODE, audioMode);
    EEPROM.put(EEPROM_ADDR_EFFECT, visualEffect);
    EEPROM.put(EEPROM_ADDR_TIME_OFFSET, timeCalibrationOffset);
    
    EEPROM.commit();
}

void SystemState::load() {
    uint16_t magic;
    uint8_t stateByte;
    
    EEPROM.get(EEPROM_ADDR_MAGIC, magic);
    EEPROM.get(EEPROM_ADDR_STATE, stateByte);
    powerOn = (stateByte != 0);
    EEPROM.get(EEPROM_ADDR_VOLUME, volume);
    EEPROM.get(EEPROM_ADDR_MOUTH_BRIGHT, mouthBrightness);
    EEPROM.get(EEPROM_ADDR_EYE_BRIGHT, eyeBrightness);
    EEPROM.get(EEPROM_ADDR_THRESHOLD, audioThreshold);
    EEPROM.get(EEPROM_ADDR_MODE, audioMode);
    EEPROM.get(EEPROM_ADDR_EFFECT, visualEffect);
    EEPROM.get(EEPROM_ADDR_TIME_OFFSET, timeCalibrationOffset);
    
    // Validate loaded values
    volume = constrain(volume, 0, MAX_VOLUME);
    mouthBrightness = constrain(mouthBrightness, 0, MAX_BRIGHTNESS);
    eyeBrightness = constrain(eyeBrightness, 0, MAX_BRIGHTNESS);
    audioMode = constrain(audioMode, 0, MAX_AUDIO_MODES - 1);
    visualEffect = constrain(visualEffect, 0, MAX_VISUAL_EFFECTS - 1);
    
    if (audioThreshold == 0) audioThreshold = DEFAULT_AUDIO_THRESHOLD;
}

bool SystemState::isValidEEPROM() {
    uint16_t magic;
    EEPROM.get(EEPROM_ADDR_MAGIC, magic);
    return (magic == EEPROM_MAGIC_VALUE);
}

void SystemState::writeDefaults() {
    powerOn = false;
    volume = 10;
    mouthBrightness = 8;
    eyeBrightness = 10;
    audioThreshold = DEFAULT_AUDIO_THRESHOLD;
    audioMode = 0;
    visualEffect = 0;
    timeCalibrationOffset = 0;
}

