# Hardware

The Flipper Zero is built around a single dual-core microcontroller plus a handful of
purpose-built radio/reader front ends, a tiny monochrome display, and a D-pad. There is no
general-purpose networking and no touchscreen — design decisions should start from that.

## Core

- **MCU**: STM32WB55RG — one ARM Cortex-M4 application core at 64 MHz and one Cortex-M0+
  core dedicated to the radio stack (BLE). 1 MB flash, 256 KB SRAM, shared between firmware,
  system services, and whatever app is currently running.
- **Battery**: ~2000 mAh LiPo, charged over USB-C. Radio transmission and the display
  backlight are the main power draws.
- **USB-C**: charging, serial console/logging, firmware flashing, and device-mode USB (e.g.
  acting as a USB HID keyboard or mass-storage device).

## Display and input

- **Display**: 128×64 monochrome LCD, driven over SPI. 1-bit per pixel — no grayscale, no
  color, no video.
- **Input**: a 5-way D-pad (up/down/left/right/OK/center) plus a separate Back button. No
  touchscreen, no pointer device.
- **Feedback**: a small vibration motor and a piezo speaker, used for short confirmation
  buzzes/tones (e.g. on a successful scan) rather than for audio playback.

## Radios and readers

- **Sub-GHz transceiver**: CC1101, covering regional ISO-M bands in the 300–928 MHz range
  (exact sub-bands depend on device region). Used to capture, analyze, and replay
  fixed-code and rolling-code RF remotes (garage doors, gates, some car fobs, etc.).
- **High-frequency RFID/NFC**: ST25R3916 reader/emulator at 13.56 MHz, covering
  ISO14443A/B, ISO15693, FeliCa, and Mifare Classic/Ultralight/NTAG family tags, with the
  usual caveats around proprietary crypto (see [limitations.md](limitations.md)).
- **Low-frequency RFID**: a dedicated 125 kHz analog front end and coil for reading and
  emulating LF proximity cards (EM4100, HID Prox, and similar).
- **Infrared**: an IR receiver/transmitter pair for capturing and replaying remote-control
  signals, with a built-in database of common universal-remote codes.
- **iButton**: a 1-Wire contact probe for Dallas/Maxim iButton keys.
- **Bluetooth Low Energy**: provided by the STM32WB55's integrated BLE stack; used for the
  companion mobile app and for BLE HID use cases.

## Storage and expansion

- **microSD slot**: the primary storage for apps' own data — saved Sub-GHz captures, NFC/RFID
  dumps, IR remotes, logs, and any sizeable asset. Internal flash is small and mostly
  reserved for firmware and settings.
- **GPIO header**: up to 18 pins exposing GPIO, UART, SPI, I²C, 1-Wire, and 3.3 V/5 V power,
  for external add-on modules (e.g. an ESP32-based WiFi dev board, extra RF/NFC front ends).
  This is how the device gets capabilities it doesn't have natively, such as WiFi.
