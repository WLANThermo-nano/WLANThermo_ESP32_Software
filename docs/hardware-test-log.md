# Hardware Test Log

Protokolle der Hardware-Tests während Phase 1 (Kernel Upgrade) und Phase 4c (ArduinoJson 7).

---

## Phase 1 — miniV3 Hardware-Test (2026-04-20)

| Bereich | Status | Anmerkung |
|---------|--------|-----------|
| Boot / Stabilität | ✅ OK | Bootloop behoben |
| Web-Interface laden | ✅ OK | Lädt nach canHandle()-Fix |
| REST-API `/data`, `/settings` | ✅ OK | Nach canHandle()-Fix |
| Settings speichern | ✅ OK | WDT-Fix bestätigt. Root Cause: Heap-Korruption durch 13× unsized `DynamicJsonBuffer`. Fix: alle auf `DynamicJsonBuffer(Settings::jsonBufferSize)` + NVS-Writes via Pending-Flags auf ConnectTask. |
| WiFi-Stabilität | ⏳ Ausstehend | Langzeittest noch nicht durchgeführt |
| Power Management | ✅ OK (2026-04-24) | Custom-Framework `2.0.17-pm-enable`, `CONFIG_PM_ENABLE=y`. `setPowerSaveMode()` fix: `min_freq_mhz` 240→40. WiFi Modem Sleep aktiv. Strommessung ausstehend. |
| BLE / NRF-Chip | ⏳ Fix implementiert, Test ausstehend | `remoteIndex`-Discovery-Fix in `Bluetooth.cpp`; kein Re-Pairing nötig |

### Fixes während des Phase-1-Tests

| Fix | Datei | Ursache |
|-----|-------|---------|
| `LogRingBuffer::write()` fehlte `return 1u` | `src/LogRingBuffer.cpp` | GCC 8.x emittiert `ILL`-Trap für fehlenden Return-Wert → `IllegalInstruction`-Crash beim ersten `Log.notice()` |
| Task-Stacks zu klein | `src/main.cpp` | FreeRTOS-Overhead in ESP-IDF 4.4.x größer als 1.x: MainTask 3000→8000 B, ConnectTask 4000→12000 B |
| Bluetooth-Task-Stack zu klein | `src/bluetooth/Bluetooth.cpp` | Gleiche Ursache: 3000→6000 B |
| `NanoWebHandler::canHandle()` fehlte `const` | `src/WebHandler.h/.cpp` | ESPAsyncWebServer 3.x deklariert virtual als `const` → kein Override → alle Endpunkte gaben leere Seite zurück |
| `NanoWebHandler::isRequestHandlerTrivial()` fehlte `const`, `boolean`→`bool` | `src/WebHandler.h` | Gleiche Ursache |
| `Wlan::setHostName()` rief `MDNS.begin()` im `async_tcp`-Kontext auf | `src/Wlan.cpp/.h` | `MDNS.begin()` blockiert in arduino-esp32 2.x mehrere Sekunden → Task-WDT. Fix: `mdnsUpdatePending`-Flag, Abarbeitung in `Wlan::update()` |
| `Bluetooth::getDevices()` sendete falsche Bitmask nach NVS-Restore | `src/bluetooth/Bluetooth.cpp` | `remoteIndex` nie in NVS gespeichert → nach Boot `remoteIndex=0xFF` → `1u<<255` → Xtensa maskiert auf 5 Bit → `requestedDevices=0x80000000` → NRF deaktiviert Probe. Fix: `remoteIndex==0xFF`-Devices aus Bitmask ausschließen |
| `handleBody()` nicht null-terminierter Buffer | `src/WebHandler.cpp` | ESPAsyncWebServer 3.x liefert Body ohne `\0` → `parseObject()` las über Puffer hinaus. Fix: null-terminierte Kopie `new uint8_t[len+1]` |
| `setBluetooth()`: fehlendes `return` + unsized `DynamicJsonBuffer` | `src/WebHandler.cpp` | UB (GCC 8.x) + Heap-Korruption. Fix: `return 1;` + `DynamicJsonBuffer(Settings::jsonBufferSize)` |
| `TemperatureGrp::saveConfig()` unsized `DynamicJsonBuffer` | `src/temperature/TemperatureGrp.cpp` | Gleiche Ursache. Fix: `DynamicJsonBuffer(Settings::jsonBufferSize)` |
| `setSystem()`: 4 NVS-Writes im `async_tcp`-Kontext | `src/WebHandler.cpp` | Blockiert async_tcp → Task-WDT. Fix: Stückelung via `processPendingSave()` — 1 Write/s in ConnectTask |
| Widespread unsized `DynamicJsonBuffer` | `WebHandler.cpp` (9×), `API.cpp`, `Wlan.cpp`, `PitmasterGrp.cpp`, `Connect.cpp`, `Bluetooth.cpp` | Heap-Korruption über 40–80s → async_tcp WDT `CORRUPTED`. Fix: alle auf `DynamicJsonBuffer(Settings::jsonBufferSize)` |
| `setPowerSaveMode()` Endlos-Retry | `src/system/SystemBase.cpp` | `CONFIG_PM_ENABLE=n` → immer `ESP_ERR_NOT_SUPPORTED`. Fix: bei Fehler `powerSaveModeSupport=false` |
| PCA9533-Startup-Pause (~170ms) | `src/display/tft/DisplayTft.cpp` | ESP-IDF I2C-Treiber: jede NACK-Transaktion ~10ms. 17 Transaktionen wenn PCA9533 nicht verbaut. Fix: `ping()`-Guard voranstellen |

### Stack-Messwerte miniV3 (Frühphase, Grundlast)

- MainTask: ~1.636 B / 8.000 B → reduzierbar auf ~2.200 B nach Langzeittest
- ConnectTask: ~1.788 B / 12.000 B → reduzierbar auf ~2.300 B nach Langzeittest
- Bluetooth-Task: Messung ausstehend

---

## Phase 1 — nanoV3 Hardware-Test (2026-04-25)

| Bereich | Status | Anmerkung |
|---------|--------|-----------|
| Boot / Stabilität | ⚠️ Teilweise — Brownout bei WiFi-Init | B18-Fix (ESP_PM_APB_FREQ_MAX) hat TG1WDT_SYS_RESET behoben. Dann: `rst:0xc (SW_CPU_RESET)` durch Brownout direkt nach `Wlan::loadConfig` beim ersten `WiFi.begin()`. Root Cause: Batterie schwach → Spannungseinbruch (~380mA Spitze). War vorher durch WDT-Crash verdeckt. Diagnose: Batterie laden; falls Brownout bleibt → `WiFi.setTxPower(WIFI_POWER_13dBm)` vor `WiFi.begin()`. |
| Übrige Funktionen | ⏳ Ausstehend | Erst nach Stabilitätsbestätigung |

---

## Phase 4c — miniV3 Hardware-Test (2026-04-22)

| Bereich | Status | Anmerkung |
|---------|--------|-----------|
| Pitmaster (Servo, Lüfter) | ✅ OK | |
| Pitmaster SSR | ✅ OK | DAC/LEDC-Konflikt auf GPIO 25 gefixt (B17) |
| System (Sprache, Einheit) | ✅ OK | |
| IoT (Cloud) | ✅ OK | |
| MQTT | ⏳ Ausstehend | Keine Gegenstelle verfügbar |
| Benachrichtigungen (Telegram, Pushover, Alarmton) | ✅ OK | |
| Bluetooth | ✅ OK | |
| Temperaturmessung + Typ K | ✅ OK | |
| Kanaleinstellungen + Temperaturschwellen | ✅ OK | |
| Batterieerkennung | ✅ OK | Ladeendzustand noch nicht getestet |
| Recovery-Mode starten | ✅ OK | B33 gefixt (2026-04-25) |
| Recovery-Seite (Auto-Redirect) | ✅ OK | B15+B33 gefixt |
| Recovery-Export | ✅ OK | B14 gefixt (2026-04-22) |
| Recovery-Import | ⏳ Ausstehend | Noch nicht getestet |
| Recovery-Update (Restart) | ✅ OK | B16+B34 gefixt |
| Recovery-Upload Fortschritt | ⏳ Ausstehend | Flash-Writes in async_tcp → TCP-Backpressure; strukturelles Problem offen |
| REST-API (`/data`, `/settings`) | ✅ OK | |
| Display-Funktionen | ✅ OK | |
| Standby-Erkennung | ✅ OK | |
| WiFi-Stabilität | ⏳ Langzeittest lief | Gerät lief über Nacht |

**Compile-Ergebnis miniV3 (2026-04-25, nach B33–B35):** RAM 27.4% (89.788 B / 327.680 B), Flash 30.2% (1.902.485 B / 6.291.456 B)
