# Milestone 8: OTA Implementation and Validation

## OTA Strategy for 2MB Flash

- **Firmware Partitioning:** The 2MB flash is divided into two 1MB slots (`eagle.flash.2m.ld`).
- **Current App Size:** ~515 KB (leaving ~500 KB headroom in each slot).
- **Update Methods:**
  - **Web OTA:** Access via `/update` in the web UI.
  - **ArduinoOTA:** Standard library for network flashing from IDE/PlatformIO.

## Validation Plan

- **Slot Fit:** Binary must be < 1MB. (Verified: current 515KB)
- **Update Success Path:** Flash over serial -> Update via Web -> Success reboot.
- **Bad Image Rejection:** Attempt to flash an invalid binary; verify rejection.
- **Interruption Recovery:** Interrupted flash should not brick the device as it should boot the old firmware from the other slot.

## Recovery Procedure

If OTA fails and the device becomes unreachable:
1. Connect via Serial (TX/RX).
2. Use PlatformIO to flash `esp01_2mb_minimal` via serial.
3. Reset device and re-provision Wi-Fi.
