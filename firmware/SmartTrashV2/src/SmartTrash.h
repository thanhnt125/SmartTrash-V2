#ifndef SMART_TRASH_H
#define SMART_TRASH_H

#include <Arduino.h>
#include <Servo.h>
#include <EEPROM.h>
#include "AudioPlayerConfig.h"

// Sound mapping for folder 01 (SmartTrash)
// Based on sounds/README.md
enum SoundIndex
{
    HAY_BO_RAC_VAO_THUNG = 2,   // 002.mp3 - Function 0: Hãy bỏ rác vào thùng
    RAC_HUU_CO = 3,             // 003.mp3 - Function 1: Rác hữu cơ
    RAC_VO_CO = 4,              // 004.mp3 - Function 2: Rác vô cơ
    RAC_TAI_CHE = 5,            // 005.mp3 - Function 3: Rác tái chế
    CAC_LOAI_PIN_VA_AC_QUY = 6, // 006.mp3 - Function 4: Các loại pin và ắc quy
    XIN_CAM_ON = 7,             // 007.mp3 - Phát khi đóng nắp
    THUNG_RAC_DAY = 8           // 008.mp3 - Cảnh báo thùng rác đầy
};

class SmartTrash
{
private:
    // Pins
    uint8_t servoPin;
    uint8_t trigPin;
    uint8_t echoPin;
    uint8_t trigVolPin;
    uint8_t echoVolPin;
    uint8_t redPin;
    uint8_t ylPin;
    uint8_t grPin;
    uint8_t buzPin;

    // Servo
    Servo servo;
    int currentAngle;

    // Configuration
    int distanceOpen;
    int distanceKeepOpen;
    int openAngle;
    int closeAngle;
    int delayOpenTime;
    int delayReadSensor;
    int reopenCooldown;

    // Timing
    unsigned long lastReadTime;
    unsigned long lastCloseTime;

    // Current sound index
    uint8_t currentSoundIndex;

    // Initialization flag
    bool isInitialized;

    // Full trash state flag
    bool isTrashFull;

    // Che do: true = tu dong (cam bien mo nap), false = thu cong (dieu khien tu app)
    bool autoMode;

    // Audio player reference
    AudioPlayer *audioPlayer;

    // Helper functions
    int moveServoWithDelay(int currentAngle, int targetAngle);
    long readUltrasonicDistance();
    long readUltrasonicDistanceVolume();
    void updateVolumeLED(int distance);

public:
    // Sound options array - public for direct access
    static const SoundIndex soundOptions[5];

    SmartTrash(uint8_t servoPin, uint8_t trigPin, uint8_t echoPin,
               uint8_t trigVolPin, uint8_t echoVolPin,
               uint8_t redPin, uint8_t ylPin, uint8_t grPin, uint8_t buzPin);

    void begin(AudioPlayer *audio);
    void reset();
    void setSoundIndex(uint8_t index);
    void update();
    void run(AudioPlayer *audio);
    void testLED();

    // Dieu khien tu app PC
    void setAutoMode(bool enable);
    void openLid();
    void closeLid();

    // Getter
    bool getIsInitialized() { return isInitialized; }
    bool isAutoMode() { return autoMode; }
};

#endif
