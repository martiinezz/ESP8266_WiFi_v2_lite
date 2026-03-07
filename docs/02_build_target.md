# Milestone 2: Define the 2MB ESP-01 Build Target

## Dedicated Target: `esp01_2mb_minimal`

A new environment has been added to `platformio.ini` to target the ESP-01 with 2MB flash.

### Configuration

- **Board:** `esp01_1m` (base)
- **Flash Layout:** 2MB (1MB App + 1MB OTA/App)
- **LD Script:** `eagle.flash.2m.ld`
- **Flash Mode:** `dout` (most compatible for ESP-01)
- **Frequency:** 80MHz

### Build Flags

- `ENABLE_OTA`: Enabled
- `WIFI_LED=0`: Defined (standard for ESP-01)
- `BUILD_TAG`: `2.9.1.minimal`

### Initial Size Report

- **Flash Usage:** 814,792 bytes (~78% of 1MB slot)
- **RAM Usage:** 36,124 bytes (~44% of 80KB)

### OTA Strategy

With a 2MB flash and `eagle.flash.2m.ld`, the flash is partitioned into two 1MB slots for OTA. The current binary size (814KB) fits within this limit, but it's relatively tight (leaving ~200KB for growth). Milestone 3 (Feature Reduction) will be critical to increase this headroom.

### Release vs Debug

- **Release:** Minimal logs, no extra debug flags.
- **Debug:** (Can be enabled via `common.debug_flags` if needed during development).

## Flashing Instructions

To flash for the first time via serial:
```bash
~/.platformio/penv/bin/pio run -e esp01_2mb_minimal -t upload
```

To update via OTA (once running):
Use the web interface OTA upload feature.
