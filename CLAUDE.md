# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository

- **Upstream/Remote:** `git@github.com:WLANThermo-nano/WLANThermo_ESP32_Software.git` (SSH)
- **GitHub-User:** `Phantomias2006`
- **Branches:**
  - `master` — drei eigene Commits auf Upstream-History:
    - `225e87f` — Phase 1: Kernel Upgrade
    - `a2fad1c` — Phase 4c: ArduinoJson v7 Migration
    - `dc130ee` — docs: CLAUDE.md Repo-Info
  - `develop` — Integration für Hotfixes und schnelle Releases (immer releasable)
  - `next` — Spielwiese für Phasen-Arbeit; alle `feature/*`-Branches landen hier zuerst, dann nach Test in `develop` → `master`
- **Feature-Branch-Schema:** `feature/phase-0-stabilization`, `feature/phase-2-vue3`, `feature/phase-4a-lib-upgrade`, `feature/phase-4b-kernel-3x`, `feature/phase-5-lvgl9`
- **Referenz-Backup:** `WLANThermo_ESP32_Software-master_old/` (ZIP-Extrakt ohne Git, kann gelöscht werden)

## Project Overview

WLANThermo ESP32 is Arduino-framework firmware for a WiFi BBQ thermometer. It supports multiple hardware variants with different displays, sensors, and control outputs. The firmware embeds a gzip-compressed Vue.js web UI and exposes a REST/WebSocket API.

## Build Commands

**Firmware (PlatformIO required):**
```bash
~/.platformio/penv/bin/pio run -e miniV3             # Build for miniV3 hardware
~/.platformio/penv/bin/pio run -e miniV3 -t upload   # Build and flash via USB
~/.platformio/penv/bin/pio run -e miniV3 -t monitor  # Serial monitor (115200 baud)
```

> **macOS:** `pio` ist nicht im PATH — vollständigen Pfad `~/.platformio/penv/bin/pio` verwenden.

> **Build-Fix (2026-04-21):** Falls `ModuleNotFoundError: No module named 'intelhex'` auftritt:
> ```bash
> ~/.platformio/penv/bin/pip install intelhex
> ```
> Das Paket fehlt nach Neuinstallation von PlatformIO im isolierten Python-Environment.

> **Full-Flash-Fix (2026-04-22):** Das Hardware-Target miniV3 hat 16MB Flash. Ohne explizite Flash-Konfiguration in `platformio.ini` nutzt PlatformIO den Board-Default (4MB) → inkompatiblen Bootloader → Boot-Loop ohne serielle Ausgabe. Fix ist in `[env]` eingetragen:
> ```ini
> board_build.flash_mode = dio
> board_build.flash_size = 16MB
> board_upload.flash_size = 16MB
> board_build.flash_freq = 40m
> ```
> Beim ersten Flash nach dieser Änderung empfohlen: `pio run -e miniV3 -t erase` vor `upload`, um alte NVS/OTA-Daten zu löschen.

Hardware variants: `miniV1`, `miniV2`, `miniV3`, `connectV1`, `nanoV3`, `linkV1`, `boneV1`

**Web UI (Node.js v12 + yarn):**
```bash
cd webui
yarn install
yarn serve          # Dev server with hot reload
yarn build          # All variants (nano, mini, link, bone)
yarn build-mini     # Single variant
```

After changing the web UI, rebuild firmware to embed the new assets — `extra_script.py` gzips HTML/JS/CSS into C header files at build time.

## Architecture

**Firmware entry:** `src/main.cpp` creates two FreeRTOS tasks pinned to separate cores:
- `ConnectTask` — WiFi, MQTT, OTA, cloud, push notifications
- `MainTask` — serial input, temperature polling, pitmaster control

**Global singleton:** `extern SystemBase *gSystem` is the central access point for all subsystems (sensors, display, WiFi, settings, etc.). Hardware variants subclass `SystemBase` (e.g., `SystemMiniV3`) and override `hwInit()`.

**Key subsystems:**
| Path | Role |
|------|------|
| `src/system/` | Hardware abstraction; one `System[Model]V[N].cpp` per hardware variant |
| `src/temperature/` | Sensor drivers (MAX31855, MAX11615, MCP3208, BLE, remote) coordinated by `TemperatureGrp` |
| `src/pitmaster/` | PID-based automated temperature control; `PitmasterGrp` manages multiple controllers |
| `src/display/` | Display abstraction (`DisplayBase`) with subclasses for Nextion HMI, OLED, and TFT/LVGL |
| `src/connect/` | Remote sensor connectivity |
| `src/bluetooth/` | BLE stack for wireless probe sensors |
| `src/WebHandler.cpp` | REST endpoint routing via ESPAsyncWebServer |
| `src/API.h` | JSON API schema (ArduinoJson 7) |
| `src/Wlan.cpp` | WiFi + MQTT (AsyncMQTT) |

**Web UI:** Vue 2 SPA in `webui/src/`. Build output lands in `webui/dist/[variant]/`. The firmware's `extra_script.py` converts these into embedded C arrays.

**Display variants:** Nextion HMI (serial), SSD1306 OLED (I2C), ILI9341 TFT with LVGL touchscreen. TFT UI lives in `src/display/tft/`.

## Key Conventions

- ArduinoJson **7.x** — `JsonDocument` statt `DynamicJsonBuffer`, Value-Typen (`JsonObject`, `JsonArray`) statt References, `deserializeJson()` statt `parseObject()`. v5 ist EOL.
- Platform: **`espressif32@^6.0.0`** (official, arduino-esp32 2.x / ESP-IDF 4.4.x). The old `tuniii/platform-espressif32` WifiFix fork is no longer used.
- WiFi fix (`WIFI_ALL_CHANNEL_SCAN` — connect to strongest AP) is set at runtime in `src/Wlan.cpp` after each `WiFi.begin()` call via `esp_wifi_set_config()`.
- `extra_script.py` patches `ESPRandom@1.4.1` (missing `#include <vector>`) at build time via `patch_esp_random()`. Idempotent.
- WiFi event enums: `ARDUINO_EVENT_WIFI_STA_GOT_IP`, `ARDUINO_EVENT_WIFI_STA_DISCONNECTED`, `ARDUINO_EVENT_WIFI_AP_STADISCONNECTED` (see `src/Wlan.cpp`).
- Async networking uses `ESP32Async/AsyncTCP@^3.4.10` + `ESP32Async/ESPAsyncWebServer@^3.10.3` (Nachfolger-Repos; `mathieucarbou/*` seit Januar 2025 archiviert). AsyncTCP läuft auf Core 1 (`-DCONFIG_ASYNC_TCP_RUNNING_CORE=1` in `[env]` build_flags) — gleicher Core wie MainTask/ConnectTask, verhindert Inter-Core-Synchronisationsoverhead.
- Logging uses `ArduinoLog` macros (`Log.verbose`, `Log.notice`, `Log.error`) — not `Serial.print`.
- Settings persistence: `src/Settings.cpp` uses **NVS** (`Preferences` API) — NOT SPIFFS. SPIFFS wird nur für Cloud-URL-Cache (`Cloud.cpp`) und Nextion-Display-Updates verwendet.
- Power management: `esp_pm_config_esp32_t` + `esp_pm_configure()` in `src/system/SystemBase.cpp`. `CONFIG_PM_ENABLE` ist im vorkompilierten arduino-esp32 2.x SDK **nicht gesetzt** → Power-Save-Modus nicht verfügbar. `setPowerSaveMode()` deaktiviert sich nach erstem `ESP_ERR_NOT_SUPPORTED` selbst. Erst mit **Phase 4b** (arduino-esp32 3.x / ESP-IDF 5.x) nutzbar — `CONFIG_PM_ENABLE=y` soll dort Standard sein (vor Phase-4b-Start in `sdk/esp32/sdkconfig` der neuen SDK verifizieren).
- **ESPAsyncWebServer Migrationshinweise:**
  - **3.x (mathieucarbou → ESP32Async):** `AsyncWebHandler::canHandle()` und `isRequestHandlerTrivial()` sind jetzt `const virtual` → alle Subklassen müssen `const override` verwenden. Body-Daten in `handleBody()` sind **nicht null-terminiert** → null-terminierte Kopie (`new uint8_t[len+1]`) vor `deserializeJson()` erstellen. `AsyncJsonResponse::getRoot()` gibt `JsonVariant` zurück → `.to<JsonObject>()` aufrufen.
  - **3.10.x (ESP32Async):** `WebRequestMethodComposite` ist jetzt eine eigene Klasse (kein `int32_t` mehr). Struct-Felder für HTTP-Methoden müssen `WebRequestMethodComposite` statt `int32_t` sein. Literal `0` als Initialisierer → `HTTP_UNKNOWN`. Vergleich `(request->method() & field) > 0u` → `field & request->method()` (operator& gibt `bool` zurück; Operandenreihenfolge: Composite links, `WebRequestMethod` rechts).

## Project Roadmap

**Ziele:** UI-Redesign, App-Store-Publishing (Android + iOS), Arduino Kernel Upgrade auf 3.x (Power Save wiederherstellen), Heap-Stabilität  
**Reihenfolge:** Phase 0 (Stabilisierung) → Phase 1+2 (parallel: Kernel 2.x / UI) → Phase 3 (App Store) → Phase 4a (Lib Pre-Upgrade) → Phase 4b (Kernel 3.x + Power Save) → Phase 4c (ArduinoJson 7) → Phase 5 (LVGL 9)

**Mobile App:** Flutter-App in Branch `android-build-test` (PR #180, Repo: `WLANThermo-nano/WLANThermo_ESP32_Software`). Flutter WebView-Wrapper lädt `assets/html/index.html` (Vue.js Build) mit JS-Bridge für mDNS, Firebase FCM, Permissions. Cordova (`/mobile/`) wird dadurch abgelöst.

### Phase 0 — Stabilisierung (ausstehend)

Bug-Fixes und SRAM-Optimierungen — siehe Abschnitte unten.

### Phase 1 — Kernel Upgrade ✅ ABGESCHLOSSEN (2026-04-20)

Alle 7 Hardware-Varianten auf `espressif32@^6.0.0` (arduino-esp32 2.x / ESP-IDF 4.4.x) umgestellt. `platform` zentral in `[env]` gesetzt, alle tuniii-Fork-Overrides aus den Einzel-Envs entfernt. Temporäre Test-Env `miniV3_v3kernel` gelöscht.  
Compile-Test bestanden: RAM 18–29%, Flash 24–31% je Variante.

**Hardware-Test miniV3 (2026-04-20) — Ergebnis:**

| Bereich | Status | Anmerkung |
|---------|--------|-----------|
| Boot / Stabilität | ✅ OK | Bootloop behoben (siehe Fixes unten) |
| Web-Interface laden | ✅ OK | Lädt nach canHandle()-Fix |
| REST-API `/data`, `/settings` | ✅ OK | Nach canHandle()-Fix |
| Settings speichern | ✅ OK | WDT-Fix bestätigt (2026-04-20). Root Cause war Heap-Korruption durch 13× unsized `DynamicJsonBuffer` (nicht NVS-Blocking). Fix: alle auf `DynamicJsonBuffer(Settings::jsonBufferSize)` + NVS-Writes von setChannels/setPush/setPitmaster/setPID via Pending-Flags auf ConnectTask delegiert. |
| WiFi-Stabilität | ⏳ Ausstehend | Langzeittest noch nicht durchgeführt |
| Power Management | ❌ Nicht verfügbar | `CONFIG_PM_ENABLE` im vorkompilierten arduino-esp32 2.x SDK nicht gesetzt. `sdkconfig.board` hatte keine Wirkung (gilt nur für ESP-IDF-native Builds). Fix: `setPowerSaveMode()` deaktiviert sich nach erstem `ESP_ERR_NOT_SUPPORTED` selbst. Erst mit Phase 4b (arduino-esp32 3.x) nutzbar — ob dort `CONFIG_PM_ENABLE=y` Standard ist, vor Phase-4b-Start verifizieren. |
| BLE / NRF-Chip | ⏳ Fix implementiert, Test ausstehend | `remoteIndex`-Discovery-Fix in `Bluetooth.cpp`; kein Re-Pairing nötig; `{"d":[]}` = Probe noch nicht in Reichweite, löst sich von selbst |

**Fixes die während des Hardware-Tests notwendig waren:**

| Fix | Datei | Ursache |
|-----|-------|---------|
| `LogRingBuffer::write()` fehlte `return 1u` | `src/LogRingBuffer.cpp` | GCC 8.x (arduino-esp32 2.x) emittiert `ILL`-Trap für fehlenden Return-Wert bei `size_t`-Funktion → `IllegalInstruction`-Crash beim ersten `Log.notice()` |
| Task-Stacks zu klein | `src/main.cpp` | FreeRTOS-Overhead in ESP-IDF 4.4.x größer als 1.x: MainTask 3000→8000 B, ConnectTask 4000→12000 B |
| Bluetooth-Task-Stack zu klein | `src/bluetooth/Bluetooth.cpp` | Gleiche Ursache: 3000→6000 B |
| `NanoWebHandler::canHandle()` fehlte `const` | `src/WebHandler.h/.cpp` | `mathieucarbou/ESPAsyncWebServer@^3.0.0` deklariert die virtuelle Methode als `const` → ohne `const` kein Override → alle REST-Endpunkte gaben leere Seite zurück |
| `NanoWebHandler::isRequestHandlerTrivial()` fehlte `const`, `boolean`→`bool` | `src/WebHandler.h` | Gleiche Ursache |
| `Wlan::setHostName()` rief `MDNS.begin()` im `async_tcp`-Kontext auf | `src/Wlan.cpp/.h` | `MDNS.begin()` blockiert in arduino-esp32 2.x mehrere Sekunden → Task-Watchdog auf `async_tcp`. Fix: `mdnsUpdatePending`-Flag, Abarbeitung in `Wlan::update()` (ConnectTask) |
| `Bluetooth::getDevices()` sendete falsche Bitmask nach NVS-Restore | `src/bluetooth/Bluetooth.cpp` | `remoteIndex` wird nie in NVS gespeichert → nach jedem Boot `remoteIndex=0xFF` → `1u<<255` → Xtensa maskiert Shift auf 5 Bit → `requestedDevices=0x80000000` → NRF ruft `enable(0x80000000)` auf → Bit 0 = 0 → Probe deaktiviert. Fix: `remoteIndex==0xFF`-Devices aus Bitmask ausschließen; wenn `needsDiscovery`, `getDevices=0` senden. Sobald NRF-Scanner die Probe entdeckt, gibt `getDevicesJson()` sie zurück (unabhängig von `enable`) → Adress-Matching weist korrekten `remoteIndex` zu → nächster Zyklus aktiviert sie korrekt. `{"d":[]}` = Probe noch nicht in Reichweite, kein Re-Pairing nötig |
| `handleBody()` übergab nicht null-terminierten Buffer | `src/WebHandler.cpp` | `ESPAsyncWebServer@^3.0.0` liefert Body-Daten ohne `\0` → `parseObject()` las über Puffer hinaus → Crash in `setSystem`. Fix: null-terminierte Kopie (`new uint8_t[len+1]`) in `handleBody()` vor Handler-Aufruf. Gilt für alle Body-Handler. |
| `setBluetooth()`: fehlendes `return` + unsized `DynamicJsonBuffer` | `src/WebHandler.cpp` | Kein `return`-Statement → UB (GCC 8.x). `DynamicJsonBuffer jsonBuffer;` ohne Größe → 256-Byte-Slab wächst mit mehrfachen `malloc`-Slabs → Heap-Korruption. Symptom: `CORRUPT HEAP: Bad head ... Expected 0xabba1234`. Fix: `return 1;` + `DynamicJsonBuffer(Settings::jsonBufferSize)`. |
| `TemperatureGrp::saveConfig()` unsized `DynamicJsonBuffer` | `src/temperature/TemperatureGrp.cpp` | Gleiche Ursache wie vorheriger Fix. Fix: `DynamicJsonBuffer(Settings::jsonBufferSize)`. |
| `setSystem()`: 4 NVS-Writes im `async_tcp`-Kontext | `src/WebHandler.cpp` | Alle saveConfig()-Calls direkt im Handler → blockiert `async_tcp` → Task-Watchdog. Fix: 4 `xxxSavePending`-Flags + `SystemBase::processPendingSave()` in `Wlan::update()` (ConnectTask). One write per second, staggered. |
| Widespread unsized `DynamicJsonBuffer` → Heap-Korruption | `src/WebHandler.cpp` (9×), `API.cpp`, `Wlan.cpp`, `PitmasterGrp.cpp`, `Connect.cpp`, `Bluetooth.cpp` | `DynamicJsonBuffer jsonBuffer;` ohne Größe → mehrere 256-Byte-Slabs via `malloc()` → korrumpiert Heap-Metadaten über Zeit (40–80s Laufzeit) → `async_tcp` Task WDT mit `Backtrace: |<-CORRUPTED`. Fix: alle auf `DynamicJsonBuffer(Settings::jsonBufferSize)` gesetzt. Zusätzlich: NVS-Writes von `setChannels`, `setPush`, `setPitmaster`, `setPID` auf `tempConfigSavePending`/`notificationConfigSavePending`/`pitmasterConfigSavePending` Flags umgestellt. `OtaUpdate::setPrerelease()` fehlendes `return checkForUpdate` ergänzt. |
| `setPowerSaveMode()` retryt endlos bei NOT_SUPPORTED | `src/system/SystemBase.cpp` | `CONFIG_PM_ENABLE` nicht im vorkompilierten SDK → `esp_pm_configure()` gibt immer `ESP_ERR_NOT_SUPPORTED`. `powerSaveModeEnabled` wird nie gesetzt → jeder Update-Zyklus ruft es erneut auf. Fix: bei `ESP_ERR_NOT_SUPPORTED` wird `powerSaveModeSupport = false` gesetzt. |
| PCA9533-Startup-Pause (~170ms) | `src/display/tft/DisplayTft.cpp` | arduino-esp32 2.x ersetzt den alten Bit-Bang-I2C-Treiber durch den ESP-IDF Hardware-Treiber. Jede fehlgeschlagene I2C-Transaktion (NACK) hat ~10ms FreeRTOS/Treiber-Overhead statt < 1ms in 1.x. Wenn PCA9533 (Adresse `0x62`) nicht verbaut ist, liefen trotzdem 17 Transaktionen (5× `init()` + 1× `ping()` + 4× `setMODE()` mit `getReg`+`setReg` + 3× sonstige) → ~170ms Pause im Boot-Log bei „Setup LED Controller: 2". Fix: `ping()` als Guard voranstellen; `init()` und alle weiteren Calls nur wenn `ping() == 0`. |

**Stack-Messwerte miniV3 (Frühphase, unter Grundlast):**
- MainTask: ~1.636 B genutzt (von 8.000 B) → nach Langzeittest + OTA-Durchlauf auf ~2.200 B reduzierbar
- ConnectTask: ~1.788 B genutzt (von 12.000 B) → nach Langzeittest auf ~2.300 B reduzierbar
- Bluetooth-Task: Messung ausstehend

### Phase 2 — UI-Redesign (ausstehend)

Vue 2 → Vue 3 (Neubau, Vue 2 seit 31.12.2023 EOL).

### Phase 3 — App Store (ausstehend)

Flutter-App fertigstellen, Cordova ablösen.

### Phase 4a — Library Pre-Upgrade (ausstehend)

**Voraussetzung für Phase 4b.** Alle Libs auf aktuelle Versionen heben, bevor der Kernel gewechselt wird — maximale Kompatibilität, und der Phase-4b-Blocker (`AsyncMqttClient`) wird hier beseitigt.

| Lib | Jetzt | Ziel | Aufwand | Prio |
|-----|-------|------|---------|------|
| **`AsyncMqttClient@0.8.2`** | `me-no-dev` intern | `mathieucarbou/AsyncMqttClient` | Mittel (API-Diff prüfen) | **Blocker für 4b** |
| `TFT_eSPI@^2.5.31,<2.5.34` | Constraint | `^2.5.34` | **Trivial — Einzeiler** | Hoch |
| `protohaus/ESPRandom@1.4.1` | 1.4.1 | neueste | Trivial + Patch in `extra_script.py` entfernen | Mittel |
| `asyncHTTPrequest@^1.2.1` | 1.2.x | neueste 1.x | Trivial | Niedrig |
| `thijse/ArduinoLog@~1.0.3` | 1.0.4 | ~1.1.x | Trivial | Niedrig |
| `ThingPulse/esp8266-oled-ssd1306@4.0.0` | 4.0.0 | neueste | Compile+Test | Niedrig |
| `mathertel/OneButton@1.3.0` | 1.3.0 | 2.x | API-Check (v2 hat neue Multi-Click-API, alte Callbacks bleiben kompatibel) | Niedrig |
| Custom Forks (`tuniii/*`, `borisneubert/Time`) | — | — | Manuellen upstream-Diff prüfen, kein regulärer Upgrade-Pfad | Vor 4b |

**Nach Phase 4a:** Compile-Test aller 7 Varianten mit neuen Libs (noch auf Kernel 2.x).

---

### Phase 4b — Kernel 3.x / ESP-IDF 5.x (ausstehend)

**Hauptziel: Power Save wiederherstellen (Stromverbrauch senken)**  
`CONFIG_PM_ENABLE=y` ist in arduino-esp32 3.x Standard — ungeprüft, vor Start in `sdk/esp32/sdkconfig` der neuen SDK verifizieren.

**Voraussetzung:** Phase 4a abgeschlossen (AsyncMqttClient-Wechsel ist Blocker — ohne ihn kompiliert nichts).

**Eigener Code — 2 Änderungen, beide trivial:**

| Datei | Zeile | Änderung |
|-------|-------|----------|
| `src/system/SystemBase.cpp` | 326 | `esp_pm_config_esp32_t` → `esp_pm_config_t` |
| `platformio.ini` | `[env]` platform | `espressif32@^6.0.0` → `espressif32@^7.0.0` |

`src/Wlan.cpp` (`ARDUINO_EVENT_*`, `esp_wifi_set_config`) bleibt unverändert — diese API ist in ESP-IDF 5.x stabil.

**Library-Kompatibilität nach Phase 4a:**

| Lib | Status | Bemerkung |
|-----|--------|-----------|
| `ESP32Async/AsyncTCP@^3.4.10` | ✅ OK | explizit arduino-esp32 3.x unterstützt |
| `ESP32Async/ESPAsyncWebServer@^3.10.3` | ✅ OK | explizit arduino-esp32 3.x unterstützt |
| `ESP32Async/AsyncMqttClient` | ✅ OK (nach 4a) | explizit arduino-esp32 3.x unterstützt |
| `ArduinoJson@^7.x` | ✅ OK | pure C++, kein IDF-Bezug; nach Phase 4c bereits auf 7.x |
| `asyncHTTPrequest`, `ArduinoLog`, `ESPRandom`, `Time`, `MedianFilter` | ✅ OK | pure C++/Arduino |
| `TFT_eSPI@^2.5.34` (nach 4a) | ✅ OK | 2.5.34+ explizit für neuere Board-Packages |
| `esp8266-oled-ssd1306` | ✅ OK | kein IDF-spezifischer Code |
| `lvgl@~7.11.0` | ⚠️ Kompiliert evtl. | Nicht für ESP-IDF 5.x entwickelt; wahrscheinlich keine Compile-Fehler, aber ungetestet |
| `tuniii/ESPNexUpload` | ⚠️ Risiko | Nutzt ESP32-Flash/OTA-APIs die sich in 5.x geändert haben |
| `tuniii/BBQduino` | ⚠️ Unbekannt | Custom Fork ohne Upstream — manuell prüfen |
| `tuniii/ITEADLIB_Arduino_Nextion` | ⚠️ Risiko | Alte Library; serielle Kommunikation wahrscheinlich OK, aber ungeprüft |

**Weitere Vorteile ESP-IDF 5.x:**
- FreeRTOS SMP-Scheduler verbessert Task-Isolation (`xTaskCreatePinnedToCore` in `main.cpp`)
- `spinlock_t`-API sauberer → B6-Fix (`globalIndexTracker++`) eleganter lösbar
- Besseres `heap_caps`-Fragmentierungsverhalten
- Stabilerer WiFi-Reconnect-Stack

---

### Phase 4c — ArduinoJson 7 Migration ✅ ABGESCHLOSSEN (2026-04-21)

Dedizierter Sprint. Unabhängig von 4b — vor 4b durchgeführt.

**Scope:** 20 Dateien, ~404 API-Aufrufe (alle v5-Patterns) — vollständig migriert.

| Datei | ArduinoJson-Aufrufe | Hauptpatterns |
|-------|---------------------|---------------|
| `WebHandler.cpp` | ~98 | `.asString()` ×37, `DynamicJsonBuffer` ×11, `.success()` ×11 |
| `API.cpp` | ~51 | `JsonObject&` ×37, `createNestedObject()` ×24 |
| `Notification.cpp` | ~26 | `.asString()` ×12, `createNestedObject()` ×4 |
| `Bluetooth.cpp` | ~16 | `createNestedArray()` ×4 |
| `Cloud.cpp` | ~15 | `.asString()` ×4 |
| weitere 15 Dateien | ~198 | verstreut |

**Migrationstabelle v5 → v7:**

| v5 API | v7 Ersatz | Hinweis |
|--------|-----------|---------|
| `DynamicJsonBuffer jsonBuffer(N)` | entfällt — kein Buffer-Objekt mehr | — |
| `jsonBuffer.parseObject(input)` | `JsonDocument doc; deserializeJson(doc, input)` | Error-Objekt statt bool |
| `JsonObject &obj = jsonBuffer.parseObject(...)` | `JsonObject obj = doc.as<JsonObject>()` | Value statt Reference |
| `JsonObject &obj = jsonBuffer.createObject()` | `JsonDocument doc; JsonObject obj = doc.to<JsonObject>()` | — |
| `obj.createNestedObject("key")` | `obj["key"].to<JsonObject>()` | — |
| `obj.createNestedArray("key")` | `obj["key"].to<JsonArray>()` | — |
| `.asString()` | `.as<const char*>()` oder `.as<String>()` | `const char*` kann NULL sein → prüfen |
| `.as<boolean>()` | `.as<bool>()` | — |
| `json.success()` | `!doc.isNull()` oder `DeserializationError` auswerten | — |
| `JsonObject&` als Funktionsparameter | `JsonObject` (Value-Typ) | kein `&` mehr |
| `JsonArray&` als Funktionsparameter | `JsonArray` (Value-Typ) | kein `&` mehr |

**Vorteile der Migration:**
- **Kein Heap-Wachstum mehr:** `DynamicJsonBuffer` v5 wächst mit mehreren malloc()-Slabs → war Root-Cause von B11/B12/M1. v7 `JsonDocument` nutzt einen einzigen, vorab konfigurierten Puffer.
- **Stack-Allokation möglich:** `StaticJsonDocument<N>` legt Puffer auf dem Stack → null Heap-Operationen für kleine Dokumente.
- **~20% schnelleres Parsen** gegenüber v5.
- **Strukturiertes Error-Handling:** `DeserializationError err = deserializeJson(...)` statt `.success()` bool.
- **Sicherheit:** Kein implizites Buffer-Überlaufen mehr; Dokumentgröße ist explizit begrenzt.
- **Aktiver Support:** v5 ist seit 2023 offiziell EOL — keine Bugfixes oder Sicherheits-Updates mehr.
- **Langfristig:** Voraussetzung für saubereres Code-Review und Phase-5-LVGL-Refactor.

**Aufwand:** ~2–3 Tage dedizierter Sprint. Die API-Änderungen sind mechanisch (find-replace-ähnlich), aber jede Datei muss manuell verifiziert werden da v5-References (`JsonObject&`) zu Values werden.

**Compile-Ergebnis miniV3 (2026-04-21):**
- RAM: 28.5% (93.460 B / 327.680 B)
- Flash: 32.0% (2.011.261 B / 6.291.456 B)

**Zusätzliche Fixes beim Compile gefunden:**

| Fix | Datei | Ursache |
|-----|-------|---------|
| `constrain(_pid["jp"], 10, 100)` → `constrain(_pid["jp"].as<int>(), 10, 100)` | `src/WebHandler.cpp` | ArduinoJson v7 `MemberProxy` ist non-copyable — Arduino `constrain()`-Makro versucht Kopie → privater Copy-Konstruktor löst Compile-Fehler aus. Fix: `.as<int>()` vor dem `constrain()`-Aufruf. |
| `double_with_n_digits(val, N)` entfernt (8×) | `src/pitmaster/PitmasterGrp.cpp` | `double_with_n_digits()` war ArduinoJson v5 internes Helper-Funktion zum Serialisieren von Floats mit begrenzter Dezimalstellenzahl. In v7 nicht mehr vorhanden. Ersatz: direktes Float-Assignment (`_pid["Kp"] = profile->kp`). v7 serialisiert Floats mit voller Präzision. |

**Hardware-Test miniV3 (2026-04-22) — Ergebnis Phase 4c:**

| Bereich | Status | Anmerkung |
|---------|--------|-----------|
| Pitmaster (Servo, Lüfter) | ✅ OK | |
| Pitmaster SSR | ✅ OK (fix 2026-04-22) | DAC/LEDC-Konflikt auf GPIO 25 — `dacWrite()` aktivierte DAC (Priorität über GPIO-Matrix); `ledcAttachPin()` deaktiviert DAC in ESP-IDF 4.x nicht automatisch → LEDC-Signal blockiert (B17 gefixt) |
| System (Sprache, Einheit) | ✅ OK | |
| IoT (Cloud) | ✅ OK | |
| MQTT | ⏳ Ausstehend | Keine Gegenstelle verfügbar |
| Benachrichtigungen (Telegram, Pushover, Alarmton) | ✅ OK | |
| Bluetooth | ✅ OK | |
| Temperaturmessung + Typ K | ✅ OK | |
| Kanaleinstellungen + Temperaturschwellen | ✅ OK | |
| Batterieerkennung | ✅ OK | Ladeendzustand noch nicht getestet |
| Recovery-Mode starten | ✅ OK | Wird bei `/recovery` gestartet |
| Recovery-Seite (Auto-Redirect) | ✅ OK (fix 2026-04-22) | `restart.html` JS fehlte `onerror`-Handler → Seite lud erst beim 2. Aufruf (B15 gefixt) |
| Recovery-Export | ✅ OK (fix 2026-04-22) | `beginResponse_P` mit lokalem String-Pointer → HTTP-Header als Dateiinhalt (B14 gefixt) |
| Recovery-Import | ⏳ Ausstehend | Noch nicht getestet |
| Recovery-Update (Restart) | ✅ OK (fix 2026-04-22) | Kein automatischer Restart nach Firmware-Upload (B16 gefixt) |
| REST-API (`/data`, `/settings`) | ✅ OK | |
| Display-Funktionen | ✅ OK | |
| Standby-Erkennung | ✅ OK | |
| WiFi-Stabilität | ⏳ Langzeittest läuft | Gerät läuft über Nacht; Ergebnis ausstehend |

**Gesamtergebnis:** Solide — alle Kernfunktionen OK. Drei Recovery-Bugs gefixt (B14–B16), SSR DAC/LEDC-Konflikt gefixt (B17).

---

### Phase 5 — LVGL 9 + TFT-UI Neubau (Long-term, ausstehend)

Voraussetzung: Phase 4b abgeschlossen (Kernel 3.x), Phase 4c empfohlen (saubere JSON-API im TFT-UI-Code).

- LVGL 7 → 9: Widgets umbenannt, Event-System geändert — gesamtes `src/display/tft/` muss neu geschrieben werden.
- Sinnvoll zu koppeln mit Phase 2 (Vue 3 UI-Redesign), da beide das UI betreffen.

## Known Issues (pending fixes)

### Bugs

| ID | Datei | Zeile | Severity | Problem |
|----|-------|-------|----------|---------|
| B1 | `src/WebHandler.cpp` | 1231 | Critical | NULL-Pointer auf `pitmasters[id]` — vom Entwickler mit `//TODO NULL pointer!!!` markiert |
| B2 | `src/WebHandler.cpp` | 625–628 | High | `num--` kann -1 ergeben wenn JSON `"number": 0` enthält → out-of-bounds |
| B3 | `src/WebHandler.cpp` | 742, 746, 748 | High | `strcpy` in 30-Byte-Puffer (`MQTT_STRING_SIZE`) ohne Längencheck |
| B4 | `src/WebHandler.cpp` | 810–811, 827–828 | High | `strncpy` mit `sizeof` statt `sizeof-1` → kein Null-Terminator bei voller Länge |
| B5 | `src/WebHandler.cpp` | 885, 886, 898, 907 | High | `strcpy` in Notification-Device-Felder ohne Längencheck |
| B6 | `src/pitmaster/Pitmaster.cpp` | Konstruktor | Medium | Race Condition: `globalIndexTracker++` nicht atomar (FreeRTOS) |
| B7 | `src/pitmaster/Pitmaster.cpp` | pidCalc() | High | `this->temperature` kann NULL sein → Null-Pointer in PID-Berechnung |
| B8 | `src/connect/Connect.cpp` | 175 | High | `connectDevices[0]` ohne Prüfung ob Vector leer ist |
| ~~B9~~ | `src/WebHandler.cpp` | `handleBody()` | ~~High~~ **GEFIXT (2026-04-20)** | Body-Daten von `mathieucarbou/ESPAsyncWebServer@^3.0.0` sind nicht null-terminiert → Crash in `setSystem` (`parseObject` las über Puffer hinaus). Fix: `handleBody()` erstellt jetzt null-terminierte Kopie (`new uint8_t[len+1]`) vor dem Handler-Aufruf. Gilt für alle Body-Handler. |
| ~~B11~~ | `src/WebHandler.cpp` | `setBluetooth()` | ~~High~~ **GEFIXT (2026-04-20)** | (1) Fehlendes `return`-Statement → Undefined Behavior (UB) in GCC 8.x. (2) `DynamicJsonBuffer jsonBuffer;` ohne Größe → ArduinoJson 5 startet mit 256-Byte-Slab → Heap-Korruption unter Speicherdruck. Fix: `return 1;` ergänzt, `DynamicJsonBuffer(Settings::jsonBufferSize)` gesetzt. Symptom: `CORRUPT HEAP: Bad head ... Expected 0xabba1234`. |
| ~~B12~~ | `src/temperature/TemperatureGrp.cpp` | `saveConfig()` | ~~Medium~~ **GEFIXT (2026-04-20)** | `DynamicJsonBuffer jsonBuffer;` ohne Größe → selbes M1-Problem wie B11. Fix: `DynamicJsonBuffer(Settings::jsonBufferSize)`. |
| ~~B13~~ | `src/WebHandler.cpp` | `setSystem()` | ~~High~~ **GEFIXT (2026-04-20)** | 4 NVS-Writes hintereinander → Task-Watchdog (WDT) auf `async_tcp`. Root Cause: `MainTask`, `ConnectTask` und `async_tcp` laufen alle auf **Core 1** (siehe `main.cpp:160,169`). NVS-Writes sperren Core 1 für die Flash-Write-Dauer → async_tcp kann keinen WDT-Reset ausführen. Erster Fix (Auslagern zu ConnectTask) half nicht, da ConnectTask ebenfalls Core 1. Zweiter Fix: **Stückelung** — `processPendingSave()` schreibt genau **ein** NVS-Key pro Aufruf. ConnectTask ruft es jede Sekunde auf (`TASK_CYCLE_TIME_CONNECT_TASK=1000ms`). 4 Writes = 4 Sekunden verteilt, async_tcp kann dazwischen WDT zurücksetzen. Flags: `systemConfigSavePending`, `otaConfigSavePending`, `wlanConfigSavePending`, `tempConfigSavePending`. |
| ~~B10~~ | `src/system/SystemBase.cpp` | `setPowerSaveMode()` | ~~Medium~~ **GEFIXT (2026-04-20)** | `CONFIG_PM_ENABLE` ist im vorkompilierten Arduino-ESP32-2.x-SDK nicht gesetzt (`# CONFIG_PM_ENABLE is not set` in `sdk/esp32/sdkconfig`). `sdkconfig.board` hat **keine Wirkung** beim Arduino-Framework (gilt nur für ESP-IDF-native Builds). Fix: bei `ESP_ERR_NOT_SUPPORTED` wird `powerSaveModeSupport = false` gesetzt → kein weiterer Aufruf. Power-Save ist auf arduino-esp32 2.x generell nicht nutzbar; erst mit Phase 4 (ESP-IDF 5.x / arduino-esp32 3.x) möglich, wo `CONFIG_PM_ENABLE=y` Standard ist. |
| ~~B14~~ | `src/RecoveryMode.cpp` | `/export`-Handler | ~~High~~ **GEFIXT (2026-04-22)** | `beginResponse_P(200, "text/text", (uint8_t*)exportSettings.c_str(), ...)` speichert nur Raw-Pointer auf lokale `String exportSettings`. Nach Lambda-Return wird String zerstört → dangling pointer. Async-Webserver liest beim Senden freien Speicher → Browser empfängt HTTP-Header als Dateiinhalt statt NVS-Keys. Fix: `beginResponse(200, "text/plain", exportSettings)` — kopiert String-Inhalt intern. |
| ~~B15~~ | `webui/old/restart.html` | XHR-Polling-Loop | ~~Medium~~ **GEFIXT (2026-04-22)** | `xhr.onerror` nicht behandelt. Wenn ESP nach `/recovery`-Aufruf WLAN trennt (Recovery-Reboot), bekommt Browser Network Error (kein Timeout) → `onerror` feuert ohne Handler → Polling stoppt → Seite zeigt Spinner ewig. Benutzer musste `/recovery` manuell ein zweites Mal aufrufen. Fix: `xhr.onerror`-Handler ergänzt, der nach 1s Pause erneut `/ping` sendet. |
| ~~B16~~ | `src/RecoveryMode.cpp` | `/uploadfile`-POST-Handler | ~~High~~ **GEFIXT (2026-04-22)** | Nach `Update.end(true)` kein `ESP.restart()` → Firmware-Update im Recovery-Mode ohne automatischen Neustart. Fix: Im POST-Response-Handler nach `request->send()` wird geprüft ob `uploadFileType == Firmware || SPIFFS`; wenn ja: `WiFi.disconnect()` + 1s delay + `ESP.restart()`. |
| ~~B17~~ | `src/pitmaster/Pitmaster.cpp` | `initActuators()` SSR-Fall + `disableActuators()` | ~~High~~ **GEFIXT (2026-04-22)** | SSR keine Reaktion nach Phase 1 Migration. Root Cause: `dacWrite(ioPin1, 0)` aktiviert das DAC-Peripheral auf GPIO 25/26 (RTC-Pad). In ESP-IDF 4.4.x hat das DAC Vorrang über das GPIO-Matrix-Signal — LEDC läuft über die GPIO-Matrix und wird vom aktiven DAC blockiert. `ledcAttachPin()` deaktiviert das DAC in ESP-IDF 4.x nicht automatisch (in ESP-IDF 1.x funktionierte das noch). Fix: `dacWrite(ioPin1, 0)` im SSR-Init durch `dac_output_disable(DAC_CHANNEL_1/2)` ersetzt (`<driver/dac.h>`). Zusätzlich: `disableActuators()` überspringt `dacWrite` wenn `initActuator == SSR`, um Reaktivierung des DAC beim Abschalten zu verhindern. |

### SRAM / Heap-Optimierungen

> **Phase 4c abgeschlossen (2026-04-21)** — M2, M3 und M4 sind jetzt unblocked. Neue Baseline: RAM 28.5% / 93.460 B (miniV3). Heap-Profil hat sich durch ArduinoJson v7 verändert (kein `DynamicJsonBuffer`-Wachstum mehr) → Messungen können jetzt mit stabiler Baseline durchgeführt werden.

| ID | Dateien | Problem | Effekt | Wann |
|----|---------|---------|--------|------|
| M1 | ~~`WebHandler.cpp` (11×), `API.cpp:494`, +5 weitere~~ | **BEHOBEN (2026-04-20):** Alle unsized `DynamicJsonBuffer` auf `DynamicJsonBuffer(Settings::jsonBufferSize)` gesetzt. War Ursache des `setSystem`-WDT (Heap-Korruption über 40–80s). | Bis 1.800 B weniger Peak-Heap + keine Fragmentierung | ✅ Erledigt |
| M2 | `Mqtt.cpp:188,237,258,274` | Arduino `String` bei jeder MQTT-Nachricht (1×/s) → ~36.000 Heap-Ops/h | Heap-Fragmentierung über Zeit verhindern | **Ausstehend** |
| M3 | `API.cpp:139–140` | `String sc[2]/vc[2]` lokal in `pitAry()` → Heap-Allokation jede Sekunde. | Gering, aber summiert sich | **Ausstehend** |
| M4 | `Connect.cpp:49`, `OtaUpdate.cpp:49` | Task-Stacks je 10.000 B — vermutlich überdimensioniert | **7.000–9.000 B dauerhaftes SRAM** | **Ausstehend** (T1-Safety-Check zuerst) |

**Vorgehen M4:** `highWaterMark`-Debug-Output aktiv in `main.cpp:89/124`, `Bluetooth.cpp:423`. T1 = Safety-Check unter Last, dann mit 512 B Puffer kürzen.

## Dependency Upgrade Guide

Stand: 2026-04-22

### Abgeschlossen ✅

| Dep | Alt | Neu | Anmerkung |
|-----|-----|-----|-----------|
| **Arduino ESP32 Framework** | `tuniii/arduino-esp32#WifiFix` (1.x) | `espressif32@^6.0.0` (2.x / ESP-IDF 4.4.x) | WiFi-Patch via `extra_script.py`; Hardware-Test miniV3 weitgehend bestanden (Details siehe Phase 1) |
| **AsyncTCP** | `me-no-dev@1.1.1` → `mathieucarbou@^3.0.0` | `ESP32Async/AsyncTCP@^3.4.10` | `mathieucarbou`-Repo seit Jan 2025 archiviert; Nachfolger-Org `ESP32Async`; Drop-in kompatibel |
| **ESPAsyncWebServer** | `me-no-dev#1dde9cf` → `mathieucarbou@^3.0.0` | `ESP32Async/ESPAsyncWebServer@^3.10.3` | `mathieucarbou`-Repo seit Jan 2025 archiviert; Breaking: `WebRequestMethodComposite` kein `int32_t` mehr ab v3.10.x (siehe Migrationshinweise) |
| **ArduinoJson** | 5.13.4 | 7.x (Phase 4c, 2026-04-21) | 20 Dateien, ~404 API-Aufrufe; `DynamicJsonBuffer` → `JsonDocument`, `JsonObject&` → Value-Typ, `parseObject` → `deserializeJson`; kein Heap-Wachstum mehr; `double_with_n_digits()` entfernt |

### Nicht upgraden – Breaking Changes (dedizierter Sprint nötig)

| Dep | Aktuell | Ziel | Phase | Grund |
|-----|---------|------|-------|-------|
| **LVGL** | 7.11.0 | 9.x | **5** | Widgets umbenannt, Event-System geändert; gesamtes `src/display/tft/` neu. Erst nach Phase 4b sinnvoll. |
| **Vue** | 2.6.11 | 3.x | **2** | Vue 2 EOL seit 31.12.2023; vollständiger Frontend-Neubau; eigenständiges Projekt. |

### Upgrade empfohlen – Phase 4a (vor Kernel 3.x)

| Dep | Aktuell | Ziel | Aufwand | Hinweis |
|-----|---------|------|---------|---------|
| **AsyncMqttClient** | 0.8.2 | `ESP32Async/AsyncMqttClient` | Mittel | **Blocker für Phase 4b** — ohne diesen Wechsel kompiliert Kernel 3.x nicht |
| **TFT_eSPI** | `^2.5.31,<2.5.34` | `^2.5.34` | **Trivial – Einzeiler** | `<2.5.34`-Constraint war Workaround für arduino-esp32 1.x (`hal/gpio_ll.h`); mit ESP-IDF 4.4.x (Phase 1) obsolet |
| **ESPRandom** | 1.4.1 | neueste | Trivial | Upgrade macht `patch_esp_random()` in `extra_script.py` überflüssig |
| **asyncHTTPrequest** | 1.2.2 | neueste 1.x | Trivial | Kleine Fixes, abwärtskompatibel |
| **ArduinoLog** | 1.0.4 | ~1.1.x | Trivial | Patch-kompatibel |
| **esp8266-oled-ssd1306** | 4.0.0 | neueste | Testen | Patch-Releases kompatibel |
| **OneButton** | 1.3.0 | 2.x | Gering | v2 hat neue Multi-Click-API; alte Callbacks funktional; kurz testen |
| **axios** (Frontend) | 0.21.1 | 1.x | Gering–Mittel | Besseres Error-Handling |

### Sicher zu upgraden

| Dep | Aktuell | Bemerkung |
|-----|---------|-----------|
| **ArduinoLog** | 1.0.4 | Patch-kompatibel |
| **ESPRandom** | 1.4.1 | Bug (fehlendes `#include <vector>`) wird via `extra_script.py` gepatcht; Upgrade würde Patch überflüssig machen |
| **asyncHTTPrequest** | 1.2.2 | Kleine Fixes, abwärtskompatibel |
| **esp8266-oled-ssd1306** | 4.0.0 | Patch-Releases kompatibel |
| **OneButton** | 1.3.0 | v2 hat neue Multi-Click-API; alte Callbacks weiter funktional |
| **core-js** (Frontend) | ^3.6.5 | Polyfills, abwärtskompatibel |
| **sass / sass-loader** (Frontend) | 1.26 / 9.x | Aktuellste 1.x / 10.x |

### Custom Forks – kein Upstream-Equivalent

Pflege nur durch manuellen Fork-Vergleich möglich; kein regulärer Upgrade-Pfad:

- `tuniii/BBQduino`, `tuniii/ITEADLIB_Arduino_Nextion`, `tuniii/ESPNexUpload`, `tuniii/lv_lib_qrcode`, `tuniii/Arduino-MedianFilter`, `borisneubert/Time`
