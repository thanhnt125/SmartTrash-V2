#include "SmartTrash.h"
#include "Config.h"

// Define static soundOptions array (5 trash types: functions 0-4)
// Based on sounds/README.md
const SoundIndex SmartTrash::soundOptions[5] = {
    HAY_BO_RAC_VAO_THUNG,    // Option 0: track 2 - Hãy bỏ rác vào thùng
    RAC_HUU_CO,              // Option 1: track 3 - Rác hữu cơ
    RAC_VO_CO,               // Option 2: track 4 - Rác vô cơ
    RAC_TAI_CHE,             // Option 3: track 5 - Rác tái chế
    CAC_LOAI_PIN_VA_AC_QUY}; // Option 4: track 6 - Các loại pin và ắc quy

SmartTrash::SmartTrash(uint8_t servoPin, uint8_t trigPin, uint8_t echoPin,
                       uint8_t trigVolPin, uint8_t echoVolPin,
                       uint8_t redPin, uint8_t ylPin, uint8_t grPin, uint8_t buzPin)
{
    this->servoPin = servoPin;
    this->trigPin = trigPin;
    this->echoPin = echoPin;
    this->trigVolPin = trigVolPin;
    this->echoVolPin = echoVolPin;
    this->redPin = redPin;
    this->ylPin = ylPin;
    this->grPin = grPin;
    this->buzPin = buzPin;

    this->currentAngle = 0;
    this->lastReadTime = 0;
    this->lastCloseTime = 0;
    this->currentSoundIndex = 0;
    this->isInitialized = false;
    this->isTrashFull = false;
    this->autoMode = true;
    this->audioPlayer = nullptr;

    // Load configuration from Config.h
    this->distanceOpen = TRASH_DISTANCE_OPEN;
    this->distanceKeepOpen = TRASH_DISTANCE_KEEP_OPEN;
    this->openAngle = TRASH_OPEN_ANGLE;
    this->closeAngle = TRASH_CLOSE_ANGLE;
    this->delayOpenTime = TRASH_DELAY_OPEN_TIME;
    this->delayReadSensor = TRASH_DELAY_READ_SENSOR;
    this->reopenCooldown = TRASH_REOPEN_COOLDOWN;
}

void SmartTrash::begin(AudioPlayer *audio)
{
    this->audioPlayer = audio;

    // Setup pins
    servo.attach(servoPin);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(trigVolPin, OUTPUT);
    pinMode(echoVolPin, INPUT);
    pinMode(redPin, OUTPUT);
    pinMode(ylPin, OUTPUT);
    pinMode(grPin, OUTPUT);
    pinMode(buzPin, OUTPUT);

    Serial.println("SmartTrash initialized");

    // Play welcome sound (folder 1, track 1: "Thùng Rác STEM")
    if (audioPlayer != nullptr)
    {
        audioPlayer->playFolder(1, 1);
        delay(TRASH_DELAY_BEFORE_OPEN);
    }

    // Test LED
    testLED();

    // Initial servo test
    currentAngle = moveServoWithDelay(currentAngle, openAngle);
    delay(1000);
    currentAngle = moveServoWithDelay(currentAngle, closeAngle);

    // Mark as initialized
    isInitialized = true;
}

void SmartTrash::run(AudioPlayer *audio)
{
    // Auto-init nếu chưa khởi tạo
    if (!isInitialized)
    {
        begin(audio);
    }

    // Thực thi update
    update();
}

void SmartTrash::reset()
{
    // Reset state
    lastReadTime = 0;

    // Close lid
    if (currentAngle != closeAngle)
    {
        currentAngle = moveServoWithDelay(currentAngle, closeAngle);
    }
}

void SmartTrash::setSoundIndex(uint8_t index)
{
    if (index < 5 && index != currentSoundIndex) // 0-4 (5 options)
    {
        currentSoundIndex = index;

        // Phát preview sound nếu audioPlayer đã được khởi tạo
        if (audioPlayer != nullptr)
        {
            audioPlayer->playFolder(1, soundOptions[currentSoundIndex]);
            delay(100);
        }
    }
}

void SmartTrash::update()
{
    unsigned long currentTime = millis();

    // Read sensor every delayReadSensor ms
    if (currentTime - lastReadTime >= delayReadSensor)
    {
        lastReadTime = currentTime;

        // Read object detection sensor (chi dung o che do tu dong)
        long distance = -1;
        if (autoMode)
        {
            distance = readUltrasonicDistance();
            Serial.print("Object: ");
            Serial.print(distance);
            Serial.print(" cm | ");
        }

        // If object detected, open lid
        if (autoMode && distance < distanceOpen && distance >= 0)
        {
            // Prevent rapid open-close cycling when object keeps blocking sensor.
            if (currentTime - lastCloseTime < (unsigned long)reopenCooldown)
            {
                Serial.println("Reopen cooldown active");
            }
            else
            {
                do
                {
                    currentAngle = moveServoWithDelay(currentAngle, openAngle);
                    Serial.println("Lid opened");

                    // Play sound based on current index (folder 1) after opening lid
                    // If trash is full, play full warning instead of classification sound
                    if (audioPlayer != nullptr)
                    {
                        if (isTrashFull)
                        {
                            audioPlayer->playFolder(1, THUNG_RAC_DAY);
                        }
                        else if (currentSoundIndex < 5)
                        {
                            audioPlayer->playFolder(1, soundOptions[currentSoundIndex]);
                        }
                    }

                    delay(delayOpenTime);

                    // Check if object still present
                    distance = readUltrasonicDistance();

                } while (distance < distanceKeepOpen && distance > 0);

                // Wait before closing lid
                delay(TRASH_DELAY_BEFORE_CLOSE);

                // Close lid
                currentAngle = moveServoWithDelay(currentAngle, closeAngle);
                lastCloseTime = millis();
                Serial.println("Lid closed");

                // Play thank you sound (folder 1, track 7) after lid is closed
                if (audioPlayer != nullptr)
                {
                    audioPlayer->playFolder(1, XIN_CAM_ON);
                }
            }
        }

        // Read volume sensor and update LED
        long volumeDistance = readUltrasonicDistanceVolume();
        updateVolumeLED(volumeDistance);
        Serial.print("Volume: ");
        Serial.println(volumeDistance);
    }
}

void SmartTrash::testLED()
{
    digitalWrite(redPin, TRASH_LED_ON);
    digitalWrite(ylPin, TRASH_LED_ON);
    digitalWrite(grPin, TRASH_LED_ON);
    digitalWrite(buzPin, HIGH);
    delay(1000);
    digitalWrite(redPin, TRASH_LED_OFF);
    digitalWrite(ylPin, TRASH_LED_OFF);
    digitalWrite(grPin, TRASH_LED_OFF);
    digitalWrite(buzPin, LOW);
}

int SmartTrash::moveServoWithDelay(int currentAngle, int targetAngle)
{
    int angleChange = abs(targetAngle - currentAngle);
    int delayTime = angleChange * 1; // 1ms per degree for SG90

    servo.write(targetAngle);
    delay(delayTime);

    // Serial.print("Servo: ");
    // Serial.print(currentAngle);
    // Serial.print("° -> ");
    // Serial.print(targetAngle);
    // Serial.print("°, delay: ");
    // Serial.print(delayTime);
    // Serial.println(" ms");

    return targetAngle;
}

long SmartTrash::readUltrasonicDistance()
{
    return readUltrasonicSensor(trigPin, echoPin);
}

long SmartTrash::readUltrasonicDistanceVolume()
{
    return readUltrasonicSensor(trigVolPin, echoVolPin);
}

void SmartTrash::updateVolumeLED(int distance)
{
    if (distance < 0)
    {
        // No echo response - sensor error, keep current state unchanged
        return;
    }

    if (distance < 5)
    {
        // Full - Red LED + Buzzer
        digitalWrite(redPin, TRASH_LED_ON);
        digitalWrite(ylPin, TRASH_LED_OFF);
        digitalWrite(grPin, TRASH_LED_OFF);
        digitalWrite(buzPin, HIGH);

        // Play full warning sound once on transition to full
        if (!isTrashFull && audioPlayer != nullptr)
        {
            audioPlayer->playFolder(1, THUNG_RAC_DAY);
        }
        isTrashFull = true;
    }
    else if (distance < 15)
    {
        // Medium - Yellow LED
        digitalWrite(redPin, TRASH_LED_OFF);
        digitalWrite(ylPin, TRASH_LED_ON);
        digitalWrite(grPin, TRASH_LED_OFF);
        digitalWrite(buzPin, LOW);
        isTrashFull = false;
    }
    else
    {
        // Empty - Green LED
        digitalWrite(redPin, TRASH_LED_OFF);
        digitalWrite(ylPin, TRASH_LED_OFF);
        digitalWrite(grPin, TRASH_LED_ON);
        digitalWrite(buzPin, LOW);
        isTrashFull = false;
    }
}

// ================= DIEU KHIEN TU APP PC =================

void SmartTrash::setAutoMode(bool enable)
{
    autoMode = enable;

    if (enable)
    {
        // Ve tu dong: dong nap neu dang mo thu cong, de cam bien tiep quan
        closeLid();
        Serial.println("SYSTEM_MODE: AUTO");
    }
    else
    {
        Serial.println("SYSTEM_MODE: MANUAL");
    }
}

void SmartTrash::openLid()
{
    if (!isInitialized || currentAngle == openAngle)
    {
        return; // chua khoi tao servo hoac nap da mo
    }

    currentAngle = moveServoWithDelay(currentAngle, openAngle);
    Serial.println("Lid opened");

    if (audioPlayer != nullptr)
    {
        if (isTrashFull)
        {
            audioPlayer->playFolder(1, THUNG_RAC_DAY);
        }
        else if (currentSoundIndex < 5)
        {
            audioPlayer->playFolder(1, soundOptions[currentSoundIndex]);
        }
    }
}

void SmartTrash::closeLid()
{
    if (!isInitialized || currentAngle == closeAngle)
    {
        return; // chua khoi tao servo hoac nap da dong
    }

    currentAngle = moveServoWithDelay(currentAngle, closeAngle);
    lastCloseTime = millis();
    Serial.println("Lid closed");

    if (audioPlayer != nullptr)
    {
        audioPlayer->playFolder(1, XIN_CAM_ON);
    }
}
