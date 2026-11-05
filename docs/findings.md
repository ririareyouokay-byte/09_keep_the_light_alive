# M7 — Initial Build Findings

## Project
Keep the Light Alive — ESP32 Interactive Survival Game

## Initial Build
The first version of the project was created using PlatformIO and the Arduino framework for ESP32.

The game is designed to connect a physical button and LED to an interactive webpage.

## Findings

### Finding 1 — Firmware Compilation
The firmware was compiled successfully, and PlatformIO generated a firmware binary.

### Finding 2 — Upload Failure
The initial upload failed because PlatformIO automatically selected `/dev/cu.headphonesOn` instead of the ESP32 USB serial port.

Error message:
`Failed to connect to ESP32: No serial data received.`

### Finding 3 — Hardware Testing
The earlier Mood Light prototype successfully controlled an LED using the webpage and the onboard IO0 button.

The new survival game has not yet been fully tested on the ESP32.

## Planned Fixes
1. Identify the correct ESP32 USB serial port.
2. Upload the new firmware.
3. Test the physical button and LED.
4. Verify that the webpage updates the energy level and survival time.

## Reflection
The initial build revealed a serial communication problem during firmware upload. This needs to be resolved before testing the complete game.