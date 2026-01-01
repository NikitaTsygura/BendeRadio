#include "FaceAnimation.h"

FaceAnimation::FaceAnimation(int din, int clk, int cs) : lc(din, clk, cs, 1) {}

void FaceAnimation::begin() {
    lc.shutdown(0, false);
    lc.setIntensity(0, 8);
    lc.clearDisplay(0);
}

void FaceAnimation::showEyes() {
    clearMatrix();
    byte eyes[8] = {
        0b00000000,
        0b00100100,
        0b01100110,
        0b01100110,
        0b01100110,
        0b00100100,
        0b00000000,
        0b00000000
    };
    for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, eyes[i]);
    }
}

void FaceAnimation::blink() {
    clearMatrix();
    byte blinkEyes[8] = {
        0b00000000,
        0b00000000,
        0b00000000,
        0b01100110,
        0b01100110,
        0b00000000,
        0b00000000,
        0b00000000
    };
    for (int i = 0; i < 8; i++) {
        lc.setRow(0, i, blinkEyes[i]);
    }
    delay(200);
    showEyes();
}

void FaceAnimation::clearMatrix() {
    lc.clearDisplay(0);
}
