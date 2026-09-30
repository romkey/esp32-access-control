# esp32-access-control

This is a hardware project for an open source, ESP32-based Wiegand reader.

At PDX Hackerspace we used RFID keys to unlock doors and access our laser cutter. We want to expand their use, to make it easy for members to print parking permits for projects, unlock the main entrance for events, claim prints on 3D printers and more. We currently use Raspberry Pis for the doors and an ESPKey for the laser cutter; this project can provide a low cost simple way to integrate with Wiegand RFID readers and possibly as the basis for replacing the aging Raspberry Pis controlling our doors.

## Hardware Overview

- dual level-shifted 5V Wiegand interfaces with wiring for data (D0, D1), LED and beep along with power
- ESP32-S3-N16R8 - ESP32-S3 with 16MB of flash and 8MB of PSRAM
- USB C port
- power and data LEDs
- Qwiic connector for I2C devices
- header for SPI TFT displays with touch controllers
- "aux" header for satellite boards with optional power to this board

