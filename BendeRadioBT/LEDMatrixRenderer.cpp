/*
 * BendeRadio BT - LED Matrix Renderer Implementation
 * 
 * Implements real-time audio waveform visualization on mouth and eye animations
 */

#include "LEDMatrixRenderer.h"
#include <FastLED.h>

LEDMatrixRenderer::LEDMatrixRenderer()
    : mouthBrightness(8)
    , eyeBrightness(10)
    , waveformIndex(0)
    , lastEyeUpdate(0)
    , eyeAnimationFrame(0)
{
    for (uint8_t i = 0; i < MOUTH_WIDTH; i++) {
        waveformHistory[i] = 0;
    }
    eyePupilX[0] = 4;
    eyePupilY[0] = 4;
    eyePupilX[1] = 4;
    eyePupilY[1] = 4;
}

void LEDMatrixRenderer::init() {
    matrix.begin();
    updateBrightness();
    matrix.clear();
    matrix.update();
}

void LEDMatrixRenderer::tick() {
    // Called in main loop for any time-based animations
}

void LEDMatrixRenderer::setMouthBrightness(uint8_t brightness) {
    mouthBrightness = constrain(brightness, 0, MAX_BRIGHTNESS);
    updateBrightness();
}

void LEDMatrixRenderer::setEyeBrightness(uint8_t brightness) {
    eyeBrightness = constrain(brightness, 0, MAX_BRIGHTNESS);
    updateBrightness();
}

void LEDMatrixRenderer::updateBrightness() {
    // Set brightness for each matrix: 3 mouth matrices + 2 eye matrices
    uint8_t brightnesses[TOTAL_MATRICES];
    brightnesses[0] = mouthBrightness;
    brightnesses[1] = mouthBrightness;
    brightnesses[2] = mouthBrightness;
    brightnesses[3] = eyeBrightness;
    brightnesses[4] = eyeBrightness;
    matrix.setBright(brightnesses);
}

void LEDMatrixRenderer::drawMouthWaveform(uint8_t audioLevel) {
    // Store current level in history
    waveformHistory[waveformIndex] = audioLevel;
    waveformIndex = (waveformIndex + 1) % MOUTH_WIDTH;
    
    // Draw waveform based on current visual effect mode
    // For now, use mode 0 (vertical bars)
    drawWaveformMode0(audioLevel);
}

void LEDMatrixRenderer::drawWaveformMode0(uint8_t audioLevel) {
    // Vertical bars: each column represents a sample from history
    clearMouth();
    
    for (uint8_t x = 0; x < MOUTH_WIDTH; x++) {
        uint8_t level = waveformHistory[(waveformIndex + x) % MOUTH_WIDTH];
        uint8_t height = map(level, 0, 100, 0, MATRIX_HEIGHT);
        height = constrain(height, 0, MATRIX_HEIGHT - 1);
        
        // Draw vertical bar
        for (uint8_t y = 0; y <= height; y++) {
            matrix.dot(x, MATRIX_HEIGHT - 1 - y);
        }
    }
    
    matrix.update();
}

void LEDMatrixRenderer::drawWaveformMode1(uint8_t audioLevel) {
    // Centered bars: waveform centered vertically
    clearMouth();
    
    for (uint8_t x = 0; x < MOUTH_WIDTH; x++) {
        uint8_t level = waveformHistory[(waveformIndex + x) % MOUTH_WIDTH];
        uint8_t height = map(level, 0, 100, 0, MATRIX_HEIGHT / 2);
        height = constrain(height, 0, MATRIX_HEIGHT / 2);
        
        uint8_t centerY = MATRIX_HEIGHT / 2;
        
        // Draw centered bar
        for (uint8_t y = 0; y < height; y++) {
            matrix.dot(x, centerY - y);
            matrix.dot(x, centerY + y);
        }
        matrix.dot(x, centerY);
    }
    
    matrix.update();
}

void LEDMatrixRenderer::drawWaveformMode2(uint8_t audioLevel) {
    // Frequency-like: smooth curve
    clearMouth();
    
    uint8_t prevY = MATRIX_HEIGHT / 2;
    
    for (uint8_t x = 0; x < MOUTH_WIDTH; x++) {
        uint8_t level = waveformHistory[(waveformIndex + x) % MOUTH_WIDTH];
        uint8_t y = map(level, 0, 100, 0, MATRIX_HEIGHT - 1);
        y = constrain(y, 0, MATRIX_HEIGHT - 1);
        
        // Draw line from previous point
        if (x > 0) {
            int8_t y1 = prevY;
            int8_t y2 = y;
            if (y1 > y2) {
                int8_t temp = y1;
                y1 = y2;
                y2 = temp;
            }
            for (int8_t py = y1; py <= y2; py++) {
                matrix.dot(x, py);
            }
        } else {
            matrix.dot(x, y);
        }
        
        prevY = y;
    }
    
    matrix.update();
}

void LEDMatrixRenderer::updateEyeAnimation(uint8_t audioLevel, bool isBeat, uint8_t effectMode) {
    unsigned long now = millis();
    if (now - lastEyeUpdate < EYE_ANIMATION_INTERVAL) return;
    lastEyeUpdate = now;
    
    switch (effectMode) {
        case 0:
            animateEyesMode0(audioLevel, isBeat);
            break;
        case 1:
            animateEyesMode1(audioLevel, isBeat);
            break;
        case 2:
            animateEyesMode2(audioLevel, isBeat);
            break;
        default:
            animateEyesMode0(audioLevel, isBeat);
            break;
    }
}

void LEDMatrixRenderer::animateEyesMode0(uint8_t audioLevel, bool isBeat) {
    // Pulsing: eyes pulse with audio level
    clearEyes();
    
    uint8_t pulseSize = map(audioLevel, 0, 100, 2, 6);
    if (isBeat) pulseSize = 7;  // Bigger pulse on beat
    
    // Draw both eyes with pulsing pupils
    for (uint8_t eye = 0; eye < 2; eye++) {
        drawEye(eye, MOUTH_WIDTH + eye * EYE_WIDTH);
        uint8_t centerX = 4;
        uint8_t centerY = 4;
        drawEyePupil(eye, MOUTH_WIDTH + eye * EYE_WIDTH, centerX, centerY, pulseSize);
    }
    
    matrix.update();
}

void LEDMatrixRenderer::animateEyesMode1(uint8_t audioLevel, bool isBeat) {
    // Wandering pupils: pupils move randomly, pulse on beat
    clearEyes();
    
    // Update pupil positions with slight random movement
    for (uint8_t eye = 0; eye < 2; eye++) {
        // Add small random movement
        eyePupilX[eye] += random(-1, 2);
        eyePupilY[eye] += random(-1, 2);
        
        // Constrain to eye bounds (1-6, avoiding edges)
        eyePupilX[eye] = constrain(eyePupilX[eye], 2, 5);
        eyePupilY[eye] = constrain(eyePupilY[eye], 2, 5);
    }
    
    uint8_t pupilSize = isBeat ? 4 : 2;
    
    for (uint8_t eye = 0; eye < 2; eye++) {
        drawEye(eye, MOUTH_WIDTH + eye * EYE_WIDTH);
        drawEyePupil(eye, MOUTH_WIDTH + eye * EYE_WIDTH, eyePupilX[eye], eyePupilY[eye], pupilSize);
    }
    
    matrix.update();
}

void LEDMatrixRenderer::animateEyesMode2(uint8_t audioLevel, bool isBeat) {
    // Beat-reactive: eyes react strongly to beats
    clearEyes();
    
    if (isBeat) {
        // On beat: eyes widen (draw larger pupils)
        for (uint8_t eye = 0; eye < 2; eye++) {
            drawEye(eye, MOUTH_WIDTH + eye * EYE_WIDTH);
            drawEyePupil(eye, MOUTH_WIDTH + eye * EYE_WIDTH, 4, 4, 6);
        }
    } else {
        // Normal: pupils follow audio level
        uint8_t pupilSize = map(audioLevel, 0, 100, 2, 5);
        for (uint8_t eye = 0; eye < 2; eye++) {
            drawEye(eye, MOUTH_WIDTH + eye * EYE_WIDTH);
            drawEyePupil(eye, MOUTH_WIDTH + eye * EYE_WIDTH, 4, 4, pupilSize);
        }
    }
    
    matrix.update();
}

void LEDMatrixRenderer::drawEye(uint8_t eyeIndex, uint8_t xOffset) {
    // Draw eye outline (Bender-style eye shape)
    // Outer rectangle
    matrix.rect(xOffset + 1, 1, xOffset + 6, 6, GFX_FILL);
    
    // Eye outline lines
    matrix.lineV(xOffset + 0, 2, 5);
    matrix.lineV(xOffset + 7, 2, 5);
    matrix.lineH(0, xOffset + 2, xOffset + 5);
    matrix.lineH(7, xOffset + 2, xOffset + 5);
}

void LEDMatrixRenderer::drawEyePupil(uint8_t eyeIndex, uint8_t xOffset, int8_t pupilX, int8_t pupilY, uint8_t size) {
    // Draw pupil (clear area for pupil)
    int8_t halfSize = size / 2;
    int8_t x1 = pupilX - halfSize;
    int8_t y1 = pupilY - halfSize;
    int8_t x2 = pupilX + halfSize;
    int8_t y2 = pupilY + halfSize;
    
    // Constrain to eye bounds
    x1 = constrain(x1, 1, 6);
    y1 = constrain(y1, 1, 6);
    x2 = constrain(x2, 1, 6);
    y2 = constrain(y2, 1, 6);
    
    // Clear pupil area (draw inverted)
    matrix.rect(xOffset + x1, y1, xOffset + x2, y2, GFX_CLEAR);
}

void LEDMatrixRenderer::showClock(uint8_t hours, uint8_t minutes) {
    clearMouth();
    clearEyes();
    
    // Draw clock digits on eyes
    // Left eye: hours (2 digits)
    uint8_t hoursTens = hours / 10;
    uint8_t hoursOnes = hours % 10;
    
    // Right eye: minutes (2 digits)
    uint8_t minutesTens = minutes / 10;
    uint8_t minutesOnes = minutes % 10;
    
    // Draw hours on left eye (split across 8 columns)
    matrix.setCursor(MOUTH_WIDTH + 1, 2);
    matrix.print(hoursTens);
    matrix.setCursor(MOUTH_WIDTH + 4, 2);
    matrix.print(hoursOnes);
    
    // Draw minutes on right eye
    matrix.setCursor(MOUTH_WIDTH + EYE_WIDTH + 1, 2);
    matrix.print(minutesTens);
    matrix.setCursor(MOUTH_WIDTH + EYE_WIDTH + 4, 2);
    matrix.print(minutesOnes);
    
    matrix.update();
}

void LEDMatrixRenderer::showPowerOnState() {
    clearMouth();
    
    // Draw happy eyes (centered pupils)
    for (uint8_t eye = 0; eye < 2; eye++) {
        drawEye(eye, MOUTH_WIDTH + eye * EYE_WIDTH);
        drawEyePupil(eye, MOUTH_WIDTH + eye * EYE_WIDTH, 4, 4, 3);
    }
    
    matrix.update();
}

void LEDMatrixRenderer::showVolume(uint8_t volume) {
    clearMouth();
    
    // Show "V" and volume value on mouth
    matrix.setCursor(2, 2);
    matrix.print('V');
    matrix.setCursor(10, 2);
    matrix.print(volume / 10);
    matrix.setCursor(18, 2);
    matrix.print(volume % 10);
    
    matrix.update();
    
    // Clear after 2 seconds (handled by main loop if needed)
}

void LEDMatrixRenderer::showMode(uint8_t mode) {
    clearMouth();
    
    // Show "M" and mode number
    matrix.setCursor(2, 2);
    matrix.print('M');
    matrix.setCursor(10, 2);
    matrix.print(mode);
    
    matrix.update();
}

void LEDMatrixRenderer::showEffectChange(uint8_t effect) {
    clearMouth();
    
    // Show "E" and effect number
    matrix.setCursor(2, 2);
    matrix.print('E');
    matrix.setCursor(10, 2);
    matrix.print(effect);
    
    matrix.update();
}

void LEDMatrixRenderer::showMouthBrightness(uint8_t brightness) {
    clearMouth();
    
    matrix.setCursor(2, 2);
    matrix.print('B');
    matrix.setCursor(10, 2);
    matrix.print('M');
    matrix.setCursor(18, 2);
    matrix.print(brightness);
    
    matrix.update();
}

void LEDMatrixRenderer::showEyeBrightness(uint8_t brightness) {
    clearMouth();
    
    matrix.setCursor(2, 2);
    matrix.print('B');
    matrix.setCursor(10, 2);
    matrix.print('E');
    matrix.setCursor(18, 2);
    matrix.print(brightness);
    
    matrix.update();
}

void LEDMatrixRenderer::showCalibrationMenu() {
    matrix.clear();
    
    // Show "CAL" on mouth
    matrix.setCursor(4, 2);
    matrix.print("CAL");
    
    matrix.update();
}

void LEDMatrixRenderer::showCalibrationSelection(uint8_t selection) {
    clearMouth();
    
    // Show selection number and hint
    const char* hints[] = {"SIL", "TIM", "SEN"};
    matrix.setCursor(2, 2);
    matrix.print(selection);
    matrix.setCursor(10, 2);
    matrix.print(hints[selection]);
    
    matrix.update();
}

void LEDMatrixRenderer::showCalibrationMessage(const char* msg, uint8_t line) {
    if (line == 0) {
        clearMouth();
        matrix.setCursor(4, 2);
        matrix.print(msg);
    }
    matrix.update();
}

void LEDMatrixRenderer::showCalibrationComplete() {
    clearMouth();
    
    matrix.setCursor(4, 2);
    matrix.print("OK");
    
    matrix.update();
}

void LEDMatrixRenderer::showTimeEdit(uint8_t hours, uint8_t minutes, bool editingHours) {
    clearMouth();
    clearEyes();
    
    // Show hours and minutes with cursor
    matrix.setCursor(MOUTH_WIDTH + 1, 2);
    if (editingHours) {
        matrix.invertDisplay(true);
        matrix.print(hours / 10);
        matrix.print(hours % 10);
        matrix.invertDisplay(false);
    } else {
        matrix.print(hours / 10);
        matrix.print(hours % 10);
    }
    
    matrix.setCursor(MOUTH_WIDTH + EYE_WIDTH + 1, 2);
    if (!editingHours) {
        matrix.invertDisplay(true);
        matrix.print(minutes / 10);
        matrix.print(minutes % 10);
        matrix.invertDisplay(false);
    } else {
        matrix.print(minutes / 10);
        matrix.print(minutes % 10);
    }
    
    matrix.update();
}

void LEDMatrixRenderer::clearMouth() {
    matrix.rect(0, 0, MOUTH_WIDTH - 1, MATRIX_HEIGHT - 1, GFX_CLEAR);
}

void LEDMatrixRenderer::clearEyes() {
    matrix.rect(MOUTH_WIDTH, 0, MOUTH_WIDTH + 2 * EYE_WIDTH - 1, MATRIX_HEIGHT - 1, GFX_CLEAR);
}

