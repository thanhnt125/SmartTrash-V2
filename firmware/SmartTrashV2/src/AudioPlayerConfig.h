#ifndef AUDIO_PLAYER_CONFIG_H
#define AUDIO_PLAYER_CONFIG_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define AUDIO_MODULE_NAME "DFPlayer Mini"

// Lop bao (wrapper) cho module DFPlayer Mini, giao tiep qua SoftwareSerial
// Can thu vien: DFRobotDFPlayerMini (Library Manager)
class AudioPlayer
{
private:
    SoftwareSerial serial;
    DFRobotDFPlayerMini player;
    bool connected;

public:
    // rxPin: chan Arduino nhan du lieu (noi vao TX cua DFPlayer)
    // txPin: chan Arduino gui du lieu (noi vao RX cua DFPlayer, nen qua tro 1k)
    AudioPlayer(uint8_t rxPin, uint8_t txPin)
        : serial(rxPin, txPin), connected(false) {}

    bool begin()
    {
        serial.begin(9600);
        connected = player.begin(serial);
        if (connected)
        {
            player.setTimeOut(500);
        }
        return connected;
    }

    // Bao trang thai ket noi qua LED: tat LED neu OK, nhay LED neu loi
    // He thong van chay tiep khi khong co DFPlayer (chi mat am thanh)
    void checkConnection(uint8_t ledPin)
    {
        if (connected)
        {
            digitalWrite(ledPin, LOW);
            Serial.println("DFPlayer connected.");
            return;
        }

        Serial.println("DFPlayer not found! Check wiring and SD card.");
        for (uint8_t i = 0; i < 10; i++)
        {
            digitalWrite(ledPin, HIGH);
            delay(100);
            digitalWrite(ledPin, LOW);
            delay(100);
        }
    }

    // Am luong theo phan tram 0-100, DFPlayer dung thang 0-30
    void setVolume(uint8_t percent)
    {
        if (percent > 100)
        {
            percent = 100;
        }
        if (connected)
        {
            player.volume(map(percent, 0, 100, 0, 30));
        }
    }

    // Phat file <track>.mp3 trong thu muc <folder> tren the nho
    // VD: playFolder(1, 3) -> phat /01/003.mp3
    void playFolder(uint8_t folder, uint8_t track)
    {
        if (connected)
        {
            player.playFolder(folder, track);
        }
    }

    bool isConnected() const { return connected; }
};

#endif
