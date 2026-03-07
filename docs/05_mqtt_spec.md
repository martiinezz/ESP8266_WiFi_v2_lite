# Milestone 5: Minimal MQTT Service Implementation

## Topic Schema

### Publish (Status)

All status topics are published as **retained**.

| Topic | Description | Values |
| :--- | :--- | :--- |
| `<base>/status/state` | EVSE State | 1: Ready, 2: Charging, 3: Waiting, etc. |
| `<base>/status/amp` | Current charging Amps | Float (e.g., 16.0) |
| `<base>/status/pilot` | Pilot Current Limit | Integer (e.g., 32) |
| `<base>/status/wh` | Total Watt-hours | Integer |
| `<base>/status/rssi` | Wi-Fi Signal Strength | Integer (dBm) |
| `<base>/status/uptime` | Device Uptime | Seconds |
| `<base>/status/mqtt_connected` | MQTT Connection Status | 0: Disconnected (LWT), 1: Connected |
| `<base>/status/wifi_connected` | Wi-Fi Connection Status | 0: Disconnected, 1: Connected |

### Subscribe (Commands)

| Topic | Purpose | Payload |
| :--- | :--- | :--- |
| `<base>/cmd/start` | Start/Resume charging | (Any) |
| `<base>/cmd/pause` | Pause charging | (Any) |
| `<base>/cmd/current` | Set charging current | Integer (Amps) |
| `<base>/rapi/in` | Send raw RAPI command | String (e.g., `$GC`) |

### Publish (Command Response)

| Topic | Description |
| :--- | :--- |
| `<base>/rapi/out` | Response to raw RAPI command | String (e.g., `$OK 16`) |

## Implementation Details

- **LWT (Last Will and Testament):** The topic `<base>/status/mqtt_connected` is set as the LWT with a value of `0`.
- **Async Execution:** Commands received via MQTT are executed asynchronously using the RAPI queue.
- **Robustness:** The MQTT client handles broker restarts and Wi-Fi drops automatically.
