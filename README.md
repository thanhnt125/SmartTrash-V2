# SmartTrash V2

Smart trash bin on **Arduino Uno**: the lid opens automatically when someone approaches, a voice prompt tells them which waste type this bin is for, and LEDs plus a buzzer warn when the bin is full. A **Python desktop app** shows the fill level and controls the bin over USB serial.

## Features

- **Auto lid** — HC-SR04 #1 detects an object within 35 cm; servo SG90 opens the lid (10° → 170°), keeps it open while the object is still there, then closes and says "Xin cảm ơn".
- **Voice guidance** — DFPlayer Mini plays one of 5 waste-type prompts: default, organic, inorganic, recyclable, batteries.
- **Fill-level alert** — HC-SR04 #2 measures the trash level: green (empty), yellow (medium), red + buzzer + voice warning (full).
- **Button with interrupt** — short press on BTN1 cycles the waste type; ISR + 200 ms debounce, long-press support.
- **Persistent setting** — the selected waste type is stored in EEPROM and restored after power loss.
- **Re-open cooldown** — the lid must stay closed ≥ 1 s before opening again, avoiding rapid open/close cycles.

## Hardware

| Part | Pin |
|---|---|
| Servo SG90 (lid) | D5 |
| HC-SR04 #1 (approach) TRIG / ECHO | D6 / D4 |
| HC-SR04 #2 (fill level) TRIG / ECHO | A4 / A3 |
| LED red / yellow / green | D11 / D12 / A5 |
| Buzzer | D10 |
| DFPlayer Mini RX / TX | D7 / D8 |
| Button BTN1 | D2 (INT0, INPUT_PULLUP) |

All pins and timings are in [`firmware/SmartTrashV2/src/Config.h`](firmware/SmartTrashV2/src/Config.h).

## Firmware structure

```
firmware/SmartTrashV2/
├── SmartTrashV2.ino        setup/loop, EEPROM, button interrupt, serial command handler
└── src/
    ├── Config.h            pins, thresholds, timings, shared ultrasonic helper
    ├── AudioPlayerConfig.h DFPlayer Mini wrapper (SoftwareSerial)
    ├── Button.h/.cpp       interrupt-driven button with debounce, short/long press callbacks
    └── SmartTrash.h/.cpp   lid state machine, auto/manual mode, sensors, LEDs, audio prompts
```

Firmware is written in C++ with an object-oriented structure: `SmartTrash` owns the lid logic and sensors, `Button` handles input via callbacks.

### Audio files

Copy MP3 files to the DFPlayer SD card in folder `01/`:

| File | Content |
|---|---|
| 001.mp3 | Welcome — "Thùng Rác STEM" |
| 002.mp3 | Hãy bỏ rác vào thùng |
| 003.mp3 | Rác hữu cơ |
| 004.mp3 | Rác vô cơ |
| 005.mp3 | Rác tái chế |
| 006.mp3 | Các loại pin và ắc quy |
| 007.mp3 | Xin cảm ơn |
| 008.mp3 | Thùng rác đầy |

## PC app

`pc-app/app.py` — dark-mode desktop dashboard built with customtkinter, reading serial data in a background thread.

- Real-time fill level bar (green / yellow / red)
- Lid status, waste-type dropdown
- Buttons: Auto mode, Manual mode, Open lid, Close lid

```bash
cd pc-app
pip install -r requirements.txt
python app.py      # edit PORT = 'COM3' to match your Arduino
```

Serial protocol (9600 baud):

| Direction | Message |
|---|---|
| Arduino → PC | `Volume: <cm>`, `Lid opened`, `Lid closed`, `TYPE: <0-4>`, `SYSTEM_MODE: AUTO/MANUAL` |
| PC → Arduino | `O` open, `C` close, `A` auto, `M` manual, `0`–`4` waste type |

- **Auto mode** (default): the approach sensor controls the lid.
- **Manual mode**: the approach sensor is ignored; the lid only moves on `O` / `C`. Sending `O` or `C` while in auto mode switches to manual first, so the sensor and the app never fight over the lid.
- Switching back to auto closes the lid if it was left open.
- On boot the Arduino sends `TYPE` and `SYSTEM_MODE` so the app syncs immediately; the fill-level sensor keeps reporting in both modes.

## Build & upload

1. Open `firmware/SmartTrashV2/SmartTrashV2.ino` in Arduino IDE (keep the `src/` folder next to it).
2. Install libraries: **Servo** (built-in), **DFRobotDFPlayerMini**.
3. Board: *Arduino Uno* → Upload.

## Future work

- Camera + CNN for automatic waste classification
- Wi-Fi (ESP8266/ESP32) and a web dashboard
- Mobile app replacing the PC app

## Tools

Arduino IDE · C++ · Python · customtkinter · pyserial
