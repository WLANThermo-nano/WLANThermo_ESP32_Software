# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Language
Always respond in German (Deutsch), regardless of file contents or code comments.

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
- **Feature-Branch-Schema:** `feature/phase-0-stabilization`, `feature/phase-2-vue3`, `feature/phase-4a-lib-upgrade`, `feature/phase-5-lvgl9`

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
- **Custom Framework:** `platform_packages` in `[env]` zeigt auf `WLANThermo-nano/arduino-esp32@2.0.17-pm-enable` — ein Custom-Build von arduino-esp32 2.x mit `CONFIG_PM_ENABLE=y`. Zum Deaktivieren diese Zeile in `platformio.ini` auskommentieren.
- WiFi fix (`WIFI_ALL_CHANNEL_SCAN` — connect to strongest AP) is set at runtime in `src/Wlan.cpp` after each `WiFi.begin()` call via `esp_wifi_set_config()`.
- `extra_script.py` patches `ESPRandom@1.4.1` (missing `#include <vector>`) at build time via `patch_esp_random()`. Idempotent.
- WiFi event enums: `ARDUINO_EVENT_WIFI_STA_GOT_IP`, `ARDUINO_EVENT_WIFI_STA_DISCONNECTED`, `ARDUINO_EVENT_WIFI_AP_STADISCONNECTED` (see `src/Wlan.cpp`).
- Async networking uses `ESP32Async/AsyncTCP@^3.4.10` + `ESP32Async/ESPAsyncWebServer@^3.10.3` (Nachfolger-Repos; `mathieucarbou/*` seit Januar 2025 archiviert). AsyncTCP läuft auf Core 1 (`-DCONFIG_ASYNC_TCP_RUNNING_CORE=1` in `[env]` build_flags) — gleicher Core wie MainTask/ConnectTask, verhindert Inter-Core-Synchronisationsoverhead.
- Logging uses `ArduinoLog` macros (`Log.verbose`, `Log.notice`, `Log.error`) — not `Serial.print`.
- Settings persistence: `src/Settings.cpp` uses **NVS** (`Preferences` API) — NOT SPIFFS. SPIFFS wird nur für Cloud-URL-Cache (`Cloud.cpp`) und Nextion-Display-Updates verwendet.
- Power management: `esp_pm_config_esp32_t` + `esp_pm_configure()` in `src/system/SystemBase.cpp`. `CONFIG_PM_ENABLE=y` ist im Custom-Framework aktiv. `setPowerSaveMode()` konfiguriert Light Sleep mit `max_freq_mhz=240`, `min_freq_mhz=40` (XTAL-Frequenz — Pflicht für Light Sleep auf ESP32). Bei jedem Fehler (`ESP_ERR_NOT_SUPPORTED` oder `ESP_ERR_INVALID_ARG`) wird `powerSaveModeSupport=false` gesetzt → kein Retry. miniV3, connectV1, nanoV3 setzen `powerSaveModeSupport=true` in `hwInit()`.
- **I2C + DFS:** `wirePmHandle` in `SystemBase` verwendet `ESP_PM_APB_FREQ_MAX` (nicht `NO_LIGHT_SLEEP`). `ESP_PM_NO_LIGHT_SLEEP` verhindert nur Light Sleep, aber **nicht** APB-Taktfrequenz-Änderungen (DFS). `ESP_PM_APB_FREQ_MAX` verhindert beides — notwendig, da I2C-Takt APB-relativ ist (`Wire.setClock(700kHz)`): beim APB-Abfall 240→40 MHz würde der I2C-Takt auf ~116 kHz fallen und laufende Transaktionen korrumpieren. Betrifft NanoV3 (8× MAX11615 + OLED via I2C) und alle Varianten mit aktivem PSM auf Batterie.
- **Recovery-Mode-Trigger aus async_tcp-Kontext:** `Wlan::setRecoveryPending()` (Flag) statt direktem `RecoveryMode::runFromApp()`-Aufruf. `runFromApp()` enthält `delay()` → würde async_tcp blockieren und Response-Flush verhindern. `Wlan::update()` (ConnectTask) verarbeitet das Flag sicher außerhalb von async_tcp. Gleiches Muster wie `mdnsUpdatePending` / `wlanSaveConfigPending`.
- **Restart nach async_tcp-Response:** `esp_timer` (One-Shot, ≥2s) statt `delay() + ESP.restart()` im Handler-Kontext. `delay()` in async_tcp-Callback blockiert den Task; `WiFi.disconnect()` unmittelbar nach `request->send()` killt die TCP-Verbindung vor dem Flush. Der Timer feuert in einem separaten Kontext, async_tcp kann die Response in der Zwischenzeit senden.
- **ESPAsyncWebServer Migrationshinweise:**
  - **3.x (mathieucarbou → ESP32Async):** `AsyncWebHandler::canHandle()` und `isRequestHandlerTrivial()` sind jetzt `const virtual` → alle Subklassen müssen `const override` verwenden. Body-Daten in `handleBody()` sind **nicht null-terminiert** → null-terminierte Kopie (`new uint8_t[len+1]`) vor `deserializeJson()` erstellen. `AsyncJsonResponse::getRoot()` gibt `JsonVariant` zurück → `.to<JsonObject>()` aufrufen.
  - **3.10.x (ESP32Async):** `WebRequestMethodComposite` ist jetzt eine eigene Klasse (kein `int32_t` mehr). Struct-Felder für HTTP-Methoden müssen `WebRequestMethodComposite` statt `int32_t` sein. Literal `0` als Initialisierer → `HTTP_UNKNOWN`. Vergleich `(request->method() & field) > 0u` → `field & request->method()` (operator& gibt `bool` zurück; Operandenreihenfolge: Composite links, `WebRequestMethod` rechts).

## Project Roadmap

**Ziele:** UI-Redesign, App-Store-Publishing (Android + iOS), Heap-Stabilität  
**Reihenfolge:** Phase 0 (Stabilisierung) → Phase 1+2 (parallel: Kernel 2.x / UI) → Phase 3 (App Store) → Phase 4a (Lib Pre-Upgrade) → Phase 4c (ArduinoJson 7) → Phase 5 (LVGL 9)

**Mobile App:** Flutter-App in Branch `android-build-test` (PR #180, Repo: `WLANThermo-nano/WLANThermo_ESP32_Software`). Flutter WebView-Wrapper lädt `assets/html/index.html` (Vue.js Build) mit JS-Bridge für mDNS, Firebase FCM, Permissions. Cordova (`/mobile/`) wird dadurch abgelöst.

### Phase 0 — Stabilisierung (teilweise abgeschlossen)

Bug-Fixes: B1–B8 gefixt 2026-04-22, B18–B22 gefixt 2026-04-25 (NanoV3 TG1WDT-Reboot), B23–B28 gefixt 2026-04-25 (/wlanthermo-review WebHandler+API), B29–B32 gefixt 2026-04-25 (/wlanthermo-review Wlan.cpp+.h), B33–B35 gefixt 2026-04-25 (RecoveryMode Fixes), `next`-Branch. SRAM-Optimierungen M2–M4 ausstehend — siehe Abschnitte unten.

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
| Power Management | ✅ OK (2026-04-24) | Custom-Framework `2.0.17-pm-enable` mit `CONFIG_PM_ENABLE=y` via `platform_packages`. `setPowerSaveMode()` fix: `min_freq_mhz` 240→40. `PSM: enabled` im Serial-Monitor bestätigt. WiFi Modem Sleep (`WIFI_PS_MIN_MODEM`) aktiv. Strommessung ausstehend. |
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
| `setPowerSaveMode()` retryt endlos bei NOT_SUPPORTED | `src/system/SystemBase.cpp` | `CONFIG_PM_ENABLE` nicht im vorkompilierten SDK → `esp_pm_configure()` gibt immer `ESP_ERR_NOT_SUPPORTED`. `powerSaveModeEnabled` wird nie gesetzt → jeder Update-Zyklus ruft es erneut auf. Fix: bei jedem Fehler wird `powerSaveModeSupport = false` gesetzt. |
| PCA9533-Startup-Pause (~170ms) | `src/display/tft/DisplayTft.cpp` | arduino-esp32 2.x ersetzt den alten Bit-Bang-I2C-Treiber durch den ESP-IDF Hardware-Treiber. Jede fehlgeschlagene I2C-Transaktion (NACK) hat ~10ms FreeRTOS/Treiber-Overhead statt < 1ms in 1.x. Wenn PCA9533 (Adresse `0x62`) nicht verbaut ist, liefen trotzdem 17 Transaktionen (5× `init()` + 1× `ping()` + 4× `setMODE()` mit `getReg`+`setReg` + 3× sonstige) → ~170ms Pause im Boot-Log bei „Setup LED Controller: 2". Fix: `ping()` als Guard voranstellen; `init()` und alle weiteren Calls nur wenn `ping() == 0`. |

**Stack-Messwerte miniV3 (Frühphase, unter Grundlast):**
- MainTask: ~1.636 B genutzt (von 8.000 B) → nach Langzeittest + OTA-Durchlauf auf ~2.200 B reduzierbar
- ConnectTask: ~1.788 B genutzt (von 12.000 B) → nach Langzeittest auf ~2.300 B reduzierbar
- Bluetooth-Task: Messung ausstehend

**Hardware-Test nanoV3 (2026-04-25) — Ergebnis:**

| Bereich | Status | Anmerkung |
|---------|--------|-----------|
| Boot / Stabilität | ✅ Fix implementiert (B18–B22), Hardware-Test ausstehend | `TG1WDT_SYS_RESET` ~1–2s nach Task-Start; Root Cause: `ESP_PM_NO_LIGHT_SLEEP` verhinderte kein DFS → I2C korrupt bei APB-Abfall. Fix: `ESP_PM_APB_FREQ_MAX`. Zwei NanoV3-Geräte betroffen. |
| Übrige Funktionen | ⏳ Ausstehend | Erst nach Stabilitätsbestätigung |

### Phase 2 — UI-Redesign (ausstehend)

Vue 2 → Vue 3 (Neubau, Vue 2 seit 31.12.2023 EOL).

### Phase 3 — App Store (ausstehend)

Flutter-App fertigstellen, Cordova ablösen.

### Phase 4a — Library Pre-Upgrade ✅ ABGESCHLOSSEN (2026-04-24)

Alle regulären Libs auf aktuelle Versionen gehoben. Custom Forks ohne Upstream-Equivalent bleiben unverändert.

| Lib | Alt | Neu | Status |
|-----|-----|-----|--------|
| ~~**`AsyncMqttClient@0.8.2`**~~ | ~~`me-no-dev`~~ | ~~`mathieucarbou`~~ | ✅ **2026-04-23** → `marvinroger/AsyncMqttClient@0.9.0`. Kein API-Umbau. Commit `db83e5c`. |
| **TFT_eSPI** | `^2.5.31,<2.5.34` | `^2.5.34` | ✅ **2026-04-24** — Constraint für arduino-esp32 1.x war obsolet. Commit `901e744`. |
| **ArduinoLog** | `~1.0.3` (1.0.4) | `^1.1.1` | ✅ **2026-04-24** — Breaking: `loggingPrefix`-Signatur in `main.cpp` um `int level` erweitert. Commit `901e744`. |
| **esp8266-oled-ssd1306** | `#4.0.0` | `#4.6.2` | ✅ **2026-04-24** — Drop-in kompatibel. Commit `901e744`. |
| **OneButton** | `#1.3.0` | `#2.6.2` | ✅ **2026-04-24** — v2 API abwärtskompatibel für bestehende Callbacks. Commit `901e744`. |
| **asyncHTTPrequest** | `^1.2.1` | `^1.2.1` | ✅ Bereits auf 1.2.2 (neueste 1.x) — kein Constraint-Update nötig. |
| **ESPRandom** | `1.4.1` | `1.4.1` | ⏳ Kein neueres Release im PlatformIO-Registry. Patch in `extra_script.py` bleibt. |
| Custom Forks (`tuniii/*`, `borisneubert/Time`) | — | — | ⏳ Kein regulärer Upgrade-Pfad — manueller upstream-Diff bei Bedarf. |

---

### ~~Phase 4b — Kernel 3.x / ESP-IDF 5.x~~ ENTFÄLLT

Kernel-Upgrade auf arduino-esp32 3.x wird nicht durchgeführt. Das ursprüngliche Hauptziel (Power Save) wurde auf Kernel 2.x gelöst: Custom-Framework `WLANThermo-nano/arduino-esp32@2.0.17-pm-enable` mit `CONFIG_PM_ENABLE=y` via `platform_packages` in `platformio.ini` (2026-04-24).

---

### Phase 4c — ArduinoJson 7 Migration ✅ ABGESCHLOSSEN (2026-04-21)

Dedizierter Sprint.

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

**Compile-Ergebnis miniV3 (2026-04-21, nach Phase 4c):**
- RAM: 28.5% (93.460 B / 327.680 B)
- Flash: 32.0% (2.011.261 B / 6.291.456 B)

**Compile-Ergebnis miniV3 (2026-04-25, nach B33–B35):**
- RAM: 27.4% (89.788 B / 327.680 B)
- Flash: 30.2% (1.902.485 B / 6.291.456 B)

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
| Recovery-Mode starten | ✅ OK (B33 gefixt 2026-04-25) | `handleRecovery()` rief `runFromApp()` im async_tcp-Kontext auf → `restart.html` nie ausgeliefert → 3–5 Versuche nötig. Fix: Pending-Flag → ConnectTask |
| Recovery-Seite (Auto-Redirect) | ✅ OK (B15+B33 gefixt) | B15: `onerror`-Handler ergänzt. B33: Root Cause der Unzuverlässigkeit war async_tcp-Blockierung in `handleRecovery()` |
| Recovery-Export | ✅ OK (fix 2026-04-22) | `beginResponse_P` mit lokalem String-Pointer → HTTP-Header als Dateiinhalt (B14 gefixt) |
| Recovery-Import | ⏳ Ausstehend | Noch nicht getestet |
| Recovery-Update (Restart) | ✅ OK (B16+B34 gefixt) | B16: `ESP.restart()` nach Upload ergänzt. B34: `WiFi.disconnect()` killt TCP vor Response-Flush → durch `esp_timer` (2s) ersetzt |
| Recovery-Upload Fortschritt | ⏳ Ausstehend | Upload langsam + hängt unter Last: `Update.write()` im async_tcp-Kontext blockiert TCP-Empfangspuffer → TCP-Backpressure → Browser drosselt. B35 (silent failure) gefixt; strukturelles Problem (Flash-Writes in async_tcp) offen |
| REST-API (`/data`, `/settings`) | ✅ OK | |
| Display-Funktionen | ✅ OK | |
| Standby-Erkennung | ✅ OK | |
| WiFi-Stabilität | ⏳ Langzeittest läuft | Gerät läuft über Nacht; Ergebnis ausstehend |

**Gesamtergebnis:** Solide — alle Kernfunktionen OK. Drei Recovery-Bugs gefixt (B14–B16), SSR DAC/LEDC-Konflikt gefixt (B17). Recovery-Zuverlässigkeit nachträglich durch B33+B34 (2026-04-25) vollständig behoben.

---

### Phase 5 — LVGL 9 + TFT-UI Neubau (Long-term, ausstehend)

Voraussetzung: Phase 4c abgeschlossen (saubere JSON-API im TFT-UI-Code).

- LVGL 7 → 9: Widgets umbenannt, Event-System geändert — gesamtes `src/display/tft/` muss neu geschrieben werden.
- Sinnvoll zu koppeln mit Phase 2 (Vue 3 UI-Redesign), da beide das UI betreffen.

## Known Issues (pending fixes)

### Bugs

| ID | Datei | Zeile | Severity | Problem |
|----|-------|-------|----------|---------|
| ~~B1~~ | `src/WebHandler.cpp` | `setDCTest()` | ~~Critical~~ **GEFIXT (2026-04-22)** | NULL-Pointer auf `pitmasters[id]` — Entwickler-TODO. Fix: NULL-Check vor `startDutyCycleTest()`. Commit `c86f4f5`. |
| ~~B2~~ | `src/WebHandler.cpp` | `setChannels()` | ~~High~~ **GEFIXT (2026-04-22)** | `num--` ergab -1 wenn JSON `"number": 0` → out-of-bounds auf `gSystem->temperatures[-1]`. Fix: Guard `if (num < 1) return 0` vor Dekrement. Commit `a0ecaad`. |
| ~~B3~~ | `src/WebHandler.cpp` | `setIoT()` | ~~High~~ **GEFIXT (2026-04-22)** | `strcpy()` in 30-Byte-MqttConfig-Felder (host, user, password) ohne Längencheck → Stack-Korruption bei überlangem Input. Fix: `strncpy()` mit `sizeof-1` + explizitem Null-Terminator. Commit `aa9a067`. |
| ~~B4~~ | `src/WebHandler.cpp` | `setPush()` | ~~High~~ **GEFIXT (2026-04-22)** | `strncpy()` mit `sizeof()` statt `sizeof()-1` → kein Null-Terminator wenn Puffer voll (telegram.token/chatId, pushover.token/userKey). Fix: `sizeof()-1` + expliziter Null-Terminator. Commit `b36fea1`. |
| ~~B5~~ | `src/WebHandler.cpp` | `setPush()` | ~~High~~ **GEFIXT (2026-04-22)** | Zwei Fehler: (1) Längencheck für name/id nutzte `&&` statt `\|\|` → überlanger name passierte wenn id kurz genug war. (2) `androidchannelid` hatte keinen Längencheck vor `strcpy()`. Fix: `\|\|` + zusätzlicher Längencheck für androidchannelid. Commit `f29408b`. |
| ~~B6~~ | `src/pitmaster/Pitmaster.cpp` | Konstruktor | ~~Medium~~ **GEFIXT (2026-04-22)** | `globalIndexTracker++` nicht atomar auf Xtensa-Dual-Core → beide Pitmaster-Objekte könnten denselben `globalIndex` bekommen. Fix: `__sync_fetch_and_add()` (GCC-Atomic-Built-in, lock-frei). Commit `ffde720`. |
| ~~B7~~ | `src/pitmaster/Pitmaster.cpp` | `update()` | ~~High~~ **GEFIXT (2026-04-22)** | `this->temperature` ist im Konstruktor NULL und wird erst durch `setTemperature()` gesetzt. `checkOpenLid()` und `pidCalc()` greifen ohne Prüfung zu → Hard-Fault. Fix: Early-Return am Anfang von `update()` wenn `temperature == NULL`. Commit `255d3fa`. |
| ~~B8~~ | `src/connect/Connect.cpp` | `onReadyStateChange()` | ~~High~~ **GEFIXT (2026-04-22)** | Zugriff auf `connectDevices[0]` ohne Leer-Check — wenn Callback vor Registrierung eines Geräts feuert, ist der Vector leer → UB. Fix: `if (connectDevices.empty()) return` vor dem ersten Zugriff. Commit `709ae42`. |
| ~~B9~~ | `src/WebHandler.cpp` | `handleBody()` | ~~High~~ **GEFIXT (2026-04-20)** | Body-Daten von `mathieucarbou/ESPAsyncWebServer@^3.0.0` sind nicht null-terminiert → Crash in `setSystem` (`parseObject` las über Puffer hinaus). Fix: `handleBody()` erstellt jetzt null-terminierte Kopie (`new uint8_t[len+1]`) vor dem Handler-Aufruf. Gilt für alle Body-Handler. |
| ~~B11~~ | `src/WebHandler.cpp` | `setBluetooth()` | ~~High~~ **GEFIXT (2026-04-20)** | (1) Fehlendes `return`-Statement → Undefined Behavior (UB) in GCC 8.x. (2) `DynamicJsonBuffer jsonBuffer;` ohne Größe → ArduinoJson 5 startet mit 256-Byte-Slab → Heap-Korruption unter Speicherdruck. Fix: `return 1;` ergänzt, `DynamicJsonBuffer(Settings::jsonBufferSize)` gesetzt. Symptom: `CORRUPT HEAP: Bad head ... Expected 0xabba1234`. |
| ~~B12~~ | `src/temperature/TemperatureGrp.cpp` | `saveConfig()` | ~~Medium~~ **GEFIXT (2026-04-20)** | `DynamicJsonBuffer jsonBuffer;` ohne Größe → selbes M1-Problem wie B11. Fix: `DynamicJsonBuffer(Settings::jsonBufferSize)`. |
| ~~B13~~ | `src/WebHandler.cpp` | `setSystem()` | ~~High~~ **GEFIXT (2026-04-20)** | 4 NVS-Writes hintereinander → Task-Watchdog (WDT) auf `async_tcp`. Root Cause: `MainTask`, `ConnectTask` und `async_tcp` laufen alle auf **Core 1** (siehe `main.cpp:160,169`). NVS-Writes sperren Core 1 für die Flash-Write-Dauer → async_tcp kann keinen WDT-Reset ausführen. Erster Fix (Auslagern zu ConnectTask) half nicht, da ConnectTask ebenfalls Core 1. Zweiter Fix: **Stückelung** — `processPendingSave()` schreibt genau **ein** NVS-Key pro Aufruf. ConnectTask ruft es jede Sekunde auf (`TASK_CYCLE_TIME_CONNECT_TASK=1000ms`). 4 Writes = 4 Sekunden verteilt, async_tcp kann dazwischen WDT zurücksetzen. Flags: `systemConfigSavePending`, `otaConfigSavePending`, `wlanConfigSavePending`, `tempConfigSavePending`. |
| ~~B10~~ | `src/system/SystemBase.cpp` | `setPowerSaveMode()` | ~~Medium~~ **GEFIXT (2026-04-24)** | Mit Standard-SDK (CONFIG_PM_ENABLE=n) gab `esp_pm_configure()` immer `ESP_ERR_NOT_SUPPORTED` → Endlos-Retry. Zweites Problem: `min_freq_mhz=240` mit `light_sleep_enable=true` → `ESP_ERR_INVALID_ARG` (ESP-IDF verlangt XTAL-Frequenz 40 MHz). Fix: (1) `min_freq_mhz` 240→40, (2) bei jedem Fehler `powerSaveModeSupport=false`. Power Save aktiv via Custom-Framework `2.0.17-pm-enable`. |
| ~~B14~~ | `src/RecoveryMode.cpp` | `/export`-Handler | ~~High~~ **GEFIXT (2026-04-22)** | `beginResponse_P(200, "text/text", (uint8_t*)exportSettings.c_str(), ...)` speichert nur Raw-Pointer auf lokale `String exportSettings`. Nach Lambda-Return wird String zerstört → dangling pointer. Async-Webserver liest beim Senden freien Speicher → Browser empfängt HTTP-Header als Dateiinhalt statt NVS-Keys. Fix: `beginResponse(200, "text/plain", exportSettings)` — kopiert String-Inhalt intern. |
| ~~B15~~ | `webui/old/restart.html` | XHR-Polling-Loop | ~~Medium~~ **GEFIXT (2026-04-22)** | `xhr.onerror` nicht behandelt. Wenn ESP nach `/recovery`-Aufruf WLAN trennt (Recovery-Reboot), bekommt Browser Network Error (kein Timeout) → `onerror` feuert ohne Handler → Polling stoppt → Seite zeigt Spinner ewig. Benutzer musste `/recovery` manuell ein zweites Mal aufrufen. Fix: `xhr.onerror`-Handler ergänzt, der nach 1s Pause erneut `/ping` sendet. |
| ~~B16~~ | `src/RecoveryMode.cpp` | `/uploadfile`-POST-Handler | ~~High~~ **GEFIXT (2026-04-22), verfeinert B34 (2026-04-25)** | Nach `Update.end(true)` kein `ESP.restart()` → Firmware-Update im Recovery-Mode ohne automatischen Neustart. Fix (B16): `WiFi.disconnect()` + 1s delay + `ESP.restart()` im Response-Handler. Fix (B34): `WiFi.disconnect()` entfernt + `delay()` durch `esp_timer` (2s) ersetzt — sonst killt WiFi.disconnect() die TCP-Verbindung vor dem Response-Flush. |
| ~~B17~~ | `src/pitmaster/Pitmaster.cpp` | `initActuators()` SSR-Fall + `disableActuators()` | ~~High~~ **GEFIXT (2026-04-22)** | SSR keine Reaktion nach Phase 1 Migration. Root Cause: `dacWrite(ioPin1, 0)` aktiviert das DAC-Peripheral auf GPIO 25/26 (RTC-Pad). In ESP-IDF 4.4.x hat das DAC Vorrang über das GPIO-Matrix-Signal — LEDC läuft über die GPIO-Matrix und wird vom aktiven DAC blockiert. `ledcAttachPin()` deaktiviert das DAC in ESP-IDF 4.x nicht automatisch (in ESP-IDF 1.x funktionierte das noch). Fix: `dacWrite(ioPin1, 0)` im SSR-Init durch `dac_output_disable(DAC_CHANNEL_1/2)` ersetzt (`<driver/dac.h>`). Zusätzlich: `disableActuators()` überspringt `dacWrite` wenn `initActuator == SSR`, um Reaktivierung des DAC beim Abschalten zu verhindern. |
| ~~B18~~ | `src/system/SystemBase.cpp` | Konstruktor | ~~Critical~~ **GEFIXT (2026-04-25)** | NanoV3 `rst:0x8 (TG1WDT_SYS_RESET)` ~1–2s nach Task-Start beim Betrieb auf Batterie. Root Cause: `wirePmHandle` als `ESP_PM_NO_LIGHT_SLEEP` angelegt → verhindert Light Sleep, aber **nicht** APB-DFS-Frequenzänderungen. Wenn PSM erstmals aktiviert wird (erster `ONCE_PER_SECOND_CYCLE` ~600ms nach Tasks-Start), fällt APB von 240→40 MHz. I2C-Takt ist APB-relativ (`Wire.setClock(700kHz)`) → fällt auf ~116 kHz → korrumpiert laufende Transaktionen der 8 MAX11615-Sensoren / OLED → FreeRTOS-Tick-Interrupt blockiert >300ms → Interrupt-WDT. Fix: `ESP_PM_NO_LIGHT_SLEEP` → `ESP_PM_APB_FREQ_MAX` in Konstruktor. miniV3 unbetroffenen da beim Test per USB betrieben → PSM nie aktiviert. |
| ~~B19~~ | `src/peripherie/Buzzer.cpp` | Konstruktor | ~~Low~~ **GEFIXT (2026-04-25)** | `ledcSetup(channel, 0, 8)` mit `frequency=0` → LEDC meldet "LEDC not initialized" + "LEDC not initialized" bei `ledcAttachPin()` im Boot-Log. Fix: Startfrequenz 0 → 4000 Hz. |
| ~~B20~~ | `src/pitmaster/Pitmaster.cpp` | `disableActuators()` | ~~Medium~~ **GEFIXT (2026-04-25)** | B17-Fix unvollständig: Guard deckte nur `initActuator == SSR` ab. Bei `initActuator == NOAR` (Initialzustand — kein Aktuator konfiguriert) wurde `dacWrite(ioPin1=25, 0)` trotzdem aufgerufen → aktiviert DAC auf GPIO 25 bei jedem Boot, bevor SSR-LEDC initialisiert wird. Fix: Guard um `initActuator != NOAR` erweitert. |
| ~~B21~~ | `src/Wlan.cpp` | `onWifiConnect()` | ~~Medium~~ **GEFIXT (2026-04-25)** | `updateMdns()` direkt im WiFi-Event-Callback — gleicher unfixter Code wie der ursprüngliche `setHostName()`-Bug (B, bereits gefixt in `setHostName`). `MDNS.begin()` blockiert mehrere Sekunden in arduino-esp32 2.x → kann WDT im Event-Task auslösen. Fix: `updateMdns()` → `mdnsUpdatePending = true`, Abarbeitung im ConnectTask via `Wlan::update()`. |
| ~~B22~~ | `src/main.cpp` | `loop()` | ~~Low~~ **GEFIXT (2026-04-25)** | `vTaskDelete(NULL)` ohne vorheriges `esp_task_wdt_delete(NULL)` → Loop-Task war noch im WDT-Monitor registriert beim Löschen. Fix: `esp_task_wdt_delete(NULL)` vor `vTaskDelete(NULL)`. |
| ~~B23~~ | `src/WebHandler.cpp` | `setPitmaster()` | ~~Critical~~ **GEFIXT (2026-04-25)** | `byte cha = _pitmaster["channel"]` — bei `cha=0` ergibt `cha-1` unsigned-Underflow auf 255 → `temperatures[255]` → NULL → `assignTemperature(NULL)` → `temperature->getType()` in `Pitmaster.cpp:163` ohne NULL-Check → Hard-Fault. Fix: Guard `if (cha < 1 \|\| cha > temperatures.count()) return 0;` vor `cha-1`. |
| ~~B24~~ | `src/WebHandler.cpp` | `setPitmaster()` | ~~High~~ **GEFIXT (2026-04-25)** | `pm->getAssignedProfile()->id` — `this->profile` startet im Konstruktor als NULL (`Pitmaster.cpp:79`). Bei erstem Boot oder leerer NVS → NULL-Deref → Hard-Fault. Fix: `PitmasterProfile *currentProfile = pm->getAssignedProfile(); if (currentProfile && temppid != currentProfile->id)`. |
| ~~B25~~ | `src/WebHandler.cpp` | `setPitmaster()` | ~~High~~ **GEFIXT (2026-04-25)** | `getPitmasterProfile(temppid)` gibt NULL zurück wenn `temppid >= pitmasterProfileCount` — kein Bounds-Check. `assignProfile(NULL)` schreibt `this->profile = NULL` → nächster PID-Regelungs-Zugriff crasht. Fix: `if (temppid >= getPitmasterProfileCount()) return 0;` vor `assignProfile()`. |
| ~~B26~~ | `src/WebHandler.cpp` | `setSystem()` | ~~Medium~~ **GEFIXT (2026-04-25)** | `gSystem->temperatures.setUnit((TemperatureUnit)unit.charAt(0))` wurde bedingungslos aufgerufen, auch wenn `"unit"` gar nicht im JSON war. `String unit` bleibt leer → `charAt(0)` = `'\0'` → `setUnit(0)` resettet Einheit unbeabsichtigt. Fix: Aufruf in `if (!unit.isEmpty())` gewrapped. |
| ~~B27~~ | `src/WebHandler.cpp` | `canHandle()` | ~~Low~~ **GEFIXT (2026-04-25)** | `boolean supported = false;` — Arduino-Typedef `uint8_t`, nicht ISO-C++ `bool`. Fix: → `bool`. |
| ~~B28~~ | `src/API.cpp` | `customObj()` | ~~Low~~ **GEFIXT (2026-04-25)** | `_currentChannel["temp"] = (char*)0;` — C-Cast auf Null-Pointer statt `nullptr`. Fix: → `nullptr`. |
| ~~B29~~ | `src/Wlan.cpp` | `loadConfig()` | ~~High~~ **GEFIXT (2026-04-25)** | `strlen(wifiEntry["SSID"].as<const char*>())` — ArduinoJson v7 gibt `nullptr` zurück wenn Key fehlt/null ist → `strlen(nullptr)` = UB → Crash beim Laden einer korrupten NVS-Wifi-Konfiguration. Fix: Pointer zuerst in `const char*` zwischenspeichern, `!ssid \|\| !pass` als Guard voranstellen. |
| ~~B30~~ | `src/Wlan.cpp` | `onWifiConnect()` | ~~Medium~~ **GEFIXT (2026-04-25)** | `saveConfig()` direkt im WiFi-Event-Callback → `Settings::write()` → `prefs.putString()` → NVS-Write im Event-Task-Kontext. Inkonsistent mit dem Pending-Flag-Pattern; potenzieller Mutex-Konflikt mit `processPendingSave()`. Fix: `wlanSaveConfigPending = true`, Abarbeitung in `Wlan::update()` analog zu `mdnsUpdatePending`. |
| ~~B31~~ | `src/Wlan.cpp` | `connectToKnownStations()` | ~~Low~~ **GEFIXT (2026-04-25)** | `connectTimeout--` (`uint16_t`) bedingungslos am Funktionsende. Wenn alle SSID-Slots leer sind, läuft For-Loop durch ohne `connectTimeout` zu setzen → bleibt 0 → Underflow auf 65535 → Gerät versucht ~18h lang nicht erneut zu verbinden. Fix: Guard `if (connectTimeout > 0u) connectTimeout--`. |
| ~~B32~~ | `src/Wlan.h` + `src/Wlan.cpp` | `isConnected()`, `isAP()` | ~~Low~~ **GEFIXT (2026-04-25)** | `boolean` Rückgabetyp (Arduino-Typedef `uint8_t`) statt ISO-C++ `bool` — identisches Pattern wie B27. Fix: → `bool`. |
| ~~B33~~ | `src/WebHandler.cpp` + `src/Wlan.cpp/.h` | `handleRecovery()` | ~~Critical~~ **GEFIXT (2026-04-25)** | `runFromApp()` direkt im async_tcp-Handler aufgerufen. `delay(500)` darin blockierte den async_tcp-Task vor dem Response-Flush; `WiFi.disconnect()` killte danach die TCP-Verbindung → Browser empfing nie `restart.html` → kein Spinner → 3–5 Versuche nötig. Fix: `Wlan::setRecoveryPending()`-Flag, Abarbeitung in `Wlan::update()` (ConnectTask). |
| ~~B34~~ | `src/RecoveryMode.cpp` | `/uploadfile`-Completion-Handler | ~~High~~ **GEFIXT (2026-04-25)** | `WiFi.disconnect()` + `delay(1000)` + `ESP.restart()` im async_tcp-Kontext: `WiFi.disconnect()` killte TCP-Verbindung sofort nach `request->send()` → „200 OK" nie ausgeliefert → Browser empfing Network Error statt Erfolg. Fix: `esp_timer` (2s One-Shot, außerhalb async_tcp) ruft `ESP.restart()` auf; `WiFi.disconnect()` entfernt. |
| ~~B35~~ | `src/RecoveryMode.cpp` | `/uploadfile`-Upload-Callback | ~~Medium~~ **GEFIXT (2026-04-25)** | `Update.begin/write/end()` Rückgabewerte nicht geprüft → Silent failure bei ungültiger Partitionsgröße oder Flash-Fehler. Fix: Fehlercheck mit `Update.abort()` + `RMPRINTF` bei jedem Schritt; `uploadFileType = UploadFileType::None` bricht weiteren Upload ab. |

### SRAM / Heap-Optimierungen

> **Phase 4c abgeschlossen (2026-04-21)** — M2, M3 und M4 sind jetzt unblocked. Neue Baseline: RAM 28.5% / 93.460 B (miniV3). Heap-Profil hat sich durch ArduinoJson v7 verändert (kein `DynamicJsonBuffer`-Wachstum mehr) → Messungen können jetzt mit stabiler Baseline durchgeführt werden.

| ID | Dateien | Problem | Effekt | Wann |
|----|---------|---------|--------|------|
| M1 | ~~`WebHandler.cpp` (11×), `API.cpp:494`, +5 weitere~~ | **BEHOBEN (2026-04-20):** Alle unsized `DynamicJsonBuffer` auf `DynamicJsonBuffer(Settings::jsonBufferSize)` gesetzt. War Ursache des `setSystem`-WDT (Heap-Korruption über 40–80s). | Bis 1.800 B weniger Peak-Heap + keine Fragmentierung | ✅ Erledigt |
| M2 | `Mqtt.cpp:188,237,258,274` | Arduino `String` bei jeder MQTT-Nachricht (1×/s) → ~36.000 Heap-Ops/h | Heap-Fragmentierung über Zeit verhindern | **Ausstehend** |
| M3 | `API.cpp:139–140` | `String sc[2]/vc[2]` lokal in `pitAry()` → Heap-Allokation jede Sekunde. | Gering, aber summiert sich | **Ausstehend** |
| M4 | `Connect.cpp:49`, `OtaUpdate.cpp:49` | Task-Stacks je 10.000 B — vermutlich überdimensioniert | **7.000–9.000 B dauerhaftes SRAM** | **Ausstehend** (T1-Safety-Check zuerst) |

**Vorgehen M4:** `highWaterMark`-Debug-Output aktiv in `main.cpp:89/124`, `Bluetooth.cpp:423`, `SystemBase.cpp` (SystemTask, Stack 6000 B — nach 2026-04-25 erhöht von 3000 B). T1 = Safety-Check unter Last, dann mit 512 B Puffer kürzen.

## Dependency Upgrade Guide

Stand: 2026-04-24 (Phase 4a abgeschlossen)

### Abgeschlossen ✅

| Dep | Alt | Neu | Anmerkung |
|-----|-----|-----|-----------|
| **Arduino ESP32 Framework** | `tuniii/arduino-esp32#WifiFix` (1.x) | `espressif32@^6.0.0` (2.x / ESP-IDF 4.4.x) | WiFi-Patch via `extra_script.py`; Hardware-Test miniV3 weitgehend bestanden (Details siehe Phase 1) |
| **arduino-esp32 Custom Build** | Standard SDK (CONFIG_PM_ENABLE=n) | `WLANThermo-nano/arduino-esp32@2.0.17-pm-enable` via `platform_packages` (2026-04-24) | `CONFIG_PM_ENABLE=y`; Power Save auf Kernel 2.x nutzbar; `setPowerSaveMode()` fix: `min_freq_mhz` 240→40; WiFi Modem Sleep aktiviert |
| **AsyncTCP** | `me-no-dev@1.1.1` → `mathieucarbou@^3.0.0` | `ESP32Async/AsyncTCP@^3.4.10` | `mathieucarbou`-Repo seit Jan 2025 archiviert; Nachfolger-Org `ESP32Async`; Drop-in kompatibel |
| **ESPAsyncWebServer** | `me-no-dev#1dde9cf` → `mathieucarbou@^3.0.0` | `ESP32Async/ESPAsyncWebServer@^3.10.3` | `mathieucarbou`-Repo seit Jan 2025 archiviert; Breaking: `WebRequestMethodComposite` kein `int32_t` mehr ab v3.10.x (siehe Migrationshinweise) |
| **ArduinoJson** | 5.13.4 | 7.x (Phase 4c, 2026-04-21) | 20 Dateien, ~404 API-Aufrufe; `DynamicJsonBuffer` → `JsonDocument`, `JsonObject&` → Value-Typ, `parseObject` → `deserializeJson`; kein Heap-Wachstum mehr; `double_with_n_digits()` entfernt |
| **AsyncMqttClient** | 0.8.2 (`me-no-dev`) | `marvinroger/AsyncMqttClient@0.9.0` (2026-04-23) | `mathieucarbou`-Repo existiert nicht mehr; `ESP32Async` hat kein MQTT-Repo. 0.9.0 nutzt `<AsyncTCP.h>` direkt → kompatibel mit `ESP32Async/AsyncTCP@^3.4.10`. Kein API-Umbau. |
| **TFT_eSPI** | `^2.5.31,<2.5.34` | `^2.5.34` (2026-04-24) | Constraint für arduino-esp32 1.x obsolet; 2.5.43 aktuell |
| **ArduinoLog** | 1.0.4 | 1.1.1 (2026-04-24) | Breaking: `setPrefix`-Callback-Signatur `+int level`; `loggingPrefix` in `main.cpp` angepasst |
| **esp8266-oled-ssd1306** | 4.0.0 | 4.6.2 (2026-04-24) | Drop-in kompatibel |
| **OneButton** | 1.3.0 | 2.6.2 (2026-04-24) | v2 Multi-Click-API abwärtskompatibel für bestehende Callbacks |
| **asyncHTTPrequest** | `^1.2.1` | `^1.2.1` → 1.2.2 (2026-04-24) | Neueste 1.x durch Constraint bereits abgedeckt |

### Nicht upgraden – Breaking Changes (dedizierter Sprint nötig)

| Dep | Aktuell | Ziel | Phase | Grund |
|-----|---------|------|-------|-------|
| **LVGL** | 7.11.0 | 9.x | **5** | Widgets umbenannt, Event-System geändert; gesamtes `src/display/tft/` neu. |
| **Vue** | 2.6.11 | 3.x | **2** | Vue 2 EOL seit 31.12.2023; vollständiger Frontend-Neubau; eigenständiges Projekt. |

### Ausstehend

| Dep | Aktuell | Ziel | Hinweis |
|-----|---------|------|---------|
| **ESPRandom** | 1.4.1 | neueste | Kein neueres Release im Registry; Upgrade würde `patch_esp_random()` in `extra_script.py` überflüssig machen |
| **axios** (Frontend) | 0.21.1 | 1.x | Besseres Error-Handling |
| **core-js** (Frontend) | ^3.6.5 | neueste | Polyfills, abwärtskompatibel |
| **sass / sass-loader** (Frontend) | 1.26 / 9.x | neueste 1.x / 10.x | Abwärtskompatibel |

### Custom Forks – kein Upstream-Equivalent

Pflege nur durch manuellen Fork-Vergleich möglich; kein regulärer Upgrade-Pfad:

- `tuniii/BBQduino`, `tuniii/ITEADLIB_Arduino_Nextion`, `tuniii/ESPNexUpload`, `tuniii/lv_lib_qrcode`, `tuniii/Arduino-MedianFilter`, `borisneubert/Time`

---

## /wlanthermo-review Tracking

Skill-Aufruf: `/wlanthermo-review <Datei(en)>` — strukturierter Safety-Review nach Checkliste (Memory, FreeRTOS/Async, ArduinoJson v7, ESPAsyncWebServer 3.10.x, Input-Validation, ESP32-Fallstricke).

### Bereits reviewed

| Datum | Dateien | Befunde | Fixes |
|-------|---------|---------|-------|
| 2026-04-25 | `src/WebHandler.cpp`, `src/API.h`, `src/API.cpp` | B23 Critical, B24+B25 High, B26 Medium, B27+B28 Low | Alle gefixt (same session) |
| 2026-04-25 | `src/Wlan.cpp`, `src/Wlan.h` | B29 High, B30 Medium, B31+B32 Low | Alle gefixt (same session) |
| 2026-04-25 | `src/RecoveryMode.cpp`, `src/WebHandler.cpp` (handleRecovery) | B33 Critical, B34 High, B35 Medium | Alle gefixt (same session); Upload-Stall unter hoher Flash-Last noch nicht vollständig analysiert |

### Ausstehend — Priorisiert

**Priorität 1 — Netzwerk-/Input-facing (höchstes Risiko)**

| Datei(en) | Grund |
|-----------|-------|
| `src/Notification.cpp` + `.h` | String-Handling Push-Tokens (Telegram, Pushover, App), HTTP-Requests |
| `src/Mqtt.cpp` + `.h` | MQTT-Publish (M2-Kandidat: Arduino `String` 1×/s), Callback-Kontext |
| `src/Cloud.cpp` + `.h` | asyncHTTPrequest, SPIFFS-URL-Cache, String-Handling |
| `src/RecoveryMode.cpp` + `.h` | B33–B35 gefixt (2026-04-25); Import-Pfad noch nicht gereviewed; Upload-Stall unter Flash-Last offen |
| `src/OtaUpdate.cpp` + `.h` | M4-Kandidat (10kB Task-Stack), Update-Download-Handler |

**Priorität 2 — Pitmaster / Sensor (Hardware-Safety)**

| Datei(en) | Grund |
|-----------|-------|
| `src/pitmaster/Pitmaster.cpp` + `.h` | B6, B7, B17, B20 bereits gefixt; Autotune-Pfad und PID-Calc noch unreviewed |
| `src/pitmaster/PitmasterGrp.cpp` + `.h` | loadConfig/saveConfig, ArduinoJson v7 Migration |
| `src/temperature/TemperatureGrp.cpp` + `.h` | B12 bereits gefixt; loadConfig, Sensor-Registrierung |
| `src/temperature/TemperatureBase.cpp` + `.h` | Basisklasse, getValue/getType Null-Safety |
| `src/bluetooth/Bluetooth.cpp` + `.h` | B (remoteIndex), BLE-Callbacks, getDevicesJson() |
| `src/connect/Connect.cpp` + `.h` | B8 bereits gefixt; asyncHTTPrequest, onReadyStateChange |

**Priorität 3 — System / Settings**

| Datei(en) | Grund |
|-----------|-------|
| `src/system/SystemBase.cpp` + `.h` | B18/B10 gefixt; processPendingSave, PSM-Logik |
| `src/Settings.cpp` + `.h` | NVS-Lesen/Schreiben, Defaults |
| `src/main.cpp` | Task-Stacks, WDT-Registrierung (B22 gefixt) |
| `src/SerialCmd.cpp` + `.h` | Serial-Input-Parsing |
| `src/Item.cpp` + `.h` | NVS-Key-Handling |

**Priorität 4 — Display / Peripherie (geringes Netzwerk-Risiko)**

| Datei(en) | Grund |
|-----------|-------|
| `src/display/tft/` (alle `lv*.cpp`) | LVGL 7 — wird in Phase 5 neu geschrieben; Review erst sinnvoll danach |
| `src/display/DisplayNextion.cpp` + `.h` | Nextion-Protokoll, Serial-Parsing |
| `src/display/DisplayOled.cpp` + `.h` | I2C, relativ einfach |
| `src/peripherie/Battery.cpp`, `Buzzer.cpp`, `PbGuard.cpp`, `SdCard.cpp` | Einfache Hardware-Treiber |

**Überspringen (kein Review nötig)**

| Datei(en) | Grund |
|-----------|-------|
| `src/bluetooth/bleFirmware*.h` | Binärdaten (NRF52-Firmware-Blobs) |
| `src/webui/*.gz.h` | Generierte gzip-Header, kein handgeschriebener Code |
| `src/Constants.h`, `src/TaskConfig.h`, `src/Version.h`, `src/DbgPrint.h`, `src/MedianFilterFloat.h` | Reine Konstanten/Makros/Inline-Header ohne Logik |
| `src/system/System[Variant].cpp` (7 Dateien) | Dünne `hwInit()`-Wrapper, kaum eigene Logik |
| `src/temperature/Temperature[Sensor].cpp` (6 Sensor-Dateien) | Sensor-spezifische Lese-Logik, kein Netzwerk-Facing |
| `src/API.h` | Reine Deklaration (bereits gereviewed als Teil des WebHandler-Reviews) |
