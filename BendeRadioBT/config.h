/*
 * BendeRadio BT - Hardware Configuration
 * 
 * Pin definitions for ESP32 38-pin Dev Board
 */

#pragma once

// ==================== I2S Audio Pins (External DAC) ====================
#define I2S_BCLK      27    // Bit Clock (BCLK)
#define I2S_LRC       26    // Left/Right Clock (LRC/LRCLK)
#define I2S_DOUT      25    // Data Output (DIN)

// ==================== LED Matrix Pins (via Level Shifter) ====================
// MAX7219 LED Matrix Driver
#define MTRX_CS       22    // Chip Select
#define MTRX_DAT      23    // Data (MOSI)
#define MTRX_CLK      21    // Clock (SCK)

// Matrix dimensions
#define MOUTH_WIDTH   24    // 3 matrices × 8 columns = 24 columns for mouth
#define EYE_WIDTH     8     // 1 matrix × 8 columns = 8 columns per eye
#define MATRIX_HEIGHT 8     // 8 rows

// Matrix layout: [Mouth 24 cols] [Eye 1: 8 cols] [Eye 2: 8 cols] = 40 total columns
#define TOTAL_MATRICES 5    // 3 for mouth + 2 for eyes

// ==================== Rotary Encoder Pins ====================
#define ENC_S1        19    // Encoder Phase A
#define ENC_S2        18    // Encoder Phase B
#define ENC_BTN       5     // Encoder Push Button

// ==================== Audio Analysis Pin ====================
#define ANALYZ_PIN    34    // ADC input for audio level analysis (from amplifier output)

// ==================== System Constants ====================
#define MAX_VOLUME           21    // Maximum volume level (0-21)
#define MIN_VOLUME           0     // Minimum volume level
#define MAX_BRIGHTNESS       15    // Maximum LED brightness (0-15)
#define MIN_BRIGHTNESS       0     // Minimum LED brightness
#define MAX_AUDIO_MODES      4     // Number of audio modes/stations
#define MAX_VISUAL_EFFECTS   3     // Number of visual effect modes

// ==================== Timing Constants (milliseconds) ====================
#define CLOCK_UPDATE_INTERVAL    1000    // Clock refresh rate
#define AUDIO_UPDATE_INTERVAL    50      // Audio visualization update rate
#define EYE_ANIMATION_INTERVAL   100     // Eye animation update rate
#define CALIBRATION_TIMEOUT      10000   // Calibration menu timeout

// ==================== Audio Analysis Constants ====================
#define DEFAULT_AUDIO_THRESHOLD  512     // Default ADC threshold for silence detection
#define BEAT_DETECTION_WINDOW    20      // Samples for beat detection
#define AUDIO_SMOOTHING_FACTOR   0.7f    // Smoothing factor for audio levels (0.0-1.0)

// ==================== Audio Gain Limiting Constants ====================
// Digital gain reduction to prevent DAC/amplifier overload from high PC audio levels
// Range: 0.0 to 1.0 (e.g., 0.3 = 30% of full scale)
// Lower values = more gain reduction = safer but quieter
// Recommended: 0.2-0.4 for PC compatibility (smartphones work fine at higher values)
#define DEFAULT_AUDIO_GAIN_FACTOR  0.3f  // 30% - prevents PC audio clipping
#define MIN_DAC_VALUE            -32768  // Minimum 16-bit signed integer value

// ==================== EEPROM Addresses ====================
#define EEPROM_SIZE             512
#define EEPROM_MAGIC_VALUE      0xBEBE   // Magic value to verify EEPROM data
#define EEPROM_ADDR_MAGIC       0
#define EEPROM_ADDR_STATE       2
#define EEPROM_ADDR_VOLUME      3
#define EEPROM_ADDR_MOUTH_BRIGHT 4
#define EEPROM_ADDR_EYE_BRIGHT  5
#define EEPROM_ADDR_THRESHOLD   6
#define EEPROM_ADDR_MODE        8
#define EEPROM_ADDR_EFFECT      9
#define EEPROM_ADDR_TIME_OFFSET 10

