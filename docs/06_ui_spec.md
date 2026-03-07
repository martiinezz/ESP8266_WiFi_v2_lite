# Milestone 6: New Lightweight Web UI

## UI Design Goals

- **Single-Page Application (SPA):** Both Dashboard and Settings are in one file (`home.html`).
- **Minimalist Aesthetic:** Clean, card-based layout using basic CSS.
- **Framework-free:** Pure vanilla JavaScript and CSS for maximum flash savings.
- **Responsive:** Mobile-friendly by default using a simple flex/grid-like layout.

## Screens

### Dashboard
- **Live Status:** Displays EVSE State, charging Amps, Pilot limit, and Energy (Wh).
- **Controls:**
  - Resume (RAPI `$FE`)
  - Pause (RAPI `$FS`)
  - Set Current (RAPI `$SC XX`)

### Settings
- **Wi-Fi:** SSID and Password configuration.
- **MQTT:** Server and Base Topic configuration.
- **System:** Restart, Factory Reset, and Link to OTA update.

## Implementation Details

- **WebSockets:** Live updates are pushed via `/ws`.
- **Backup Polling:** If the WebSocket is disconnected or slow, the UI falls back to polling `/status` every 5 seconds.
- **Dynamic Content:** Pages are swapped using the `hidden` class to avoid page reloads.
- **Size:** The entire `home.html` is ~5 KB (including CSS and JS), compared to the previous multi-file UI which was > 300 KB.
