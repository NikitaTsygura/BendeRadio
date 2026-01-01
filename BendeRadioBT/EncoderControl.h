/*
 * BendeRadio BT - Rotary Encoder Control Module
 * 
 * Handles rotary encoder with push button state machine
 * Supports: rotate, click, double-click, triple-click, hold, and combinations
 */

#pragma once

#include <Arduino.h>
#include <EncButton.h>
#include "config.h"

// Encoder event types
enum EncoderEventType {
    EVENT_NONE = 0,
    EVENT_ROTATE,                  // Simple rotation
    EVENT_HOLD_ROTATE,             // Hold button + rotate
    EVENT_SINGLE_CLICK,            // Single click
    EVENT_DOUBLE_CLICK,            // Double click
    EVENT_TRIPLE_CLICK,            // Triple click
    EVENT_CLICK_HOLD_ROTATE,       // Click + hold + rotate (mouth brightness)
    EVENT_DOUBLE_CLICK_HOLD_ROTATE // Double click + hold + rotate (eye brightness)
};

// Encoder event structure
struct EncoderEvent {
    EncoderEventType type;
    int8_t value;  // Rotation direction (-1 or 1) or click count
};

class EncoderControl {
public:
    EncoderControl();
    
    // Initialization
    void init();
    
    // Main update function (call in loop)
    void tick();
    
    // Get the latest event (returns EVENT_NONE if no new event)
    EncoderEvent getEvent();
    
    // Check if button is currently being held
    bool isHolding() const { return encButton.pressing(); }
    
    // Check current encoder position
    int32_t getPosition() const { return encButton.getPosition(); }
    
private:
    EncButton encButton;
    EncoderEvent lastEvent;
    bool eventConsumed;
    
    // State tracking for complex gestures
    unsigned long clickStartTime;
    uint8_t clickCount;
    bool wasHolding;
    
    void processEncoderState();
    EncoderEventType determineEventType();
};

