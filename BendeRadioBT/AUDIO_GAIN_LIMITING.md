# Audio Gain Limiting Implementation Guide

## Problem

Windows PCs send Bluetooth audio at higher digital levels than smartphones, causing:
- Audio clipping and distortion
- DAC overload
- Amplifier overload
- Even at very low PC system volume (1/100)

## Solution

Digital gain limiting applied before the DAC:
- Scales down incoming audio samples (20-40% of full scale)
- Prevents clipping using `constrain()`
- Applied globally to all Bluetooth audio
- Non-blocking implementation

## Implementation Methods

The exact implementation depends on which Bluetooth library you're using. Here are the most common approaches:

### Method 1: Using I2S Callback (Recommended if Supported)

Some libraries (like `btAudio`) allow you to register a callback that processes audio before I2S:

```cpp
void i2s_audio_callback(const uint8_t* data, uint32_t len) {
    static uint8_t buffer[512];
    size_t bytes = (len < sizeof(buffer)) ? len : sizeof(buffer);
    
    memcpy(buffer, data, bytes);
    audioGainLimiter.processBuffer(buffer, bytes);
    
    size_t written = 0;
    i2s_write(I2S_NUM_0, buffer, bytes, &written, portMAX_DELAY);
}
```

### Method 2: For pschatzmann BluetoothA2DPSink Library

The pschatzmann library handles I2S internally. You have two options:

#### Option A: Modify Library Source (Advanced)

1. Find the I2S write function in the library source
2. Add gain limiting before the `i2s_write()` call
3. Look for files like `BluetoothA2DPSink.cpp` or `i2s_output.cpp`

Example modification:
```cpp
// In library source, before i2s_write():
int16_t* samples = (int16_t*)data;
for (int i = 0; i < len/2; i++) {
    samples[i] = (int16_t)(samples[i] * 0.3f);  // 30% gain
    samples[i] = constrain(samples[i], -32768, 32767);
}
```

#### Option B: Use Volume Scaling (Simpler but Less Precise)

The library's volume control can provide some limiting, but it's less precise:

```cpp
// In setup(), after a2dp_sink.start():
a2dp_sink.set_volume(30);  // 30% volume (0-100 scale)
```

This helps but may not be sufficient for very high PC levels.

### Method 3: Hardware Solution (Not Software, But Effective)

Add a voltage divider or attenuator between DAC and amplifier:
- Resistor divider: 2:1 or 3:1 ratio
- Provides hardware-level protection
- Works regardless of software

## Current Implementation

The firmware includes `AudioGainLimiter` class that provides:

- `processSample()` - Process single 16-bit sample
- `processStereoPair()` - Process left/right pair
- `processBuffer()` - Process entire audio buffer

**Default gain factor: 0.3 (30% of full scale)**

This prevents PC audio clipping while maintaining good quality from smartphones.

## Configuration

Adjust gain in `config.h`:

```cpp
#define DEFAULT_AUDIO_GAIN_FACTOR  0.3f  // 30% of full scale
```

Recommended values:
- **0.2-0.25**: Very safe, prevents all clipping (may be quiet)
- **0.3-0.35**: Balanced (recommended default)
- **0.4-0.5**: Less limiting (may clip with very loud PC sources)

## Testing

1. Connect to Windows PC
2. Play audio at various PC volume levels
3. Check for distortion/clipping
4. Adjust `DEFAULT_AUDIO_GAIN_FACTOR` if needed
5. Test with smartphone for comparison

## Library-Specific Notes

### pschatzmann/ESP32-A2DP
- Library handles I2S internally
- May need to modify library source (see Method 2A)
- Or use volume scaling as workaround (see Method 2B)

### earlephilhower/ESP32-A2DP
- Check library documentation for audio processing callbacks
- May support similar callback mechanism

### btAudio Library
- Supports I2S callbacks directly
- Use Method 1 implementation

## Safety

- Gain limiting is applied **before** DAC, protecting both DAC and amplifier
- Uses `constrain()` to ensure samples stay within valid range (-32768 to +32767)
- Non-blocking implementation doesn't affect system responsiveness
- Applied globally to all audio sources

