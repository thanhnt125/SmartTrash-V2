#include <EEPROM.h>
#include "src/Config.h"
#include "src/AudioPlayerConfig.h"
#include "src/Button.h"
#include "src/SmartTrash.h"


AudioPlayer audio(DFPLAYER_RX, DFPLAYER_TX);
Button btn1(BTN_1_PIN);
SmartTrash smartTrash(TRASH_SERVO_PIN, TRASH_TRIG_PIN, TRASH_ECHO_PIN, TRASH_TRIG_VOL_PIN, TRASH_ECHO_VOL_PIN,
                      TRASH_RED_PIN, TRASH_YL_PIN, TRASH_GR_PIN, TRASH_BUZ_PIN);

uint8_t currentTrashType = 0;

void loadTrashTypeFromEEPROM()
{
    uint8_t savedType = EEPROM.read(EEPROM_SOUND_ADDR);
    if (savedType < 5)
    {
        currentTrashType = savedType;
    }
    else
    {
        currentTrashType = 0;
        EEPROM.update(EEPROM_SOUND_ADDR, currentTrashType);
    }
}

void saveTrashTypeToEEPROM()
{
    EEPROM.update(EEPROM_SOUND_ADDR, currentTrashType);
}

// Gui loai rac hien tai len app PC de dong bo dropdown
void sendTrashType()
{
    Serial.print("TYPE: ");
    Serial.println(currentTrashType);
}

// Gui toan bo trang thai len app PC (khi khoi dong)
void sendStatus()
{
    sendTrashType();
    Serial.println(smartTrash.isAutoMode() ? "SYSTEM_MODE: AUTO" : "SYSTEM_MODE: MANUAL");
}

// Doi loai rac: dung chung cho nut bam va lenh tu app
void changeTrashType(uint8_t type)
{
    if (type >= 5)
    {
        return;
    }
    currentTrashType = type;
    smartTrash.setSoundIndex(currentTrashType);
    saveTrashTypeToEEPROM();
    sendTrashType();
}

void btn1InterruptWrapper()
{
    btn1.handleInterrupt();
}

void onBtn1ShortPress()
{
    changeTrashType((currentTrashType + 1) % 5);

    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
}

// Doc lenh 1 ky tu tu app PC (Serial 9600 baud)
//   'A' = che do tu dong      'M' = che do thu cong
//   'O' = mo nap              'C' = dong nap
//   '0'..'4' = chon loai rac
// Lenh O/C tu dong chuyen sang che do thu cong de cam bien khong tranh quyen dieu khien nap
void handleSerialCommands()
{
    while (Serial.available() > 0)
    {
        char cmd = Serial.read();

        switch (cmd)
        {
        case 'A':
            smartTrash.setAutoMode(true);
            break;

        case 'M':
            smartTrash.setAutoMode(false);
            break;

        case 'O':
            if (smartTrash.isAutoMode())
            {
                smartTrash.setAutoMode(false);
            }
            smartTrash.openLid();
            break;

        case 'C':
            if (smartTrash.isAutoMode())
            {
                smartTrash.setAutoMode(false);
            }
            smartTrash.closeLid();
            break;

        default:
            if (cmd >= '0' && cmd <= '4')
            {
                changeTrashType(cmd - '0');
            }
            // Bo qua ky tu khac (\r, \n, ...)
            break;
        }
    }
}

void setup()
{
    wdt_disable();
    Serial.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);

    btn1.begin();
    btn1.setShortPressHandler(onBtn1ShortPress);
    attachInterrupt(digitalPinToInterrupt(BTN_1_PIN), btn1InterruptWrapper, FALLING);

    Serial.print("Initializing ");
    Serial.print(AUDIO_MODULE_NAME);
    Serial.println("...");
    digitalWrite(LED_BUILTIN, HIGH);
    audio.begin();
    audio.checkConnection(LED_BUILTIN);
    audio.setVolume(DEFAULT_VOLUME);

    loadTrashTypeFromEEPROM();
    smartTrash.setSoundIndex(currentTrashType);

    Serial.println("Audio ready!");
    Serial.println("SmartTrash only mode.");

    sendStatus();
}

void loop()
{
    handleSerialCommands();
    btn1.update();
    smartTrash.run(&audio);
    delay(10);
}
