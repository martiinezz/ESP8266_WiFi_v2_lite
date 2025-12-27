# OpenEVSE WiFi LITE Version

## Overview
The LITE version of OpenEVSE WiFi firmware is a streamlined build that excludes non-essential features to reduce memory footprint and complexity while maintaining core functionality.

## Build Variants

### 1. openevse_lite (ESP12e - 4MB flash)
Standard LITE version for ESP12e and similar modules with 4MB flash.

### 2. openevse_lite_1m (ESP-01s - 1MB flash)
Optimized LITE version specifically for ESP-01s with only 1MB flash memory.
Uses 512KB for sketch and 512KB for filesystem with low-memory LWIP stack.

## Features Excluded in LITE Version
The following modules are NOT compiled in the LITE version:
- divert.cpp/divert.h - Solar PV divert functionality
- emoncms.cpp/emoncms.h - EmonCMS data logging
- mqtt.cpp/mqtt.h - MQTT protocol support
- ohm.cpp/ohm.h - Ohm integration

## Features Included in LITE Version
The LITE version KEEPS all essential features:
- OTA Updates - Over-the-air firmware updates
- WiFi Management - AP and STA modes
- Web Server - Configuration interface
- RAPI Communication - OpenEVSE control protocol
- LCD Support - Display functionality
- Input Handling - User input processing
- HTTP API - RESTful API endpoints

## Building the LITE Version

### For ESP12e (4MB flash):
```bash
pio run -e openevse_lite
pio run -e openevse_lite -t upload
```

### For ESP-01s (1MB flash):
```bash
pio run -e openevse_lite_1m
pio run -e openevse_lite_1m -t upload
```

### Using PlatformIO IDE:
1. Open the project in VSCode with PlatformIO extension
2. Select the environment: 
   - env:openevse_lite (for ESP12e)
   - env:openevse_lite_1m (for ESP-01s)
3. Click Build or Upload

## ESP-01s Specific Configuration

The ESP-01s build uses:
- Board: esp01_1m
- Flash mode: DOUT
- Linker script: eagle.flash.1m512.ld (512KB sketch + 512KB FS)
- LWIP: Low memory variant
- Upload speed: 115200 baud
- Reset method: nodemcu

## Memory Layout for ESP-01s (1MB)
```
Total: 1MB (1024KB)
- Sketch: ~512KB (program code)
- Filesystem: ~512KB (SPIFFS for config)
- Bootloader and system reserved
```

## Memory Savings
By excluding modules, the LITE version provides:
- Reduced flash usage - Smaller firmware binary
- Lower RAM consumption - More free heap memory
- Faster compilation - Fewer files to compile
- Simplified codebase - Easier to maintain and debug

## API Endpoints Removed
The following HTTP endpoints are NOT available in LITE version:
- /saveemoncms
- /savemqtt
- /saveohmkey
- /divertmode
- /emoncms/describe

## Use Cases
The LITE version is ideal for:
- ESP-01s modules with limited 1MB flash
- Basic EV charging without cloud integration
- Local-only operation
- Memory-constrained deployments
- Testing and development
- Minimal attack surface

## Hardware Requirements

### Minimum (ESP-01s):
- ESP8266 ESP-01s with 1MB flash
- 3.3V power supply (stable, min 500mA)
- USB to serial adapter for programming

### Recommended (ESP12e):
- ESP8266 ESP12e/ESP12f with 4MB flash
- Better for future expansion

## Flashing ESP-01s

1. Connect ESP-01s to USB-Serial adapter:
   - VCC -> 3.3V
   - GND -> GND
   - TX -> RX
   - RX -> TX
   - GPIO0 -> GND (for flash mode)
   - CH_PD -> 3.3V

2. Flash using PlatformIO:
```bash
pio run -e openevse_lite_1m -t upload
```

3. After flashing, disconnect GPIO0 from GND and reset

## OTA Updates on ESP-01s

OTA updates work on ESP-01s but require:
- Stable WiFi connection
- Sufficient free heap memory during update
- New firmware must fit in 512KB sketch space

To perform OTA update:
1. Access web interface at http://openevse.local or device IP
2. Navigate to /update endpoint
3. Upload new .bin file
4. Device will reboot with new firmware

## Troubleshooting ESP-01s

### Flash Size Detection
If flash size is detected incorrectly, the build uses explicit flash settings:
- board_build.flash_mode = dout
- board_build.ldscript = eagle.flash.1m512.ld

### Out of Memory During Compilation
The LITE version is designed to fit, but if issues occur:
- Ensure ENABLE_LITE flag is set
- Check that excluded modules are not being compiled
- Use LWIP low memory variant (already configured)

### Upload Issues
- Use 115200 baud rate (configured in platformio.ini)
- Ensure GPIO0 is grounded during upload
- Try different USB-Serial adapters if problems persist
- Check power supply stability (ESP-01s needs stable 3.3V)

## License
Same as the main OpenEVSE WiFi project - GNU General Public License v3.0
