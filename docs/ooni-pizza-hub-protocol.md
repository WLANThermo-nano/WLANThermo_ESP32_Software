# Ooni Pizza Hub — BLE protocol (reverse engineered)

Determined on 2026-11-20 by capturing a real device (Linux/BlueZ + bleak), without the Ooni app.

## Advertising

| Field | Value |
|-------|-------|
| Local name | `Ooni_DT_Hub` |
| Address type | random static (example `E8:24:BD:13:DC:3F`) |
| Service UUIDs in advertising | **none** → the hub has to be detected by its name |
| Manufacturer data | company ID `0xFFFF`, payload = own MAC (`e824bd13dc3f`) |

## GATT

| Service | Characteristic | Properties | Purpose |
|---------|----------------|------------|---------|
| `69400001-b5a3-f393-e0a9-e50e24dcca99` | `69400002-…` | notify | **measurement data** (1 Hz) |
| | `69400003-…` | write, write-without-response | unused (probably commands) |
| `7f510004-b5a3-f393-e0a9-e50e24dcca9e` | `7f510005-…` | notify | no data observed so far |
| | `7f510006-…` | write, write-without-response | unused |

No pairing, no login and no start command required — the hub starts sending as soon as notifications
on `69400002` are enabled. `7f510005` does not need to be subscribed.

**Quirk:** If notifications (CCCD write) on `69400002` are enabled right after connecting, the hub
often rejects the write with ATT error `0x0E` (Unlikely Error) and drops the connection.
Waiting after the connection is established fixes this:

| Test (5 connects each) | Success |
|------------------------|---------|
| enable notifications immediately | 1/5 |
| wait 1.5 s before enabling | 5/5 |

The NRF52 driver (`BleTemperatureOoni`) waits 2 s.

## Data frame (`69400002`, 22 bytes, little endian)

```
Offset  0    1    2-3     4-5     6-7   8-9   10   11   12-17          18-19  20-21
        flg  ??   stone1  stone2  P1    P2    bat  ??   MAC (reversed) ??     ??
Example:
        18   00   00 00   00 00   00 00 16 00 64   01   3f dc 13 bd 24 e8 18 00 00 00
```

| Offset | Type | Meaning | Status |
|--------|------|---------|--------|
| 0 | uint8 | bitmask of plugged probes: bit0 = stone 1, bit1 = stone 2, bit2 = P1, bit3 = P2; bit4 always set | bit2/bit3 verified, bit0/1 assumed |
| 1 | uint8 | always `0x00` | unknown |
| 2–3 | int16 | temperature stone 1 in °C (whole degrees) | assumed (same scheme) |
| 4–5 | int16 | temperature stone 2 in °C | assumed (same scheme) |
| 6–7 | int16 | temperature P1 (meat, left side) in °C | **verified** |
| 8–9 | int16 | temperature P2 (meat, right side) in °C | **verified** |
| 10 | uint8 | battery in % (`0x64` = 100) | plausible |
| 11 | uint8 | always `0x01` (unit °C?) | unknown |
| 12–17 | 6 bytes | own MAC address, reversed byte order | verified |
| 18–19 | uint16 | constant 24 (hub internal temperature or version?) | unknown |
| 20–21 | — | always `0x0000` | unknown |

Probe not plugged: its bit in offset 0 is cleared, value = 0.

## Verification

Test run 17:54–18:02: probe on P2 moved between 20 °C and 38 °C, unplugged (`flg` 0x18 → 0x10, P2 → 0),
plugged into P1 (`flg` → 0x14, P1 = 36 °C) and cooled down to 20 °C. All decoded values match the
temperatures shown by the hub.

Afterwards verified end to end on a WLANThermo Nano V3: the hub shows up with 4 channels and P1/P2
report correct values.

## Open points

- Confirm stone probes (offset 2–5, bit0/1) with real probes, including high temperatures (>255 °C → high byte)
- Behaviour after switching the hub to °F (offset 11?)
- Negative temperatures (sign)
- Meaning of offset 18–19
