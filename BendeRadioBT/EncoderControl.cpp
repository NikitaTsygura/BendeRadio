/*
 * BendeRadio BT - Rotary Encoder Control Implementation
 */

#include "EncoderControl.h"

EncoderControl::EncoderControl() 
    : encButton(ENC_S1, ENC_S2, ENC_BTN)
    , eventConsumed(true)
    , clickStartTime(0)
    , clickCount(0)
    , wasHolding(false)
{
    lastEvent.type = EVENT_NONE;
    lastEvent.value = 0;
}

void EncoderControl::init() {
    encButton.setEncType(EB_STEP4_LOW);  // 4-step encoder with low state
    encButton.setClickTimeout(400);      // 400ms for click detection
    encButton.setStepTimeout(50);        // 50ms debounce for encoder steps
    encButton.setHoldTimeout(500);       // 500ms to detect hold
    encButton.setDoubleTimeout(300);     // 300ms between clicks for double-click
}

void EncoderControl::tick() {
    encButton.tick();
    processEncoderState();
}

void EncoderControl::processEncoderState() {
    if (encButton.turn()) {
        // Encoder rotated
        int8_t dir = encButton.dir();
        
        if (encButton.pressing()) {
            // Button is being held
            if (clickCount == 0) {
                // Hold + Rotate (mode change)
                lastEvent.type = EVENT_HOLD_ROTATE;
                lastEvent.value = dir;
                eventConsumed = false;
            } else if (clickCount == 1) {
                // Click + Hold + Rotate (mouth brightness)
                lastEvent.type = EVENT_CLICK_HOLD_ROTATE;
                lastEvent.value = dir;
                eventConsumed = false;
            } else if (clickCount == 2) {
                // Double Click + Hold + Rotate (eye brightness)
                lastEvent.type = EVENT_DOUBLE_CLICK_HOLD_ROTATE;
                lastEvent.value = dir;
                eventConsumed = false;
            }
        } else {
            // Simple rotation (volume)
            lastEvent.type = EVENT_ROTATE;
            lastEvent.value = dir;
            eventConsumed = false;
        }
        return;
    }
    
    // Track button clicks
    if (encButton.hasClicks()) {
        clickCount = encButton.getClicks();
        
        if (clickCount == 1) {
            lastEvent.type = EVENT_SINGLE_CLICK;
            lastEvent.value = 1;
            eventConsumed = false;
            clickStartTime = millis();
        } else if (clickCount == 2) {
            lastEvent.type = EVENT_DOUBLE_CLICK;
            lastEvent.value = 2;
            eventConsumed = false;
        } else if (clickCount == 3) {
            lastEvent.type = EVENT_TRIPLE_CLICK;
            lastEvent.value = 3;
            eventConsumed = false;
            clickCount = 0;  // Reset after triple click
        }
        return;
    }
    
    // Reset click count after timeout
    if (clickCount > 0 && millis() - clickStartTime > 1000) {
        clickCount = 0;
    }
    
    // Track hold state changes
    bool currentlyHolding = encButton.pressing();
    if (wasHolding && !currentlyHolding) {
        // Button released after hold
        clickCount = 0;  // Reset click count after hold release
    }
    wasHolding = currentlyHolding;
}

EncoderEvent EncoderControl::getEvent() {
    if (eventConsumed) {
        lastEvent.type = EVENT_NONE;
        lastEvent.value = 0;
        return lastEvent;
    }
    
    eventConsumed = true;
    return lastEvent;
}

