#include "Button.h"

Button::Button(uint8_t buttonPin, uint16_t debounce, uint16_t longPress)
{
    pin = buttonPin;
    debounceDelay = debounce;
    longPressDuration = longPress;
    pressed = false;
    pressTime = 0;
    lastTriggerTime = 0;
    longHandled = false;
    onShortPress = nullptr;
    onLongPress = nullptr;
}

void Button::begin()
{
    pinMode(pin, INPUT_PULLUP);
}

void Button::handleInterrupt()
{
    unsigned long currentTime = millis();
    if (digitalRead(pin) == LOW)
    {
        if (currentTime - lastTriggerTime > debounceDelay)
        {
            pressed = true;
            pressTime = currentTime;
            longHandled = false;
        }
    }
}

void Button::update()
{
    if (!pressed || longHandled)
    {
        // Reset sau khi nhả từ long press
        if (pressed && longHandled && digitalRead(pin) == HIGH)
        {
            pressed = false;
            lastTriggerTime = millis();
        }
        return;
    }

    unsigned long currentTime = millis();

    // Chỉ xử lý sau khi hết debounce
    if (currentTime - pressTime >= debounceDelay)
    {
        // Đang giữ nút - kiểm tra long press
        if (digitalRead(pin) == LOW)
        {
            if (currentTime - pressTime >= (debounceDelay + longPressDuration))
            {
                longHandled = true;
                if (onLongPress != nullptr)
                {
                    onLongPress();
                }
            }
        }
        // Đã nhả nút - short press
        else
        {
            pressed = false;
            lastTriggerTime = currentTime;
            if (onShortPress != nullptr)
            {
                onShortPress();
            }
        }
    }
}

void Button::setShortPressHandler(ButtonCallback callback)
{
    onShortPress = callback;
}

void Button::setLongPressHandler(ButtonCallback callback)
{
    onLongPress = callback;
}
