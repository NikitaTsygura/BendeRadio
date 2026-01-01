/*
 * BendeRadio BT - ESP32 Bluetooth Speaker Firmware
 * 
 * A Bender-style Bluetooth speaker with LED matrix displays
 * - Mouth: Real-time audio waveform visualization
 * - Eyes: Animated/pulsing during playback, time display when idle
 * - Controls: Rotary encoder with button for all device functions
 */

#include <Arduino.h>
// Bluetooth A2DP Library - Install one of the following:
// 1. BluetoothA2DPSink by pschatzmann: https://github.com/pschatzmann/ESP32-A2DP
// 2. ESP32-A2DP by earlephilhower: https://github.com/earlephilhower/ESP32-A2DP
// Uncomment the appropriate include for your library:
#include "BluetoothA2DPSink.h"  // For pschatzmann library
// #include "BluetoothA2DP.h"   // Alternative include name
#include "config.h"
#include "AudioProcessor.h"
#include "AudioGainLimiter.h"
#include "EncoderControl.h"
#include "LEDMatrixRenderer.h"
#include "Timekeeper.h"
#include "SystemState.h"

// ==================== Global Objects ====================
BluetoothA2DPSink a2dp_sink;
AudioProcessor audioProcessor;
AudioGainLimiter audioGainLimiter;  // Digital gain limiter for PC audio compatibility
EncoderControl encoderControl;
LEDMatrixRenderer ledRenderer;
Timekeeper timekeeper;
SystemState systemState;

// ==================== Audio Gain Limiting Integration ====================
// The AudioGainLimiter class is initialized and ready to use.
// Integration method depends on your Bluetooth library:
//
// For pschatzmann BluetoothA2DPSink:
//   Option 1: Modify library source (see AUDIO_GAIN_LIMITING.md)
//   Option 2: Use volume scaling as workaround (see setup() comments)
//
// For libraries with callbacks (btAudio, etc.):
//   Use i2s_audio_callback() pattern (see AUDIO_GAIN_LIMITING.md)
//
// See AUDIO_GAIN_LIMITING.md for detailed integration instructions.

// ==================== Setup ====================
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n\n=== BendeRadio BT Starting ===");

    // Initialize system state from EEPROM
    systemState.init();
    
    // Initialize LED matrices
    ledRenderer.init();
    ledRenderer.setMouthBrightness(systemState.getMouthBrightness());
    ledRenderer.setEyeBrightness(systemState.getEyeBrightness());
    
    // Initialize encoder
    encoderControl.init();
    
    // Initialize timekeeper (will use calibration offset from SystemState)
    timekeeper.setCalibrationOffset(systemState.getTimeCalibrationOffset());
    timekeeper.init(0);  // Start at midnight, calibration offset will adjust it
    
    // Initialize audio processor (for level analysis/visualization)
    audioProcessor.init(
        I2S_BCLK, 
        I2S_LRC, 
        I2S_DOUT,
        ANALYZ_PIN,
        systemState.getAudioThreshold()
    );
    
    // Initialize audio gain limiter (prevents PC audio clipping/distortion)
    // This scales down high-level audio from PCs before it reaches the DAC
    // See AUDIO_GAIN_LIMITING.md and IMPLEMENTATION_GUIDE.md for integration instructions
    audioGainLimiter.init(DEFAULT_AUDIO_GAIN_FACTOR);
    
    // Configure Bluetooth A2DP sink (speaker mode)
    // NOTE: The API depends on your Bluetooth library. Common options:
    // 
    // For BluetoothA2DPSink (pschatzmann):
    //   i2s_pin_config_t pin_config = {
    //       .bck_io_num = I2S_BCLK,
    //       .ws_io_num = I2S_LRC,
    //       .data_out_num = I2S_DOUT,
    //       .data_in_num = I2S_PIN_NO_CHANGE
    //   };
    //   a2dp_sink.set_pin_config(pin_config);
    //   a2dp_sink.start("BendeRadio BT");
    //
    // For ESP32-A2DP (earlephilhower), you may need:
    //   a2dp_sink.begin("BendeRadio BT", true, I2S_BCLK, I2S_LRC, I2S_DOUT);
    //
    // Adjust the following lines based on your library's API:
    a2dp_sink.start("BendeRadio BT");
    
    // Set initial volume (adjust method name if needed: set_volume, setVolume, etc.)
    if (systemState.isPowerOn()) {
        a2dp_sink.set_volume(systemState.getVolume());
    } else {
        a2dp_sink.set_volume(0);
    }
    
    // Initial display state
    if (systemState.isPowerOn()) {
        ledRenderer.showPowerOnState();
    } else {
        ledRenderer.showClock(timekeeper.getHours(), timekeeper.getMinutes());
    }
    
    Serial.println("=== BendeRadio BT Ready ===");
}

// ==================== Main Loop ====================
void loop() {
    // Non-blocking tick functions
    encoderControl.tick();
    audioProcessor.tick();
    timekeeper.tick();
    ledRenderer.tick();
    
    // Handle encoder events
    handleEncoderEvents();
    
    // Update display based on system state
    updateDisplay();
    
    // Small delay to prevent watchdog issues
    delay(1);
}

// ==================== Event Handlers ====================
void handleEncoderEvents() {
    EncoderEvent event = encoderControl.getEvent();
    
    if (event.type == EVENT_NONE) return;
    
    switch (event.type) {
        case EVENT_ROTATE: {
            // Volume control (only when powered on)
            if (systemState.isPowerOn() && !encoderControl.isHolding()) {
                int newVol = systemState.getVolume() + event.value;
                newVol = constrain(newVol, 0, 21);
                systemState.setVolume(newVol);
                a2dp_sink.set_volume(newVol);
                ledRenderer.showVolume(newVol);
            }
            break;
        }
            
        case EVENT_HOLD_ROTATE: {
            // Mode/station change
            if (systemState.isPowerOn()) {
                int newMode = systemState.getAudioMode() + event.value;
                newMode = constrain(newMode, 0, MAX_AUDIO_MODES - 1);
                systemState.setAudioMode(newMode);
                ledRenderer.showMode(newMode);
            }
            break;
        }
            
        case EVENT_SINGLE_CLICK:
            // Power toggle
            systemState.togglePower();
            if (systemState.isPowerOn()) {
                a2dp_sink.set_volume(systemState.getVolume());
                ledRenderer.showPowerOnState();
            } else {
                a2dp_sink.set_volume(0);
                ledRenderer.showClock(timekeeper.getHours(), timekeeper.getMinutes());
            }
            systemState.save();
            break;
            
        case EVENT_DOUBLE_CLICK:
            // Change visual effect
            if (systemState.isPowerOn()) {
                int newEffect = systemState.getVisualEffect() + 1;
                if (newEffect >= MAX_VISUAL_EFFECTS) newEffect = 0;
                systemState.setVisualEffect(newEffect);
                ledRenderer.showEffectChange(newEffect);
            }
            break;
            
        case EVENT_TRIPLE_CLICK:
            // Enter calibration mode
            enterCalibrationMode();
            break;
            
        case EVENT_CLICK_HOLD_ROTATE: {
            // Mouth brightness
            int newBrightness = systemState.getMouthBrightness() + event.value;
            newBrightness = constrain(newBrightness, 0, 15);
            systemState.setMouthBrightness(newBrightness);
            ledRenderer.setMouthBrightness(newBrightness);
            ledRenderer.showMouthBrightness(newBrightness);
            systemState.save();
            break;
        }
            
        case EVENT_DOUBLE_CLICK_HOLD_ROTATE: {
            // Eye brightness
            int newEyeBrightness = systemState.getEyeBrightness() + event.value;
            newEyeBrightness = constrain(newEyeBrightness, 0, 15);
            systemState.setEyeBrightness(newEyeBrightness);
            ledRenderer.setEyeBrightness(newEyeBrightness);
            ledRenderer.showEyeBrightness(newEyeBrightness);
            systemState.save();
            break;
        }
            
        default:
            break;
    }
}

void enterCalibrationMode() {
    Serial.println("Entering calibration mode...");
    
    // Show calibration menu on display
    ledRenderer.showCalibrationMenu();
    
    // Wait for user selection via encoder
    int calibrationType = 0; // 0=Silence, 1=Time, 2=Sensitivity
    unsigned long menuStartTime = millis();
    
    while (millis() - menuStartTime < 10000) { // 10 second timeout
        encoderControl.tick();
        EncoderEvent event = encoderControl.getEvent();
        
        if (event.type == EVENT_ROTATE) {
            calibrationType = (calibrationType + event.value + 3) % 3;
            ledRenderer.showCalibrationSelection(calibrationType);
        } else if (event.type == EVENT_SINGLE_CLICK) {
            // Execute selected calibration
            performCalibration(calibrationType);
            break;
        }
    }
    
    // Return to normal display
    if (systemState.isPowerOn()) {
        ledRenderer.showPowerOnState();
    } else {
        ledRenderer.showClock(timekeeper.getHours(), timekeeper.getMinutes());
    }
}

void performCalibration(int type) {
    switch (type) {
        case 0: // Silence threshold calibration
            calibrateSilenceThreshold();
            break;
        case 1: // Time calibration
            calibrateTime();
            break;
        case 2: // Sensitivity calibration
            calibrateSensitivity();
            break;
    }
}

void calibrateSilenceThreshold() {
    Serial.println("Calibrating silence threshold...");
    ledRenderer.showCalibrationMessage("SIL", 0);
    
    // Sample audio for 3 seconds to find baseline
    unsigned long startTime = millis();
    uint16_t minLevel = 65535;
    uint16_t maxLevel = 0;
    
    while (millis() - startTime < 3000) {
        audioProcessor.tick();
        uint16_t level = audioProcessor.getRawLevel();
        if (level < minLevel) minLevel = level;
        if (level > maxLevel) maxLevel = level;
        delay(10);
    }
    
    // Set threshold slightly above baseline
    uint16_t threshold = minLevel + (maxLevel - minLevel) * 0.3;
    systemState.setAudioThreshold(threshold);
    audioProcessor.setThreshold(threshold);
    systemState.save();
    
    ledRenderer.showCalibrationComplete();
    delay(1000);
}

void calibrateTime() {
    Serial.println("Calibrating time...");
    
    // Get current time from user
    int hours = timekeeper.getHours();
    int minutes = timekeeper.getMinutes();
    bool editingHours = true;
    unsigned long lastActivity = millis();
    
    ledRenderer.showTimeEdit(hours, minutes, editingHours);
    
    while (millis() - lastActivity < 10000) {
        encoderControl.tick();
        EncoderEvent event = encoderControl.getEvent();
        
        if (event.type == EVENT_ROTATE) {
            lastActivity = millis();
            if (editingHours) {
                hours = (hours + event.value + 24) % 24;
            } else {
                minutes = (minutes + event.value + 60) % 60;
            }
            ledRenderer.showTimeEdit(hours, minutes, editingHours);
        } else if (event.type == EVENT_SINGLE_CLICK) {
            lastActivity = millis();
            editingHours = !editingHours;
            ledRenderer.showTimeEdit(hours, minutes, editingHours);
        } else if (event.type == EVENT_DOUBLE_CLICK) {
            // Confirm and set time
            timekeeper.setTime(hours, minutes);
            systemState.setTimeCalibration(timekeeper.getCalibrationOffset());
            systemState.save();
            ledRenderer.showCalibrationComplete();
            delay(1000);
            return;
        }
    }
}

void calibrateSensitivity() {
    Serial.println("Calibrating sensitivity...");
    ledRenderer.showCalibrationMessage("SEN", 0);
    
    if (!systemState.isPowerOn()) {
        ledRenderer.showCalibrationMessage("PLAY", 1);
        delay(2000);
        return;
    }
    
    // Sample audio levels during playback for 5 seconds
    unsigned long startTime = millis();
    uint16_t maxPeak = 0;
    
    while (millis() - startTime < 5000) {
        audioProcessor.tick();
        uint16_t peak = audioProcessor.getPeakLevel();
        if (peak > maxPeak) maxPeak = peak;
        
        // Show visual feedback
        uint8_t vol = audioProcessor.getAudioLevel();
        ledRenderer.drawMouthWaveform(vol);
        delay(10);
    }
    
    // Adjust sensitivity based on peak
    uint16_t newThreshold = maxPeak * 0.4; // 40% of peak
    systemState.setAudioThreshold(newThreshold);
    audioProcessor.setThreshold(newThreshold);
    systemState.save();
    
    ledRenderer.showCalibrationComplete();
    delay(1000);
}

// ==================== Display Update ====================
void updateDisplay() {
    if (!systemState.isPowerOn()) {
        // Show clock on eyes when powered off
        static unsigned long lastClockUpdate = 0;
        if (millis() - lastClockUpdate > 1000) { // Update every second
            lastClockUpdate = millis();
            ledRenderer.showClock(timekeeper.getHours(), timekeeper.getMinutes());
        }
    } else {
        // Show audio visualization when powered on
        if (audioProcessor.hasNewData()) {
            uint8_t audioLevel = audioProcessor.getAudioLevel();
            bool isBeat = audioProcessor.isBeatDetected();
            
            // Update mouth with waveform
            ledRenderer.drawMouthWaveform(audioLevel);
            
            // Update eyes with animation/pulse
            ledRenderer.updateEyeAnimation(audioLevel, isBeat, systemState.getVisualEffect());
        }
    }
}

