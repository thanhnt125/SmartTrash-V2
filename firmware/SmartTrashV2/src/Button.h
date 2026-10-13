#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

typedef void (*ButtonCallback)();

class Button
{
private:
    uint8_t pin;
    volatile bool pressed;
    volatile unsigned long pressTime;
    volatile unsigned long lastTriggerTime;
    bool longHandled;

    uint16_t debounceDelay;
    uint16_t longPressDuration;

    ButtonCallback onShortPress;
    ButtonCallback onLongPress;

public:
    Button(uint8_t buttonPin, uint16_t debounce = 200, uint16_t longPress = 1000);

    void begin();
    void update();
    void handleInterrupt();

    // Gán callback functions
    void setShortPressHandler(ButtonCallback callback);
    void setLongPressHandler(ButtonCallback callback);

    // Các biến volatile cần truy cập từ ISR
    volatile bool *getPressedFlag() { return &pressed; }
    volatile unsigned long *getPressTimePtr() { return &pressTime; }
    volatile unsigned long *getLastTriggerTimePtr() { return &lastTriggerTime; }
    bool *getLongHandledPtr() { return &longHandled; }
    uint8_t getPin() { return pin; }
    uint16_t getDebounceDelay() { return debounceDelay; }
};

#endif
