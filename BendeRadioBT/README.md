# BendeRadio BT - ESP32 Bluetooth Speaker Firmware

A Bender-style Bluetooth speaker firmware for ESP32 with LED matrix displays.

## Features

- **Bluetooth Speaker**: Receives audio via Bluetooth A2DP (Advanced Audio Distribution Profile)
- **Audio Visualization**: Real-time waveform display on mouth LED matrix during playback
- **Eye Animations**: Eyes animate/pulse with music beat and audio levels
- **Clock Display**: Eyes show current time (HH:MM) when powered off
- **Rotary Encoder Controls**: Single encoder with button for all device functions
- **Timekeeping**: Software-based timekeeping with user calibration (no RTC required)
- **Calibration System**: Adjust silence threshold, time, and animation sensitivity

## Hardware Requirements

- ESP32 (38-pin Dev Board)
- Rotary encoder with push button
- External DAC (for I2S audio output)
- Audio amplifier
- Speakers
- Level shifter (logic level converter for LED matrices)
- LED matrix modules:
  - 3× MAX7219 matrices (white, common cathode) for mouth (24 columns total)
  - 2× MAX7219 matrices (yellow, common cathode) for eyes (8 columns each)
- Power supply: 5V, 1.5-2A

## Pin Configuration

See `config.h` for pin definitions:

- **I2S Audio**: BCLK=27, LRC=26, DOUT=25
- **LED Matrices**: CS=22, DAT=23, CLK=21
- **Rotary Encoder**: S1=19, S2=18, BTN=5
- **Audio Analysis**: ADC pin 34 (connected to amplifier output)

## Required Libraries

Install the following libraries in Arduino IDE:

1. **EncButton** - For rotary encoder handling
   - Library Manager: Search "EncButton" by Alex Gyver
   - Or: https://github.com/GyverLibs/EncButton

2. **GyverMAX7219** - For LED matrix control
   - Library Manager: Search "GyverMAX7219" by Alex Gyver
   - Or: https://github.com/GyverLibs/GyverMAX7219

3. **GyverGFX** - Graphics library (dependency of GyverMAX7219)
   - Library Manager: Search "GyverGFX" by Alex Gyver
   - Or: https://github.com/GyverLibs/GyverGFX

4. **FastLED** - LED control (for some utility functions)
   - Library Manager: Search "FastLED"
   - Or: https://github.com/FastLED/FastLED

5. **Bluetooth A2DP Library** - Choose one:
   - **Option 1**: `ESP32-A2DP` by pschatzmann (recommended)
     - Library Manager: Search "ESP32-A2DP" by pschatzmann
     - GitHub: https://github.com/pschatzmann/ESP32-A2DP
     - Class name: `BluetoothA2DPSink`
   - **Option 2**: `ESP32-A2DP` by earlephilhower
     - Library Manager: Search "ESP32-A2DP" by earlephilhower
     - GitHub: https://github.com/earlephilhower/ESP32-A2DP
     - May have different API

**Important**: After installing the Bluetooth library, you may need to adjust the initialization code in `BendeRadioBT.ino` (around line 70-85) to match your library's API. See comments in the code for guidance.

## Controls

### Rotary Encoder Actions

- **Rotate** → Adjust volume (0-21)
- **Hold + Rotate** → Change audio mode/station (0-3)
- **Single Click** → Power ON/OFF
- **Double Click** → Change visual effect mode (0-2)
- **Triple Click** → Enter calibration mode
- **Click + Hold + Rotate** → Adjust mouth LED brightness (0-15)
- **Double Click + Hold + Rotate** → Adjust eye LED brightness (0-15)

### Calibration Mode

When triple-clicking, you enter calibration mode with three options:

1. **Silence Threshold** (SIL): Calibrate audio silence detection
   - Device samples audio for 3 seconds
   - Sets threshold slightly above baseline noise

2. **Time Calibration** (TIM): Set current time
   - Rotate to adjust hours/minutes
   - Single click to switch between hours and minutes
   - Double click to confirm and save

3. **Sensitivity Calibration** (SEN): Adjust animation sensitivity
   - Requires music to be playing
   - Samples audio levels for 5 seconds
   - Adjusts sensitivity based on peak levels

## Project Structure

```
BendeRadioBT/
├── BendeRadioBT.ino       # Main firmware file
├── config.h                # Hardware pin definitions and constants
├── SystemState.h           # System state management header
├── SystemState.cpp         # System state implementation (EEPROM persistence)
├── AudioProcessor.h        # Audio level analysis header
├── AudioProcessor.cpp      # Audio processing implementation
├── EncoderControl.h        # Rotary encoder control header
├── EncoderControl.cpp      # Encoder state machine implementation
├── LEDMatrixRenderer.h     # LED matrix rendering header
├── LEDMatrixRenderer.cpp   # Display functions implementation
├── Timekeeper.h            # Timekeeping module header
├── Timekeeper.cpp          # Timekeeping implementation
└── README.md               # This file
```

## How Audio Visualization Works

### Mouth Waveform Display

The mouth LED matrix displays a real-time audio waveform by:

1. **Audio Sampling**: The `AudioProcessor` reads audio levels from an ADC pin connected to the amplifier output
2. **Level Conversion**: Raw ADC values are converted to a 0-100 scale, subtracting the silence threshold
3. **Waveform History**: Maintains a circular buffer of recent audio levels (24 samples = 24 columns)
4. **Display Modes**: Three visualization modes:
   - **Mode 0**: Vertical bars (each column = one sample)
   - **Mode 1**: Centered bars (waveform centered vertically)
   - **Mode 2**: Smooth frequency-like curve

### Eye Animations

The eye LED matrices animate based on audio:

1. **Beat Detection**: `AudioProcessor` detects beats by comparing current level to a moving average
2. **Animation Modes**: Three eye animation modes:
   - **Mode 0**: Pulsing eyes (pupil size varies with audio level, larger on beats)
   - **Mode 1**: Wandering pupils (pupils move randomly, react to beats)
   - **Mode 2**: Beat-reactive (eyes widen significantly on beats)
3. **Update Rate**: Eyes update every 100ms for smooth animation

### When Powered Off

When the device is powered off (no audio playback):

- Eyes display current time in HH:MM format
- Time updates every second
- Time is maintained using `millis()` with user calibration offset

## Compilation and Upload

1. Install Arduino IDE with ESP32 support
2. Install required libraries (see above)
3. Open `BendeRadioBT/BendeRadioBT.ino` in Arduino IDE
4. Select your ESP32 board in Tools → Board
5. Configure port in Tools → Port
6. Click Upload

## Configuration

Most settings can be adjusted in `config.h`:

- Pin definitions
- LED matrix dimensions
- Volume and brightness ranges
- Timing constants
- Audio analysis parameters

## Notes

- The firmware uses non-blocking code (no `delay()` in main loops)
- All state is saved to EEPROM automatically
- Time calibration is required on first use (or after power loss)
- The Bluetooth library handles I2S configuration automatically
- Audio analysis uses ADC pin 34 (ensure proper voltage level from amplifier)

## Troubleshooting

**Bluetooth not working:**
- Check that the correct Bluetooth library is installed
- Verify I2S pins are correctly connected
- Ensure ESP32 has Bluetooth enabled (some boards may require configuration)

**LED matrices not working:**
- Check level shifter connections (ESP32 uses 3.3V, matrices use 5V)
- Verify pin connections (CS, DAT, CLK)
- Check power supply (LED matrices can draw significant current)

**Audio visualization not working:**
- Verify ADC pin 34 is connected to amplifier output
- Adjust audio threshold via calibration mode
- Check that audio levels are within ADC range (0-3.3V)

**Time not keeping:**
- Perform time calibration (triple-click → select TIM)
- Note: Time resets on power loss if not calibrated (this is normal)

## License

This firmware is provided as-is for educational and personal use.

