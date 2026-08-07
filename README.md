# sc2-steering-wheel

ESP32 firmware for the steering-wheel HMI.

The board sends driver IO on CAN. The display shows various data about the cars modes, speed, bms, etc. The odometer value is stored in NVS.

## Build

1. Run `git submodule update --init`.
2. Run `pio run`.

## Tasks

`main.cpp` starts these FreeRTOS tasks:

| Task | Core | Rate | Role |
|------|------|------|------|
| `ioTask` | 0 | 100 Hz | Read pedals and buttons |
| `canRxTask` | 0 | about 1 ms | Read BMS and telem frames |
| `canTxTask` | 0 | 50 Hz | Send driver inputs |
| `displayTask` | 1 | 20 Hz | Update odometer and TFT |

## Modules

1. `IOManagement` reads pins, sets pedal scale, toggles buttons, and counts laps.
2. `canSteering` receives BMS, speed, and fault frames. It sends `0x300` through `0x304`.
3. `display` draws the UI.
4. `odometer` stores tenths of a mile in NVS.

## CAN IDs

Transmit:

- `0x300` digital pack
- `0x301` regen
- `0x302` throttle
- `0x303` eco or power mode
- `0x304` hazards

Receive:

- `0x108` and `0x109` BMS
- `0x208` mph from the PDC
- `0x001` BPS fault from powertrain (`0x01` means fault)

## Pins

| Function | Pin |
|----------|-----|
| Throttle ADC | 12 |
| Regen button | 35 |
| Left blink | 34 |
| Right blink | 5 |
| Direction | 32 |
| Horn | 33 |
| Hazards | 14 |
| Drive mode | 4 |
| Lap increment | 26 |
| Lap decrement | 27 |
| CAN TX | 21 |
| CAN RX | 22 |
| TFT CS | 15 |
