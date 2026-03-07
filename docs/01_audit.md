# Milestone 1: Baseline Audit and Build Capture

## Baseline Build Report

- **Target environment:** `openevse`
- **MCU:** ESP8266 (ESP-12E in current config)
- **Flash:** 1MB (804,864 bytes used)
- **RAM:** 80KB (35,956 bytes used)
- **Status:** SUCCESS

Note: The current build is targeting 1MB flash, while the goal is 2MB for ESP-01.

## Module Dependency Map

- **Core Framework:** Arduino for ESP8266
- **Web Server:** ESP Async WebServer, ESPAsyncTCP
- **JSON:** ArduinoJson 6.15.1
- **MQTT:** PubSubClient 2.6
- **Config:** ConfigJson 0.0.6
- **EVSE Logic:** OpenEVSE 0.0.14
- **HAL:** ESPAL 0.0.4

## Feature Keep/Remove/Rewrite Table

| Area | Keep | Remove | Rewrite | Notes |
| :--- | :--- | :--- | :--- | :--- |
| Wi-Fi Station Mode | Yes | No | No | |
| AP Fallback/Provisioning | Yes | No | No | |
| OpenEVSE Serial RAPI | Yes | No | No | `rapi_transport`, `rapi_parser` in `openevse.h/cpp` |
| MQTT Service | Yes | No | Maybe | Keep as only external service. |
| OTA Update | Yes | No | No | `ota.h/cpp`, must stay functional. |
| Web UI | No | No | Yes | Replace heavy legacy GUI with lightweight single-page. |
| Emoncms | No | Yes | No | `emoncms.h/cpp` to be removed. |
| Eco/Solar Divert | No | Yes | No | `divert.h/cpp` to be removed. |
| OhmConnect | No | Yes | No | `ohm.h/cpp` to be removed. |
| LCD | Yes | No | No | `lcd.h/cpp`. |
| Input | Yes | No | No | `input.h/cpp`. |

## Flash/RAM Impact

Current binary is ~805 KB.
The embedded UI assets in `src/web_static/` are large:
- `web_server.lib.js.h`: ~179 KB
- `web_server.zones.json.h`: ~97 KB
- `web_server.home.html.h`: ~40 KB
- `web_server.home.js.h`: ~37 KB

Total UI assets alone are > 350 KB. Removing these and replacing with a tiny UI will be critical for the 2MB ESP-01 target with OTA.
OTA requires two copies of the firmware, so each copy must be < 1MB on a 2MB flash.

## Initial Observations

- The project uses a custom `StaticFileWebHandler` in `src/web_server_static.cpp` to serve the embedded headers.
- MQTT topics are currently published every 30 seconds.
- SSE-like events are handled in `web_server.cpp`.
