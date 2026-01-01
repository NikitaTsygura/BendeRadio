# Audio Gain Limiting - Quick Implementation Guide

## Problem
Windows PCs send Bluetooth audio at very high digital levels, causing clipping/distortion even at low PC volume settings. Smartphones work fine.

## Solution Provided
The firmware includes `AudioGainLimiter` class that:
- Scales audio samples to 30% of full scale (configurable)
- Uses `constrain()` to prevent clipping
- Processes 16-bit signed integer samples
- Non-blocking, safe for DAC and amplifier

## Integration Steps

### For pschatzmann BluetoothA2DPSink Library (Most Common)

**Option 1: Modify Library Source (Recommended for Best Results)**

1. Locate your library installation (usually in Arduino/libraries/ESP32-A2DP or similar)
2. Find the file that handles I2S writes (often `BluetoothA2DPSink.cpp` or similar)
3. Locate the function that writes audio to I2S (look for `i2s_write()` calls)
4. Before the `i2s_write()` call, add gain limiting code:

```cpp
// Example modification in library source
// Find code like this:
i2s_write(I2S_NUM_0, data, len, &bytes_written, portMAX_DELAY);

// Replace with:
// Apply gain limiting (30% to prevent PC audio clipping)
int16_t* samples = (int16_t*)data;
int sample_count = len / 2;  // 2 bytes per sample
for (int i = 0; i < sample_count; i++) {
    samples[i] = (int16_t)(samples[i] * 0.3f);  // 30% gain
    samples[i] = constrain(samples[i], -32768, 32767);
}
i2s_write(I2S_NUM_0, data, len, &bytes_written, portMAX_DELAY);
```

**Option 2: Use Volume Scaling (Simpler Workaround)**

In `BendeRadioBT.ino`, after `a2dp_sink.start()`, add:

```cpp
// Workaround: Use volume scaling (less precise than gain limiting)
a2dp_sink.set_volume(30);  // 30% volume instead of 100%
```

This helps but may not fully prevent clipping with very high PC levels.

### For Libraries with I2S Callbacks

If your library supports callbacks (like btAudio), you can use the callback approach. See `AUDIO_GAIN_LIMITING.md` for details.

## Testing

1. Connect ESP32 to Windows PC via Bluetooth
2. Play audio at PC volume 1/100
3. Listen for distortion/clipping
4. If still present, reduce gain factor in `config.h`:
   ```cpp
   #define DEFAULT_AUDIO_GAIN_FACTOR  0.25f  // Try 25% instead of 30%
   ```
5. Test with smartphone to ensure it still works well

## Configuration

Edit `config.h` to adjust gain:

```cpp
#define DEFAULT_AUDIO_GAIN_FACTOR  0.3f  // 30% of full scale
```

Values:
- `0.2-0.25`: Very safe, prevents all clipping (may be quieter)
- `0.3`: Recommended default (balanced)
- `0.35-0.4`: Less limiting (may clip with very loud PC sources)

## Files Added

- `AudioGainLimiter.h` / `.cpp`: Gain limiting implementation
- `AUDIO_GAIN_LIMITING.md`: Detailed documentation
- `IMPLEMENTATION_GUIDE.md`: This file (quick start)

The `AudioGainLimiter` class is initialized in the main sketch and ready to use once integrated with your Bluetooth library's audio processing pipeline.

