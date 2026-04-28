# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Language
Always respond in German (Deutsch), regardless of file contents or code comments.

## Repository

- **Upstream/Remote:** `git@github.com:WLANThermo-nano/WLANThermo_ESP32_Software.git` (SSH)
- **GitHub-User:** `Phantomias2006`
- **Branches:**
  - `master` — drei eigene Commits auf Upstream-History (`225e87f` Phase 1, `a2fad1c` Phase 4c, `dc130ee` CLAUDE.md)
  - `develop` — Integration für Hotfixes und schnelle Releases (immer releasable)
  - `next` — Spielwiese für Phasen-Arbeit; `feature/*`-Branches landen hier zuerst, dann `develop` → `master`
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
> **intelhex fehlt:** `~/.platformio/penv/bin/pip install intelhex` (tritt nach PlatformIO-Neuinstallation auf).
> **16MB Flash (miniV3):** `board_build.*`-Konfiguration in `[env]` eingetragen (`flash_size=16MB`, `flash_mode=dio`, `flash_freq=40m`). `board = esp32dev` steht in **jedem Hardware-Env einzeln** (nicht in `[env]`), damit `[env:native]` es nicht erbt. Beim ersten Flash nach Änderung: `pio run -e miniV3 -t erase` empfohlen.

**Unit Tests (native, kein Hardware nötig):**
```bash
~/.platformio/penv/bin/pio test -e native            # Alle Tests (18 Fälle, ~4 s)
~/.platformio/penv/bin/pio test -e native -f test_pid_formula          # Nur PID-Tests
~/.platformio/penv/bin/pio test -e native -f test_temperature_max31855 # Nur MAX31855-Tests
```

Tests laufen auch in CI (`compile-test.yml`). Neue Tests kommen als Verzeichnis unter `test/test_<name>/test_<name>.cpp`. Test-Dateien inkludieren direkt aus `src/` — kein Arduino-Stub nötig, solange nur header-only Logik getestet wird.

**Test-Architektur (Phase 1 — Pure Logic):**
- `src/pitmaster/PidFormula.h` — header-only `pidComputeOutput()`, keine Arduino-Deps. Getestet in `test/test_pid_formula/`.
- `src/temperature/TemperatureMax31855Calc.h` — header-only `calcMax31855Temperature()`, reines Bit-Math. Getestet in `test/test_temperature_max31855/`.
- Phase 2 (ausstehend): HAL-Mocks für GPIO/PWM/DAC → `controlFan()`, `controlSSR()` → [Issue #206](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/206)

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

**Nextion TFT Build (CI only, Windows runner):**
- Script: `nextion/build.ps1` → lädt Nextion Editor, ruft SikuliX-Automatisierung auf
- Sikuli-Script: `nextion/nextion.sikuli/nextion.py` — steuert den Nextion Editor GUI
- Produces: 6 TFT-Dateien (3 Modelle × 2 Orientierungen) + `nextion_spiffs.bin`
- **Wichtig — Anchor+Offset statt Similarity:** Der Editor-Dialog verwendet Buttons mit gleicher Hintergrundfarbe (orange/grau/lila), bei denen Template-Matching grundsätzlich versagt (Hintergrund dominiert den Score). Alle Klicks laufen über zwei Anchor-Bilder + feste Pixel-Offsets:
  - `Please_Select_Model_heading.png` → Series-Tabs + Modell-Buttons
  - `Display_direction_title.png` → Richtungs-Buttons (90°/270°)
- Bei Nextion-Editor-Update: Anchor-Bilder aus CI `debug_screen*.png`-Artefakten neu messen und Offsets in `nextion.py` anpassen → [Issue #202](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/202)

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
- **WiFi-API in Event-Callbacks verboten (ESP-IDF 4.x):** `esp_wifi_set_mode()` / `WiFi.mode()` und `esp_wifi_set_ps()` dürfen **nicht** direkt in WiFi-Event-Callbacks aufgerufen werden — der WiFi-Task hält dabei interne Mutexe → Deadlock → `TG1WDT_SYS_RESET`. Stattdessen Pending-Flag setzen und in `Wlan::update()` (ConnectTask-Kontext) ausführen. Gleiches Muster wie `mdnsUpdatePending` (B21) und `wifiModePsPending` (B39). Auf ESP-IDF 3.x war das noch erlaubt — Regression durch Phase 1.
- **Recovery-Mode-Trigger aus async_tcp-Kontext:** `Wlan::setRecoveryPending()` (Flag) statt direktem `RecoveryMode::runFromApp()`-Aufruf. `runFromApp()` enthält `delay()` → würde async_tcp blockieren und Response-Flush verhindern. `Wlan::update()` (ConnectTask) verarbeitet das Flag sicher außerhalb von async_tcp. Gleiches Muster wie `mdnsUpdatePending` / `wlanSaveConfigPending`.
- **Restart nach async_tcp-Response:** `esp_timer` (One-Shot, ≥2s) statt `delay() + ESP.restart()` im Handler-Kontext. `delay()` in async_tcp-Callback blockiert den Task; `WiFi.disconnect()` unmittelbar nach `request->send()` killt die TCP-Verbindung vor dem Flush. Der Timer feuert in einem separaten Kontext, async_tcp kann die Response in der Zwischenzeit senden.
- **ESPAsyncWebServer Migrationshinweise:**
  - **3.x (mathieucarbou → ESP32Async):** `AsyncWebHandler::canHandle()` und `isRequestHandlerTrivial()` sind jetzt `const virtual` → alle Subklassen müssen `const override` verwenden. Body-Daten in `handleBody()` sind **nicht null-terminiert** → null-terminierte Kopie (`new uint8_t[len+1]`) vor `deserializeJson()` erstellen. `AsyncJsonResponse::getRoot()` gibt `JsonVariant` zurück → `.to<JsonObject>()` aufrufen.
  - **3.10.x (ESP32Async):** `WebRequestMethodComposite` ist jetzt eine eigene Klasse (kein `int32_t` mehr). Struct-Felder für HTTP-Methoden müssen `WebRequestMethodComposite` statt `int32_t` sein. Literal `0` als Initialisierer → `HTTP_UNKNOWN`. Vergleich `(request->method() & field) > 0u` → `field & request->method()` (operator& gibt `bool` zurück; Operandenreihenfolge: Composite links, `WebRequestMethod` rechts).

## Project Roadmap

**Ziele:** UI-Redesign, App-Store-Publishing (Android + iOS), Heap-Stabilität  
**Reihenfolge:** Phase 0 → Phase 1+2 (parallel) → Phase 3 → Phase 4a → Phase 4c → Phase 5

**Mobile App:** Flutter-App in Branch `android-build-test` (PR #180). Flutter WebView-Wrapper mit JS-Bridge für mDNS, Firebase FCM. Löst Cordova (`/mobile/`) ab.

### Phase 0 — Stabilisierung (laufend)

B1–B35 gefixt (2026-04-22–25) — Details im git-Log. B36–B38 offen (Low, siehe Known Issues). B39 gefixt (2026-04-27). B42–B51 gefixt (2026-04-28, P1-Review). B52–B60 offen (P2-Review, siehe Known Issues). SRAM-Optimierungen M2–M4 ausstehend.  
nanoV3: Stabil nach B39-Fix + `WiFi.persistent(false)`. BLE-Discovery-Fix implementiert, Hardware-Test ausstehend.

### Phase 1 — Kernel Upgrade ✅ (2026-04-20)

Alle 7 Hardware-Varianten auf `espressif32@^6.0.0` (arduino-esp32 2.x / ESP-IDF 4.4.x). miniV3 Hardware-Test weitgehend OK, nanoV3 stabil (B39 gefixt).  
Hardware-Test-Protokoll: [`docs/hardware-test-log.md`](docs/hardware-test-log.md)

### Phase 2 — UI-Redesign (ausstehend)

Vue 2 → Vue 3 (Neubau, Vue 2 seit 31.12.2023 EOL).

### Phase 3 — App Store (ausstehend)

Flutter-App fertigstellen, Cordova ablösen.

### Phase 4a — Library Pre-Upgrade ✅ (2026-04-24)

Alle regulären Libs aktualisiert (Versionen siehe Dependency Upgrade Guide). Ausstehend: ESPRandom (kein neues Release im Registry) und Custom Forks (kein regulärer Upgrade-Pfad).

### ~~Phase 4b — Kernel 3.x / ESP-IDF 5.x~~ ENTFÄLLT

Power Save auf Kernel 2.x gelöst via Custom-Framework `2.0.17-pm-enable`.

### Phase 4c — ArduinoJson 7 Migration ✅ (2026-04-21)

20 Dateien, ~404 API-Aufrufe migriert. Aktuelle v7-Pattern: siehe Key Conventions. Compile miniV3: RAM 27.4%, Flash 30.2%.

### Phase 5 — LVGL 9 + TFT-UI Neubau (ausstehend)

LVGL 7 → 9: gesamtes `src/display/tft/` neu. Sinnvoll mit Phase 2 gekoppelt. → [Issue #205](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/205)  
Scope: 3922 Zeilen, ~60% betroffen. Kritisch: `lvTheme.cpp` nutzt interne LVGL-APIs → Komplettneuschreibung. Externe Abhängigkeit: `tuniii/lv_lib_qrcode` auf v9-Kompatibilität prüfen.

---

## Known Issues (pending fixes)

### Bugs

| ID | Datei | Zeile | Severity | Problem |
|----|-------|-------|----------|---------|
| B1–B35 | — | — | — | **Alle gefixt** (2026-04-22–25) — Details im git-Log |
| B36 | `src/display/DisplayOled.cpp` + `DisplayOledLink.cpp` | `initDisplay()` | Low | → [Issue #190](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/190) — kDisplay NVS double-read at boot |
| B37 | `src/system/SystemBase.cpp` | `loadConfig()` Zeile 265 | Low | → [Issue #191](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/191) — SPIFFS.begin() result unchecked |
| B38 | `src/bluetooth/Bluetooth.cpp` | `Bluetooth::task()` | Low | → [Issue #192](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/192) — Invalid JSON on first boot cycle |
| B39 | `src/Wlan.cpp` | `onWifiConnect()` | High | **Gefixt** ✅ → [Issue #200](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/200) — TG1WDT_SYS_RESET nach WiFi-Connect auf nanoV3; `wifiModePsPending`-Flag + `WiFi.persistent(false)` |
| B41 | `src/pitmaster/Pitmaster.cpp`, `src/system/SystemBase.cpp` | `controlSSR()`, `initActuators()`, `update()` | High | **Gefixt** ✅ → [Issue #201](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/201) — SSR kein Output auf miniV3 nach ESP-IDF 4.4 Upgrade; LEDC 0,5 Hz ersetzt durch millis()-basiertes time-proportional control; `pitmasters.update()` aus `ONCE_PER_SECOND_CYCLE`-Gate herausgezogen (SystemBase.cpp) → 200 ms Tick; `SSR_PERIOD_MS` = 10 000 ms → 50 Stufen à 2% Auflösung |
| B42–B51 | — | — | — | **Alle gefixt** (2026-04-28) — Code Review P1 (#196) — Details unten |
| B52 | `src/pitmaster/Pitmaster.cpp` | `update()` → `checkPause()`, `checkOpenLid()`, `controlActuators()`, `pidCalc()` | High | → [Issue #217](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/217) — Null-Deref wenn `profile == NULL` (fehlender Guard in `update()`) |
| B53 | `src/pitmaster/Pitmaster.cpp`, `src/temperature/TemperatureGrp.cpp`, `src/pitmaster/PitmasterGrp.cpp` | `assignTemperature()`, `operator[]`, `loadConfig()` | High | → [Issue #218](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/218) — Null-Deref + negativer Index in `operator[](int)` |
| B54 | `src/connect/Connect.cpp` | `onReadyStateChange()` Zeile ~183 | High | → [Issue #219](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/219) — channelIndex ohne Bounds-Check → Heap-Overflow aus Remote-Daten |
| B55 | `src/bluetooth/Bluetooth.cpp`, `src/connect/Connect.cpp` | `loadConfig()`, `getDevices()` | High | → [Issue #220](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/220) — `strcpy` ohne Längencheck (6 Stellen) aus NVS + Netzwerk |
| B56 | `src/bluetooth/Bluetooth.cpp` | `getDevices()` vs. `isDeviceConnected()`/`getSensorValue()` | Medium | → [Issue #221](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/221) — Race Condition `bleDevices`: BLE-Task schreibt, MainTask liest ohne Mutex |
| B57 | `src/connect/Connect.cpp` | `onReadyStateChange()` vs. `isDeviceConnected()`/`getTemperatureValue()` | Medium | → [Issue #222](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/222) — Race Condition `connectDevices`: async_tcp-Callback schreibt, MainTask liest ohne Mutex |
| B58 | `src/bluetooth/Bluetooth.cpp` | `Bluetooth::task()` Zeile ~447 | Medium | → [Issue #223](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/223) — `vTaskDelete(NULL)` ohne `esp_task_wdt_delete(NULL)` im OTA-Exit-Pfad |
| B59 | `src/bluetooth/Bluetooth.cpp`, `src/connect/Connect.cpp`, `src/pitmaster/Pitmaster.cpp` | diverse | Low | → [Issue #224](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/224) — `Serial.print/printf` statt `Log.*` (Logging-Konvention) |
| B60 | `src/pitmaster/Pitmaster.h` | Zeile ~189 | Low | → [Issue #225](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/225) — `uint previousMillis` statt `uint32_t` |

### SRAM / Heap-Optimierungen

| ID | Dateien | Problem | Effekt | Wann |
|----|---------|---------|--------|------|
| M2 | `Mqtt.cpp:188,237,258,274` | Arduino `String` 1×/s → ~36.000 Heap-Ops/h | Heap-Fragmentierung | → [Issue #193](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/193) |
| M3 | `API.cpp:139–140` | `String sc[2]/vc[2]` lokal in `pitAry()` 1×/s | Gering, summiert sich | → [Issue #194](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/194) |
| M4 | `Connect.cpp:49`, `OtaUpdate.cpp:49` | Task-Stacks je 10.000 B — überdimensioniert | 7.000–9.000 B SRAM | → [Issue #195](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/195) — T1-Safety-Check zuerst |

**Vorgehen M4:** `highWaterMark`-Debug-Output aktiv in `main.cpp:89/124`, `Bluetooth.cpp:423`, `SystemBase.cpp` (SystemTask, Stack 6000 B). T1 = Messung unter Last, dann mit 512 B Puffer kürzen.

---

## Dependency Upgrade Guide

### Aktuelle Versionen (Phase 4a abgeschlossen 2026-04-24)

- **Framework:** `espressif32@^6.0.0` (arduino-esp32 2.x / ESP-IDF 4.4.x), Custom-Build `WLANThermo-nano/arduino-esp32@2.0.17-pm-enable` (`CONFIG_PM_ENABLE=y`)
- **Async-Stack:** `ESP32Async/AsyncTCP@^3.4.10` + `ESP32Async/ESPAsyncWebServer@^3.10.3` (`mathieucarbou/*` seit Jan 2025 archiviert)
- **JSON / MQTT:** `ArduinoJson 7.x`, `marvinroger/AsyncMqttClient@0.9.0`
- **Display / UI:** `TFT_eSPI@^2.5.34`, `esp8266-oled-ssd1306@#4.6.2`
- **Sonstiges:** `ArduinoLog@^1.1.1` (Breaking: `+int level` in Prefix-Callback), `OneButton@#2.6.2`

### Nicht upgraden – dedizierter Sprint nötig

| Dep | Aktuell | Ziel | Phase | Grund |
|-----|---------|------|-------|-------|
| **LVGL** | 7.11.0 | 9.x | **5** | Widgets umbenannt, Event-System geändert; gesamtes `src/display/tft/` neu |
| **Vue** | 2.6.11 | 3.x | **2** | Vue 2 EOL seit 31.12.2023; vollständiger Frontend-Neubau |

### Ausstehend

| Dep | Aktuell | Ziel | Hinweis |
|-----|---------|------|---------|
| **ESPRandom** | 1.4.1 | neueste | Kein neueres Release im Registry; Patch in `extra_script.py` bleibt |
| **axios** (Frontend) | 0.21.1 | 1.x | Besseres Error-Handling |
| **core-js** (Frontend) | ^3.6.5 | neueste | Polyfills, abwärtskompatibel |
| **sass / sass-loader** (Frontend) | 1.26 / 9.x | neueste 1.x / 10.x | Abwärtskompatibel |

**Custom Forks (kein Upgrade-Pfad):** `tuniii/BBQduino`, `tuniii/ITEADLIB_Arduino_Nextion`, `tuniii/ESPNexUpload`, `tuniii/lv_lib_qrcode`, `tuniii/Arduino-MedianFilter`, `borisneubert/Time`

---

## /wlanthermo-review Tracking

Skill-Aufruf: `/wlanthermo-review <Datei(en)>` — strukturierter Safety-Review nach Checkliste (Memory, FreeRTOS/Async, ArduinoJson v7, ESPAsyncWebServer 3.10.x, Input-Validation, ESP32-Fallstricke).

**Bereits reviewed (2026-04-25):** `WebHandler.cpp`, `API.h`, `API.cpp`, `Wlan.cpp`, `Wlan.h`, `RecoveryMode.cpp` — B23–B35 gefixt.  
**Bereits reviewed (2026-04-28):** `Notification.cpp/h`, `Mqtt.cpp/h`, `Cloud.cpp/h`, `OtaUpdate.cpp/h` — B42–B51 gefixt (#207–#216). Neue Konventionen: `SAFE_STRNCPY`-Makro in `Notification.cpp`, `saveConfigPending`-Flag in `Mqtt`+`Cloud`, `OtaProgressCalc.h` header-only + 7 Unit-Tests.  
**Bereits reviewed (2026-04-28):** `Pitmaster.cpp/h`, `PitmasterGrp.cpp/h`, `TemperatureBase.cpp/h`, `TemperatureGrp.cpp/h`, `TemperatureBle`, `TemperatureConnect`, `TemperatureMavRadio`, `TemperatureMax*`, `TemperatureMcp3208`, `Bluetooth.cpp/h`, `Connect.cpp/h` — B52–B60 gefunden (#217–#225). 4× High (Null-Deref, Buffer-Overflow, strcpy), 3× Medium (Race Conditions, WDT), 2× Low. `SAFE_STRNCPY` auch auf Bluetooth + Connect ausweiten; `portMUX_TYPE` für `bleDevices`/`connectDevices` ergänzen.

### Ausstehend — Priorisiert

- **P1 — Netzwerk-/Input-facing:** → [Issue #196](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/196) — **✅ Abgeschlossen** (2026-04-28)
- **P2 — Pitmaster / Sensor:** → [Issue #197](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/197) — **Review abgeschlossen** (2026-04-28), B52–B60 offen (#217–#225)
- **P3 — System / Settings:** → [Issue #198](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/198) — SystemBase, Settings, main.cpp, SerialCmd

**Priorität 4 (Display/Peripherie, geringes Netzwerk-Risiko):** `src/display/tft/` (nach Phase 5), `DisplayNextion`, `DisplayOled`, `Battery`, `Buzzer`, `PbGuard`, `SdCard`.

**Überspringen (kein Review nötig):** `bleFirmware*.h`, `*.gz.h`, `Constants.h`, `TaskConfig.h`, `Version.h`, `DbgPrint.h`, `MedianFilterFloat.h`, `System[Variant].cpp` (7×), `Temperature[Sensor].cpp` (6×), `API.h`.
