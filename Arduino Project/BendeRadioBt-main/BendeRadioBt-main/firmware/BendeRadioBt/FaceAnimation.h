#ifndef FACE_ANIMATION_H
#define FACE_ANIMATION_H

#include <LedControl.h>

class FaceAnimation {
public:
    FaceAnimation(int din, int clk, int cs);
    void begin();
    void showEyes();
    void blink();
private:
    LedControl lc;
    void clearMatrix();
};

#endif
