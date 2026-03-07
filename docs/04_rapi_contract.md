# Milestone 4: Stabilize Core EVSE Serial/RAPI Layer

## RAPI Implementation Strategy

- **Non-blocking:** All RAPI commands in `update_rapi_values()` and `handleRapiRead()` are now purely asynchronous using `rapiSender.sendCmd` with callbacks.
- **Isinglated Logic:** RAPI status polling is encapsulated in `update_rapi_values()`, which cycles through a set of commands to avoid overwhelming the serial link.
- **State Caching:** The latest values for `state`, `amp`, `voltage`, `temp`, etc., are cached in global variables for immediate access by the web UI and MQTT.

## Polled Commands

| Command | Purpose | Tokens Parsed |
| :--- | :--- | :--- |
| `$GE` | Get Pilot and Flags | Pilot, Flags (Service, Diode, Vent, Ground, Relay, etc.) |
| `$GS` | Get Status | EVSE State, Session Time |
| `$GG` | Get Current/Voltage | Amps, Volts |
| `$GP` | Get Temperatures | Temp1, Temp2, Temp3 |
| `$GU` | Get Usage | Watt-seconds, Watt-hours total |
| `$GF` | Get Fault Counters | GFCI, No Ground, Stuck Relay |

## Command/Response Matrix

| Action | RAPI Command |
| :--- | :--- |
| Start/Resume | `$FE` (Enable) or `$ST` (Set Timer) |
| Pause | `$FS` (Sleep) |
| Set Current | `$SC XX` |
| Get Version | `$GV` |

## Boot Behavior

1. `setup()` calls `input_setup()` to register state change callbacks.
2. `loop()` waits for `OpenEVSE.isConnected()`.
3. Once connected, `handleRapiRead()` performs an initial one-time read of all static values (version, scale, offset, etc.).
4. `update_rapi_values()` is called every 2 seconds to poll dynamic status.
