# Milestone 3: Remove Non-Essential Services and Dead UI Dependencies

## Feature Reduction Summary

The following services and their associated code paths have been removed:
- **Emoncms Integration:** Removed `emoncms.cpp/h` and all references in `app_config`, `web_server`, and `src.ino`.
- **Eco/Solar Divert Logic:** Removed `divert.cpp/h` and all references in `app_config`, `web_server`, `mqtt`, `input`, and `src.ino`.
- **OhmConnect Integration:** Removed `ohm.cpp/h` and all references in `app_config`, `web_server`, and `src.ino`.
- **Legacy API:** Removed `handleUpdate` and other legacy blocks.

## UI Asset Pruning

The heavy legacy UI assets have been removed/replaced with minimal stubs:
- Removed `web_server.lib.js.h` (~179 KB)
- Removed `web_server.zones.json.h` (~97 KB)
- Removed `web_server.emoncms.jpg.h`, `web_server.ohm.jpg.h`, `web_server.assets.js.h`, `web_server.home.js.h`.
- Replaced `home.html` and `wifi_portal.html` with minimal stubs.

## Size Comparison

| Metric | Baseline (M2) | After Reduction (M3) | Change |
| :--- | :--- | :--- | :--- |
| **Flash Usage** | 814,792 bytes (78.0%) | 515,076 bytes (49.3%) | -299,716 bytes (~37% reduction) |
| **RAM Usage** | 36,124 bytes (44.1%) | 34,688 bytes (42.3%) | -1,436 bytes |

## HEADROOM for OTA

With the current size of ~515 KB, we have ~500 KB of free space in the 1MB OTA slot. This is excellent for long-term maintainability on the 2MB ESP-01.

## Next Steps

Milestone 4: Stabilize core EVSE serial/RAPI layer.
