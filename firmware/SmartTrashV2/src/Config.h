#ifndef CONFIG_H
#define CONFIG_H

#include <avr/wdt.h>

// Định nghĩa chân LED (only if not already defined)
#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

// Định nghĩa chân nút bấm
#define BTN_1_PIN 2
#define BTN_2_PIN 3

// Định nghĩa chân DFPlayer (RX, TX)
#define DFPLAYER_RX 7
#define DFPLAYER_TX 8

// Định nghĩa chân SmartTrash (Mode 1)
#define TRASH_SERVO_PIN 5
#define TRASH_TRIG_PIN 6
#define TRASH_ECHO_PIN 4
#define TRASH_TRIG_VOL_PIN A4
#define TRASH_ECHO_VOL_PIN A3
// Luu y: Arduino Uno khong co chan A6/A7 (tren Nano thi A6/A7 chi doc analog,
// khong xuat digital duoc) -> LED do/vang chuyen sang D11/D12 de sang duoc
#define TRASH_RED_PIN 11
#define TRASH_YL_PIN 12
#define TRASH_GR_PIN A5
#define TRASH_BUZ_PIN 10

// Cấu hình SmartTrash
#define TRASH_DISTANCE_OPEN 35        // cm
#define TRASH_DISTANCE_KEEP_OPEN 5    // cm
#define TRASH_OPEN_ANGLE 170          // degree
#define TRASH_CLOSE_ANGLE 10           // degree
#define TRASH_DELAY_BEFORE_OPEN 1200  // ms - wait after voice before opening lid
#define TRASH_DELAY_BEFORE_CLOSE 2000 // ms - wait before closing lid
#define TRASH_DELAY_OPEN_TIME 2000    // ms
#define TRASH_DELAY_READ_SENSOR 500   // ms
#define TRASH_REOPEN_COOLDOWN 1000    // ms - minimum closed time before opening again
#define TRASH_LED_ON 1
#define TRASH_LED_OFF 0

// Định nghĩa chân SmartCane (Mode 2)
#define CANE_TRIG_PIN 6
#define CANE_ECHO_PIN 4

// Cấu hình SmartCane
#define CANE_READ_DELAY 500      // ms - delay between sensor reads
#define CANE_DISTANCE_MIN 2      // cm - minimum reliable distance for HC-SR04
#define CANE_DISTANCE_MAX 90     // cm
#define CANE_DISTANCE_UNDER_10 8 // cm - bucket 0..7 maps to "Dưới 10 cm"
#define CANE_DISTANCE_STEP 8     // cm per bucket
#define CANE_TRACK_OFFSET 2      // bucket 1 maps to track 3
#define CANE_TRACK_MAX 11        // track 11 = 90cm
#define CANE_TRACK_UNDER_10 2    // track 2 = dưới 10cm
#define CANE_HELLO_DELAY 1000    // ms - wait after hello message

// Định nghĩa chân ProSit (Mode 5) - dùng chung cảm biến với SmartCane
#define PROSIT_TRIG_PIN CANE_TRIG_PIN
#define PROSIT_ECHO_PIN CANE_ECHO_PIN

// Cấu hình ProSit
#define PROSIT_ALERT_LED_PIN 9
#define PROSIT_AUDIO_FOLDER 5
#define PROSIT_TRACK_GOOD_POSTURE 1
#define PROSIT_TRACK_BAD_POSTURE 2
#define PROSIT_TARGET_DISTANCE_CM 30
#define PROSIT_DISTANCE_TOLERANCE_CM 3
#define PROSIT_SENSOR_READ_DELAY 250
#define PROSIT_ALERT_COOLDOWN 3000
#define PROSIT_FILTER_SIZE 5

// Định nghĩa chân FireKiller (Mode 3)
#define FIRE_SENSOR_APIN A6
#define FIRE_SENSOR_DPIN A7
#define FIRE_RELAY_PIN 9
#define FIRE_SERVO_PIN 5
#define FIRE_BUZZER_PIN 10

// Định nghĩa chân AutoWater (Mode 4)
#define AUTO_WATER_SENSOR_PIN A6
#define AUTO_WATER_RELAY_PIN 9

// Cấu hình FireKiller
#define FIRE_THRESHOLD 1          // Fire detection threshold
#define FIRE_START_ANGLE 90       // degrees
#define FIRE_READ_DELAY 200       // ms - delay between sensor reads
#define FIRE_SERVO_STEP_DELAY 200 // ms - delay between servo steps
#define FIRE_SERVO_STEP 10        // degrees per step

// Cấu hình EEPROM
#define EEPROM_SOUND_ADDR 0
#define EEPROM_MODE_ADDR 1
#define EEPROM_FUNC_ADDR 2 // 5 bytes cho 5 modes

// Cấu hình debounce
#define DEBOUNCE_DELAY 200 // ms

// Cấu hình nhấn giữ
#define LONG_PRESS_DURATION 1000 // ms

// Cấu hình AutoWater
#define AUTO_WATER_DRY_STATE 0
#define AUTO_WATER_WET_STATE 1
#define AUTO_WATER_RELAY_ON HIGH
#define AUTO_WATER_RELAY_OFF LOW
#define AUTO_WATER_DEBOUNCE_COUNT 5
#define AUTO_WATER_DEBOUNCE_INTERVAL 10

// Cấu hình âm lượng
#define DEFAULT_VOLUME 100 // 0-100%

// Hàm reset Arduino bằng watchdog timer
inline void resetArduino()
{
    wdt_enable(WDTO_15MS); // Bật watchdog 15ms
    while (1)
    {
    } // Chờ reset
}

// Hàm đọc cảm biến siêu âm dùng chung cho các mode
inline long readUltrasonicSensor(uint8_t trigPin, uint8_t echoPin)
{
    const long timeout = 23300; // 23.3ms for 400cm round trip

    // Trigger pulse
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Read echo
    unsigned long duration = pulseIn(echoPin, HIGH, timeout);

    // Return -1 if no echo
    if (duration == 0)
    {
        return -1;
    }

    // Calculate distance in cm
    double distance = (duration / 2.0) * 0.0343;
    return (long)distance;
}

// Hàm đọc digital có debounce (dùng lại cho cảm biến digital)
inline int readDigitalDebounced(uint8_t pin, int &lastReading, int &debounceCounter,
                                int &stableValue, unsigned long &lastDebounceTime,
                                uint8_t debounceCount = 5, uint16_t debounceInterval = 10)
{
    unsigned long currentTime = millis();

    if (currentTime - lastDebounceTime >= debounceInterval)
    {
        lastDebounceTime = currentTime;

        int reading = digitalRead(pin);

        if (reading == lastReading)
        {
            debounceCounter++;
            if (debounceCounter >= debounceCount)
            {
                stableValue = reading;
                debounceCounter = debounceCount;
            }
        }
        else
        {
            lastReading = reading;
            debounceCounter = 1;
        }
    }

    return stableValue;
}

#endif
