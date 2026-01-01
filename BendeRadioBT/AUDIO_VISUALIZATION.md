# Audio Visualization Explanation

This document explains how audio levels are converted into mouth and eye animations in the BendeRadio BT firmware.

## Overview

The audio visualization system consists of three main components:

1. **Audio Level Detection**: Reading audio signal from amplifier output via ADC
2. **Level Processing**: Converting raw ADC values to normalized levels with beat detection
3. **Visualization**: Mapping audio levels to LED matrix displays

## Audio Level Detection

### Hardware Connection

The audio signal path:
```
Bluetooth Audio → DAC (via I2S) → Amplifier → Speaker
                                    ↓
                              ADC Pin 34 (ESP32)
```

The ESP32 reads the audio signal from the amplifier output using ADC pin 34. This provides a voltage proportional to the audio amplitude.

### Signal Processing (`AudioProcessor.cpp`)

1. **Raw ADC Reading**:
   ```cpp
   uint16_t newRaw = analogRead(adcPin);  // 0-4095 (ESP32 ADC)
   ```

2. **Smoothing**:
   - Apply exponential smoothing to reduce noise and jitter
   - Formula: `smoothed = α × old + (1-α) × new`
   - α = 0.7 (smoothing factor)

3. **Threshold Subtraction**:
   - Subtract silence threshold to eliminate background noise
   - `adjusted = raw - threshold`
   - Negative values clamped to 0

4. **Normalization**:
   - Scale to 0-100 range for easy mapping to display
   - `level = map(adjusted, 0, max_range, 0, 100)`

## Mouth Waveform Display

### Data Flow

```
Audio Level (0-100) → Waveform History Buffer → LED Matrix Columns
```

### Implementation

1. **Circular Buffer**:
   - Maintains 24 samples (one per matrix column)
   - Newest sample overwrites oldest
   - Creates scrolling waveform effect

2. **Column Mapping**:
   - Each column (x position) represents one sample from history
   - Height of column represents audio level
   - Formula: `height = map(level, 0, 100, 0, 8)` pixels

3. **Display Modes**:

   **Mode 0 - Vertical Bars**:
   - Direct mapping: level → column height
   - Bottom-aligned bars
   - Simple and responsive

   **Mode 1 - Centered Bars**:
   - Bars centered vertically
   - Symmetrical waveform
   - Better for visualizing frequency content

   **Mode 2 - Frequency Curve**:
   - Smooth line connecting sample points
   - More organic, wave-like appearance
   - Interpolates between samples

### Code Example

```cpp
void LEDMatrixRenderer::drawWaveformMode0(uint8_t audioLevel) {
    // Store in history
    waveformHistory[waveformIndex] = audioLevel;
    waveformIndex = (waveformIndex + 1) % MOUTH_WIDTH;
    
    // Draw each column
    for (uint8_t x = 0; x < MOUTH_WIDTH; x++) {
        uint8_t level = waveformHistory[(waveformIndex + x) % MOUTH_WIDTH];
        uint8_t height = map(level, 0, 100, 0, MATRIX_HEIGHT);
        
        // Draw vertical bar
        for (uint8_t y = 0; y <= height; y++) {
            matrix.dot(x, MATRIX_HEIGHT - 1 - y);
        }
    }
}
```

## Eye Animations

### Beat Detection

The system detects beats by comparing current audio level to a moving average:

1. **Moving Average**:
   - Maintains window of last 20 samples
   - Calculates average level over window

2. **Beat Threshold**:
   - Beat detected when: `current > average × 1.5`
   - Must also be above minimum threshold (20%)
   - Minimum 100ms between beat detections (prevents multiple triggers)

3. **Code**:
   ```cpp
   uint16_t beatThreshold = beatAverage + (beatAverage / 2);
   if (smoothedLevel > beatThreshold && smoothedLevel > 20) {
       beatDetected = true;
   }
   ```

### Animation Modes

**Mode 0 - Pulsing**:
- Pupil size varies with audio level (2-6 pixels)
- On beat: pupil size increases to 7 pixels
- Smooth, rhythm-following animation

**Mode 1 - Wandering Pupils**:
- Pupils move randomly within eye bounds
- Small random movement each frame
- On beat: pupils enlarge (size 4)
- Creates "looking around" effect

**Mode 2 - Beat-Reactive**:
- Normal: pupils size follows audio level (2-5 pixels)
- On beat: pupils dramatically enlarge (size 6)
- Strong reaction to musical beats
- Eyes "widen" in response to music

### Pupil Position Calculation

```cpp
void LEDMatrixRenderer::animateEyesMode0(uint8_t audioLevel, bool isBeat) {
    // Calculate pupil size based on audio level
    uint8_t pulseSize = map(audioLevel, 0, 100, 2, 6);
    if (isBeat) pulseSize = 7;  // Bigger on beat
    
    // Draw pupils centered in eyes
    drawEyePupil(0, MOUTH_WIDTH, 4, 4, pulseSize);
    drawEyePupil(1, MOUTH_WIDTH + 8, 4, 4, pulseSize);
}
```

## Timing and Update Rates

- **Audio Sampling**: Continuous (every `tick()` call, ~50ms intervals)
- **Mouth Update**: On every new audio sample
- **Eye Update**: Every 100ms (10 FPS) for smooth animation
- **Beat Detection**: Evaluated on every audio sample

## Calibration

Three calibration modes affect visualization:

1. **Silence Threshold**:
   - Determines minimum audio level before visualization starts
   - Set during silence calibration (3-second sample)
   - Formula: `threshold = minLevel + (maxLevel - minLevel) × 0.3`

2. **Sensitivity**:
   - Adjusts animation responsiveness
   - Set during playback (5-second sample)
   - Formula: `threshold = peakLevel × 0.4`
   - Lower threshold = more sensitive (reacts to quieter sounds)

3. **Brightness**:
   - Separate controls for mouth and eyes
   - Affects LED intensity, not visualization algorithm
   - Range: 0-15 (MAX7219 brightness levels)

## Performance Considerations

- **Non-blocking**: All processing uses non-blocking code
- **Efficient Updates**: Only updates changed regions of display
- **Smoothing**: Prevents jittery displays from noise
- **Circular Buffer**: O(1) insertion for waveform history

## Tuning Tips

1. **If visualization is too sensitive**:
   - Increase silence threshold via calibration
   - Adjust smoothing factor in `AudioProcessor.cpp`

2. **If beats aren't detected well**:
   - Decrease beat detection window (BEAT_DETECTION_WINDOW)
   - Lower beat threshold multiplier (currently 1.5)

3. **If display is jittery**:
   - Increase smoothing factor (currently 0.7)
   - Reduce update rate for eyes

4. **If waveform doesn't match audio well**:
   - Check ADC connection and voltage levels
   - Adjust ADC scaling in `updateAudioLevel()`
   - Verify amplifier output is within 0-3.3V range

