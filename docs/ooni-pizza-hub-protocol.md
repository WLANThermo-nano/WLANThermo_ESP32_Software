# Ooni Pizza Hub — BLE-Protokoll (Reverse Engineering)

Ermittelt am 2026-11-20 durch Mitschnitt am echten Gerät (Linux/BlueZ + bleak), ohne Ooni-App.

## Advertising

| Feld | Wert |
|------|------|
| Local Name | `Ooni_DT_Hub` |
| Adresstyp | random static (Beispiel `E8:24:BD:13:DC:3F`) |
| Service-UUIDs im Advertising | **keine** → Erkennung muss über den Namen erfolgen |
| Manufacturer Data | Company-ID `0xFFFF`, Payload = eigene MAC (`e824bd13dc3f`) |

## GATT

| Service | Characteristic | Properties | Funktion |
|---------|----------------|------------|----------|
| `69400001-b5a3-f393-e0a9-e50e24dcca99` | `69400002-…` | notify | **Messdaten** (1 Hz) |
| | `69400003-…` | write, write-without-response | unbenutzt (vermutl. Kommandos) |
| `7f510004-b5a3-f393-e0a9-e50e24dcca9e` | `7f510005-…` | notify | bisher keine Daten beobachtet |
| | `7f510006-…` | write, write-without-response | unbenutzt |

Kein Pairing, kein Login, kein Startkommando nötig — nach Aktivieren der Notification auf `69400002` sendet der Hub sofort.

**Eigenheit:** Das Aktivieren der Notification (CCCD-Write) auf `69400002` wird gelegentlich mit ATT-Fehler `0x0E` (Unlikely Error) abgelehnt; danach trennt der Hub die Verbindung. Ein erneuter Verbindungsversuch funktioniert in der Regel. In den erfolgreichen Läufen wurde vorher `7f510005` abonniert und ~1 s gewartet — ob das nötig ist, ist nicht abschließend geklärt.

## Datenpaket (`69400002`, 22 Byte, Little Endian)

```
Offset  0    1    2-3     4-5     6-7   8-9   10   11   12-17          18-19  20-21
        flg  ??   Stein1  Stein2  P1    P2    bat  ??   MAC (reversed) ??     ??
Beispiel:
        18   00   00 00   00 00   00 00 16 00 64   01   3f dc 13 bd 24 e8 18 00 00 00
```

| Offset | Typ | Bedeutung | Status |
|--------|-----|-----------|--------|
| 0 | uint8 | Bitmaske gesteckter Fühler: Bit0 = Stein 1, Bit1 = Stein 2, Bit2 = P1, Bit3 = P2; Bit4 immer gesetzt | Bit2/Bit3 verifiziert, Bit0/1 vermutet |
| 1 | uint8 | immer `0x00` | unbekannt |
| 2–3 | int16 | Temperatur Stein 1 in °C (ganzzahlig) | vermutet (Schema) |
| 4–5 | int16 | Temperatur Stein 2 in °C | vermutet (Schema) |
| 6–7 | int16 | Temperatur P1 (Fleisch, seitlich links) in °C | **verifiziert** |
| 8–9 | int16 | Temperatur P2 (Fleisch, seitlich rechts) in °C | **verifiziert** |
| 10 | uint8 | Akku in % (`0x64` = 100) | plausibel |
| 11 | uint8 | immer `0x01` (evtl. Einheit °C?) | unbekannt |
| 12–17 | 6 Byte | eigene MAC-Adresse, Byte-Reihenfolge umgekehrt | verifiziert |
| 18–19 | uint16 | konstant 24 (evtl. Hub-Innentemperatur oder Version) | unbekannt |
| 20–21 | — | immer `0x0000` | unbekannt |

Nicht gesteckter Fühler: Bit in Offset 0 gelöscht, Wert = 0.

## Verifikation

Testreihe 17:54–18:02: Fühler an P2 zwischen 20 °C und 38 °C bewegt, abgezogen (`flg` 0x18 → 0x10, P2 → 0),
an P1 gesteckt (`flg` → 0x14, P1 = 36 °C) und auf 20 °C abgekühlt. Alle dekodierten Werte stimmen mit der
Anzeige überein.

## Offen

- Steinfühler (Offset 2–5, Bit0/1) mit echten Fühlern bestätigen, inkl. hoher Temperaturen (>255 °C → High-Byte)
- Verhalten bei °F-Umstellung (Offset 11?)
- Negative Temperaturen (Vorzeichen)
- Bedeutung von Offset 18–19
