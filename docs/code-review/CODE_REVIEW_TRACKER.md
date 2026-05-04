# Code Review Tracker — WLANThermo ESP32 Software

**Workflow-Quelle:** [`ESP32_Code_Review_System.md`](../../../Downloads/ESP32_Code_Review_System.md)
**Task-Liste:** [`CODE_REVIEW_TASKS.md`](./CODE_REVIEW_TASKS.md)

---

## 📊 Status-Übersicht

| Metrik | Wert |
|--------|------|
| **Review-Start (neuer Workflow)** | 2026-04-29 |
| **Letztes Update** | 2026-05-04 (CR-003-005 gefixt) |
| **Reviewer** | Claude Opus 4.7 |
| **Tasks gesamt** | 34 |
| **Tasks abgeschlossen** | 4 / 34 (12 %) |
| **Tasks offen (NEU + RE-REVIEW)** | 27 |
| **Tasks deferred (Phase-5-Vorbehalt)** | 3 |
| **LOC im Review-Scope** | ~21.034 |
| **Offene Bugs (vor neuem Review)** | 3 (B36, B37, B38, alle Low) |
| **Offene SRAM-Optimierungen** | 3 (M2, M3, M4) |
| **Im neuen Review gefundene Issues** | 66 (Critical: 8, High: 17, Medium: 16, Low: 25) |
| **Im neuen Review gefixte Issues** | 15 |

> **Hinweis:** Die Counts oben beziehen sich nur auf den **neuen Opus-Review-Workflow** (ab 2026-04-29).
> Historische Findings aus den Sonnet-Reviews (B23–B63) sind unten in der Historie dokumentiert.

---

## 📋 Task-Status

**Legende:** 📝 NEU · 🔄 RE-REVIEW · 🔁 IN PROGRESS · ✅ DONE · ❌ BLOCKED · ⏭️ DEFERRED

### P1 — Netzwerk- / Input-facing

| ID | Modul | LOC | Prio | Status | Findings (C/H/M/L) | Report | Notiz |
|----|-------|-----|------|--------|---------------------|--------|-------|
| TASK-001 | WebHandler — Routing+Read | 634 | 🔴 CRITICAL | ✅ DONE | 3/5/4/6 | [reviews/REVIEW_TASK_001.md](./reviews/REVIEW_TASK_001.md) | Re-Review wertvoll: 18 Findings, 8 davon Sonnet übersehen |
| TASK-001b | WebHandler — Write/POST | 760 | 🔴 CRITICAL | ✅ DONE | 3/5/4/7 | [reviews/REVIEW_TASK_001b.md](./reviews/REVIEW_TASK_001b.md) | RCE-Vektor in `setServerAPI`; `setBluetooth` NVS direkt; durchgängig fehlender JSON-Error-Check |
| TASK-002 | API | 591 | 🟠 HIGH | ✅ DONE | 0/3/4/6 | [reviews/REVIEW_TASK_002.md](./reviews/REVIEW_TASK_002.md) | 13 Findings; Plain-Text-Credentials (Pattern wie CR-001-005), 2× NULL-Deref-Pfade |
| TASK-003 | WLAN | 562 | 🔴 CRITICAL | ✅ DONE | 2/4/4/6 | [reviews/REVIEW_TASK_003.md](./reviews/REVIEW_TASK_003.md) | B39 sauber gefixt; gleiches async_tcp-Pattern bei Setter-Pfaden offen (NVS + WiFi.begin), `String hostName` Heap-Race |
| TASK-004 | WServer | 298 | 🟠 HIGH | 📝 NEU | – | – | Abh. TASK-001 |
| TASK-005 | RecoveryMode | 407 | 🟠 HIGH | 🔄 RE-REVIEW | – | – | async_tcp-Pattern beachten |
| TASK-006 | Mqtt | 362 | 🟠 HIGH | 🔄 RE-REVIEW | – | – | M2 offen |
| TASK-007 | Cloud | 527 | 🟠 HIGH | 🔄 RE-REVIEW | – | – | – |
| TASK-008 | OtaUpdate | 396 | 🟠 HIGH | 🔄 RE-REVIEW | – | – | M4 offen |
| TASK-009 | Notification + Item | 620 | 🟠 HIGH | 🔄/📝 | – | – | SAFE_STRNCPY-Pattern |

### P2 — Bluetooth / BLE

| ID | Modul | LOC | Prio | Status | Findings (C/H/M/L) | Report | Notiz |
|----|-------|-----|------|--------|---------------------|--------|-------|
| TASK-010 | bluetooth/fwu | 897 | 🟠 HIGH | 📝 NEU | – | – | Sicherheits-kritisch |
| TASK-011 | Bluetooth | 761 | 🟠 HIGH | 🔄 RE-REVIEW | – | – | B38 offen, abh. TASK-010 |

### P3 — Sensor / Control

| ID | Modul | LOC | Prio | Status | Findings (C/H/M/L) | Report | Notiz |
|----|-------|-----|------|--------|---------------------|--------|-------|
| TASK-012 | Pitmaster | 1.311 | 🟠 HIGH | 🔄 RE-REVIEW | – | – | Sehr groß, B41-Pattern |
| TASK-013 | PitmasterGrp | 266 | 🟡 MEDIUM | 🔄 RE-REVIEW | – | – | Abh. TASK-012 |
| TASK-014 | TemperatureBase | 656 | 🟠 HIGH | 🔄 RE-REVIEW | – | – | – |
| TASK-015 | TemperatureGrp | 489 | 🟠 HIGH | 🔄 RE-REVIEW | – | – | Abh. TASK-014 |
| TASK-016 | Temp-Wireless | 325 | 🟡 MEDIUM | 🔄 RE-REVIEW | – | – | Abh. TASK-014 |
| TASK-017 | Temp-Hardware | 554 | 🟡 MEDIUM | 🔄 RE-REVIEW | – | – | Abh. TASK-014 |
| TASK-018 | connect/Connect | 330 | 🟡 MEDIUM | 🔄 RE-REVIEW | – | – | M4 offen |

### P4 — System / Settings

| ID | Modul | LOC | Prio | Status | Findings (C/H/M/L) | Report | Notiz |
|----|-------|-----|------|--------|---------------------|--------|-------|
| TASK-019 | main + System | 229 | 🟠 HIGH | 🔄/📝 | – | – | – |
| TASK-020 | SystemBase | 564 | 🟠 HIGH | 🔄 RE-REVIEW | – | – | B37 offen |
| TASK-021 | Settings | 227 | 🟠 HIGH | 🔄 RE-REVIEW | – | – | – |
| TASK-022 | SerialCmd | 351 | 🟡 MEDIUM | 🔄 RE-REVIEW | – | – | – |
| TASK-023 | DeviceId + LogRingBuffer | 251 | 🟡 MEDIUM | 📝 NEU | – | – | – |

### P5 — Peripherie

| ID | Modul | LOC | Prio | Status | Findings (C/H/M/L) | Report | Notiz |
|----|-------|-----|------|--------|---------------------|--------|-------|
| TASK-024 | Battery | 363 | 🟡 MEDIUM | 📝 NEU | – | – | – |
| TASK-025 | PbGuard | 213 | 🟡 MEDIUM | 📝 NEU | – | – | – |
| TASK-026 | Buzzer + SdCard | 213 | 🟢 LOW | 📝 NEU | – | – | – |

### P6 — Display Nextion / OLED

| ID | Modul | LOC | Prio | Status | Findings (C/H/M/L) | Report | Notiz |
|----|-------|-----|------|--------|---------------------|--------|-------|
| TASK-027 | DisplayBase + Dummy | 211 | 🟡 MEDIUM | 📝 NEU | – | – | – |
| TASK-028 | DisplayNextion | 1.241 | 🟡 MEDIUM | 📝 NEU | – | – | Sehr groß, abh. TASK-027 |
| TASK-029 | DisplayOled | 1.420 | 🟡 MEDIUM | 📝 NEU | – | – | B36 offen, abh. TASK-027 |
| TASK-030 | DisplayOledLink | 825 | 🟡 MEDIUM | 📝 NEU | – | – | B36 offen, abh. TASK-027 |

### P7 — TFT / LVGL (Phase-5-Vorbehalt)

| ID | Modul | LOC | Prio | Status | Findings (C/H/M/L) | Report | Notiz |
|----|-------|-----|------|--------|---------------------|--------|-------|
| TASK-031 | TFT Display+Theme+Screen | 2.065 | 🟢 LOW | ⏭️ DEFERRED | – | – | Wird durch Phase 5 ersetzt |
| TASK-032 | TFT Home/Pit/Temp | 1.460 | 🟢 LOW | ⏭️ DEFERRED | – | – | Wird durch Phase 5 ersetzt |
| TASK-033 | TFT Menu/Wifi/Update | 397 | 🟢 LOW | ⏭️ DEFERRED | – | – | Wird durch Phase 5 ersetzt |

---

## 🐛 Offene Issues (vor neuem Review)

> Diese Issues stammen aus den vorherigen Sonnet-Reviews und sind in CLAUDE.md dokumentiert.
> Sie sollen während des neuen Opus-Reviews mit beachtet werden.

### 🟢 Low — Bug-Backlog

- [ ] **B36** — kDisplay NVS double-read at boot
  - **Datei:** `src/display/DisplayOled.cpp`, `src/display/DisplayOledLink.cpp` (`initDisplay()`)
  - **GitHub:** [Issue #190](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/190)
  - **Betroffene Tasks:** TASK-029, TASK-030
  - **Status:** ⏳ PENDING

- [ ] **B37** — SPIFFS.begin() result unchecked
  - **Datei:** `src/system/SystemBase.cpp` (`loadConfig()` Zeile 265)
  - **GitHub:** [Issue #191](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/191)
  - **Betroffene Tasks:** TASK-020
  - **Status:** ⏳ PENDING

- [ ] **B38** — Invalid JSON on first boot cycle (BLE)
  - **Datei:** `src/bluetooth/Bluetooth.cpp` (`Bluetooth::task()`)
  - **GitHub:** [Issue #192](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/192)
  - **Betroffene Tasks:** TASK-011
  - **Status:** ⏳ PENDING

### 🟡 Medium — SRAM- / Heap-Optimierungen

- [ ] **M2** — Arduino String 1×/s in MQTT-Publish (~36.000 Heap-Ops/h)
  - **Datei:** `src/Mqtt.cpp:188,237,258,274`
  - **GitHub:** [Issue #193](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/193)
  - **Effekt:** Heap-Fragmentierung
  - **Betroffene Tasks:** TASK-006
  - **Status:** ⏳ PENDING

- [ ] **M3** — `String sc[2]/vc[2]` lokal in `pitAry()` 1×/s
  - **Datei:** `src/API.cpp:139–140`
  - **GitHub:** [Issue #194](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/194)
  - **Effekt:** Gering, summiert sich
  - **Betroffene Tasks:** TASK-002
  - **Status:** ⏳ PENDING

- [ ] **M4** — Task-Stacks überdimensioniert (10.000 B)
  - **Datei:** `src/connect/Connect.cpp:49`, `src/OtaUpdate.cpp:49`
  - **GitHub:** [Issue #195](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/195)
  - **Effekt:** 7.000–9.000 B SRAM verschwendet
  - **Betroffene Tasks:** TASK-008, TASK-018
  - **Status:** ⏳ PENDING (T1-Safety-Check zuerst — `highWaterMark`-Debug aktiv)

---

## ✅ Im neuen Review gefundene & gefixte Issues

*Wird nach jedem Task-Review befüllt.*

### 🔴 CRITICAL

- [x] **CR-001-001** — `handleBody()` ignoriert Multi-Chunk-Reassembly für POST-Bodies
  - **Datei:** `src/WebHandler.cpp:143–186`
  - **Status:** ✅ DONE — Commit `6892bad`, Issue #232 (2026-05-04)
  - **Aufwand:** M (~1–2 PT, alle 10 `set*`-Handler validieren)
- [x] **CR-001-002** — `new uint8_t[]` ohne NULL-Check und ohne Size-Limit (OOM-Crash + DoS)
  - **Datei:** `src/WebHandler.cpp:163–167`
  - **Status:** ✅ DONE — Commit `6892bad`, Issue #232 (2026-05-04)
  - **Aufwand:** S — zusammen mit CR-001-001 gefixt
- [x] **CR-001-003** — `gSystem->restart()` direkt aus async_tcp-Callback (CLAUDE.md-Anti-Pattern)
  - **Datei:** `src/WebHandler.cpp:362–368` + `src/system/SystemBase.cpp:285–291`
  - **Status:** ✅ DONE — Commit `6a8933a`, Issue #231 (2026-05-01)
  - **Aufwand:** S — `restartDeferred()` einführen, weitere Aufrufstellen in TASK-001b prüfen
- [ ] **CR-001b-001** — `setServerAPI` lässt User beliebige OTA-URLs setzen (RCE-Vektor!)
  - **Datei:** `src/WebHandler.cpp:1201–1215`
  - **Status:** ⏳ PENDING
  - **Aufwand:** M — Whitelist + Auth-Pflicht für Endpoint, evtl. Endpoint ganz abschaffen
- [ ] **CR-001b-002** — `setServerAPI` ruft `startUpdate()` direkt aus async_tcp
  - **Datei:** `src/WebHandler.cpp:1217–1220`
  - **Status:** ⏳ PENDING
  - **Aufwand:** S — Pending-Flag-Pattern in OtaUpdate
- [ ] **CR-001b-003** — `setBluetooth` macht direkte NVS-Writes aus async_tcp
  - **Datei:** `src/WebHandler.cpp:1326–1327`
  - **Status:** ⏳ PENDING
  - **Aufwand:** S — `tempConfigSavePending` + `bluetoothConfigSavePending`
- [x] **CR-003-001** — `addCredentials(force=true)` und `clearCredentials()` rufen `saveConfig()` direkt aus async_tcp (NVS-Write blockiert)
  - **Datei:** `src/Wlan.cpp:149–158, 192–196` (Aufrufer: `WebHandler.cpp:365, 732`)
  - **Status:** ✅ DONE — Commit `d1b2c47`, Issue #241 (2026-05-04)
  - **Aufwand:** S — `wlanSaveConfigPending` existiert bereits, nur Roll-out
- [x] **CR-003-002** — Race auf statische `String hostName`/`accessPointName` (Heap-Realloc concurrent zu `MDNS.begin(hostName.c_str())`) → Use-After-Free möglich
  - **Datei:** `src/Wlan.cpp:35–36, 82, 84, 130–131, 371, 432–449`
  - **Status:** ✅ DONE — Commit `1366239`, Issue #242 (2026-05-04)
  - **Aufwand:** M (1 PT) — Migration zu `char[]`-Buffer, mit Phase-2-Cleanup koppeln

### 🟠 HIGH

- [x] **CR-001-004** — Memory Leak in `handleWifiResult` bei `request == NULL`-Pfad (`WebHandler.cpp:222–278`) — ✅ DONE Commit `03c2e29`, Issue #233 (2026-05-04)
- [ ] **CR-001-005** — Telegram/Pushover-Tokens werden im Klartext zurückgegeben (`WebHandler.cpp:516, 522`)
- [ ] **CR-001-006** — `handleConfigReset` löst NVS-Writes direkt aus async_tcp aus (`WebHandler.cpp:296–311`)
- [ ] **CR-001-007** — `handleRotate` ruft `gDisplay->saveConfig()` direkt aus async_tcp (`WebHandler.cpp:382–399`)
- [ ] **CR-001-008** — Schwache Version-String-Validierung in `handleUpdate` (`WebHandler.cpp:444`)
- [ ] **CR-001b-004** — Buffer-Overflow im hash-resolution-Pfad von `setPush` (`WebHandler.cpp:931–936`)
- [ ] **CR-001b-005** — Cloud-URL-Hijacking via `setServerAPI` host/page (`WebHandler.cpp:1160–1175`)
- [x] **CR-001b-006** — `deserializeJson()`-Return-Value wird in allen 10 `set*`-Handlern ignoriert
  - **Datei:** `src/WebHandler.cpp:575, 627, 688, 717, 746, 806, 970, 1072, 1154, 1267, 1287`
  - **Status:** ✅ DONE — Commit `94e94aa`, Issue #234 (2026-05-04)
  - **Aufwand:** S
- [ ] **CR-001b-007** — `setNetwork`/`addNetwork` ohne SSID/Password-Längen-Validierung (`WebHandler.cpp:696–706, 725–735`)
- [ ] **CR-001b-008** — `setIoT` ohne Port-Validierung und keine Custom-URL-Whitelist (`WebHandler.cpp:760–798`)
- [x] **CR-003-003** — `addCredentials()` ruft `WiFi.disconnect()` + `WiFi.begin()` direkt aus async_tcp (~100–300 ms blockierend)
  - **Datei:** `src/Wlan.cpp:198–209` (Aufrufer: `WebHandler.cpp:703, 732`)
  - **Status:** ✅ DONE — Commit `74230ce`, Issue #243 (2026-05-04)
  - **Aufwand:** M (1 PT) — `newCredentialsPending`-Flag, Logik nach `update()` verlagern; mit CR-003-001 kombinieren
- [x] **CR-003-004** — `loadConfig()` Type-Confusion-Crash: `host`/`ap` ohne NULL-Check → String-Crash bei JSON-Type-Mismatch (`Wlan.cpp:81–85`)
  - **Datei:** `src/Wlan.cpp:81–93`
  - **Status:** ✅ DONE — Commit `1366239`, Issue #242 (2026-05-04) — mitgefixt durch CR-003-002 (`char[]`-Migration)
  - **Aufwand:** S
- [x] **CR-003-005** — `WIFI_ALL_CHANNEL_SCAN` wird **nach** `WiFi.begin()` gesetzt → wirkt nicht auf laufenden Verbindungsversuch (`Wlan.cpp:202–207, 341–346`)
  - **Status:** ✅ DONE — Commit `a1797e7`, Issue #244 (2026-05-04)
  - **Aufwand:** S
- [ ] **CR-003-006** — `wifiState` Race zwischen async_tcp (`addCredentials`/`setStopRequest`) und ConnectTask (`update()`); löst sich mit CR-003-003-Fix mit auf (`Wlan.cpp:201, 459`)
- [ ] **CR-002-001** — Plain-Text-Credentials in `iotObj()`+`notificationObj()` (MQTT-Pass, Cloud-Token, Telegram/Pushover/App-Tokens)
  - **Datei:** `src/API.cpp:215, 220, 287, 295–296, 312`
  - **Status:** ⏳ PENDING
  - **Aufwand:** M (~2 PT) — gleiche Maskierungs-Strategie wie CR-001-005, Frontend-Sync nötig; mit Settings-Cluster aus TASK-001b zusammenführen
- [ ] **CR-002-002** — `notificationObj()` `memcpy` ohne NULL-Check auf `notificationData.testConfig` → potenzieller NULL-Deref bei Test-Pfad
  - **Datei:** `src/API.cpp:255, 259, 263`
  - **Status:** ⏳ PENDING
  - **Aufwand:** S (~0,5 PT)
- [ ] **CR-002-003** — `pitAry()` `pm->getAssignedProfile()->id` ohne NULL-Check → Crash wenn Pitmaster ohne valides Profil (Bootphase, NVS-Migration); Trigger via MQTT 1×/s + Cloud-Send
  - **Datei:** `src/API.cpp:150`
  - **Status:** ⏳ PENDING
  - **Aufwand:** S (~0,25 PT)

### 🟡 MEDIUM

- [ ] **CR-001-009** — Falsche HTTP-Status-Codes (500 statt 405 / 404) (`WebHandler.cpp:137, 182`)
- [ ] **CR-001-010** — JSON-Validation-Fehler liefert HTTP 200 + `false`-Body (`WebHandler.cpp:169–176`)
- [ ] **CR-001-011** — `handleAdmin` erlaubt 0/1-Zeichen-Passwörter (`WebHandler.cpp:418`)
- [ ] **CR-001-012** — Doppelte String-Allokation pro Request in `handleSettings`/`handleData` (`WebHandler.cpp:204–220`)
- [x] **CR-001b-009** — `printRequest()` leakt Credentials im Serial-Debug-Build (8× set*-Handler)
  - **Datei:** `src/WebHandler.cpp:715, 748, 781, 845`
  - **Status:** ✅ DONE — Commit `207c30b`, Issue #236 (2026-05-04)
  - **Aufwand:** S
- [ ] **CR-001b-010** — `setPitmaster` Loop-Index `ii` vs. JSON-`id` Inkonsistenz (`WebHandler.cpp:975–1058`)
- [x] **CR-001b-011** — `setBluetooth` UB durch fehlenden Bit-Shift-Upper-Bound (`WebHandler.cpp:1308–1313`)
  - **Datei:** `src/WebHandler.cpp:1362–1367`
  - **Status:** ✅ DONE — Commit `fc0ece1`, Issue #237 (2026-05-04)
  - **Aufwand:** S
- [x] **CR-001b-012** — `setServerAPI` `cloud.saveUrl()` direkt aus async_tcp (`WebHandler.cpp:1174`)
  - **Datei:** `src/Cloud.cpp:152–174, Cloud.h:101`
  - **Status:** ✅ DONE — Commit `100a3f1`, Issue #238 (2026-05-04)
  - **Aufwand:** S
- [ ] **CR-003-007** — `stopAllRadio()` lässt `mdnsUpdatePending`/`wifiModePsPending` aktiv → MDNS/PS auf gestoppter Radio (`Wlan.cpp:360–367`)
- [ ] **CR-003-008** — Hardcoded AP-Password `"12345678"` (`Wlan.cpp:31, 66`) — bekanntes Default, mit Phase 3 koppeln
- [ ] **CR-003-009** — `onWifiConnect()` Race auf `newWlanCredentials.ssid` (concurrent mit `addCredentials` und `saveConfig` → False-Negative-Match) (`Wlan.cpp:386–400`)
- [ ] **CR-003-010** — `esp_wifi_get_config()`/`esp_wifi_set_config()` Rückgabewerte ignoriert (`Wlan.cpp:205, 207, 344, 346`)
- [ ] **CR-002-004** — `apiData()` Heap-Allokation 1×/s über MQTT/Cloud (M2-Cluster, JsonDocument + String + Inner-String-Konvertierungen) (`src/API.cpp:489–567`)
- [ ] **CR-002-005** — `pidAry()` Parameter `cc` wird ignoriert (Loop iteriert über `getPitmasterProfileCount()`) (`src/API.cpp:182–204`)
- [ ] **CR-002-006** — `channelAry()` Underflow auf `i = -1` bei `cc == 0` und `count() > 0` (latent, kein realer Aufrufer) (`src/API.cpp:99–103`)
- [ ] **CR-002-007** — Race zwischen async_tcp/ConnectTask und MainTask auf `temperatures`/`pitmasters`-Iteration (NULL-Check auf operator[]-Ebene, aber nested Pointer ungeschützt) (`src/API.cpp:103–119, 142–177`)

### 🟢 LOW

- [ ] **CR-001-013** — Toter Code: `public_list[]` (`WebHandler.cpp:57–59`)
- [ ] **CR-001-014** — Auskommentierter Code in `handleDeviceId` (`WebHandler.cpp:493`)
- [x] **CR-001-015** — `Serial.println(version)` statt ArduinoLog (`WebHandler.cpp:443`)
  - **Status:** ✅ DONE — Commit `635e807`, Issue #235 (2026-05-04)
- [ ] **CR-001-016** — `DPRINTLN` bei jedem `handleUpdateStatus` → Log-Spam (`WebHandler.cpp:328`)
- [ ] **CR-001-017** — Linearer Loop in `canHandle()` — akzeptabel, nur dokumentiert (`WebHandler.cpp:188–202`)
- [ ] **CR-001-018** — Heap-Fragmentierung durch String-Konkatenation in WiFi-Loop (`WebHandler.cpp:254, 257`)
- [x] **CR-001b-013** — Mehrere `Serial.println` statt ArduinoLog in `setServerAPI`/`setBluetooth`
  - **Datei:** `src/WebHandler.cpp:1208, 1227, 1252, 1260, 1285, 1286, 1299, 1300, 1358`
  - **Status:** ✅ DONE — Commit `eadef8c`, Issue #239 (2026-05-04)
  - **Aufwand:** S
- [ ] **CR-001b-014** — `setChannels` `byte _typ > -1` immer wahr/falsch (unsigned-Vergleich) (`WebHandler.cpp:653`)
- [ ] **CR-001b-015** — `setDCTest` Dead variable `dc` (`WebHandler.cpp:1273`)
- [ ] **CR-001b-016** — `setDCTest` TODO-Kommentare und Frontend-Kopplung (`WebHandler.cpp:1274–1276`)
- [ ] **CR-001b-017** — `setPitmaster` auskommentierte Code/TODO (`WebHandler.cpp:1007–1008, 1022`)
- [ ] **CR-001b-018** — `addNetwork` deklariert aber nicht im Routing-Array (toter Code)
- [ ] **CR-001b-019** — `setSystem` magische 14-Zeichen-Hostname-Limit ohne Doku (`WebHandler.cpp:597, 604`)
- [x] **CR-003-011** — 15+ `Serial.print/printf` statt `ArduinoLog` in `Wlan.cpp` (`Wlan.cpp:69, 70, 97, 103, 142, 184, 189, 347, 373, 388, 389, 404, 410`)
  - **Status:** ✅ DONE — Commit `7d85dab`, Issue #240 (2026-05-04)
- [ ] **CR-003-012** — SSID im Klartext im `Serial.printf` (Privacy-Issue) (`Wlan.cpp:103, 142, 189, 347, 388`)
- [ ] **CR-003-013** — `WiFi.softAPConfig()`/`WiFi.softAP()` Return-Werte ignoriert in `init()` (`Wlan.cpp:65–66`)
- [ ] **CR-003-014** — Inkonsistentes static/instance-Pattern in `Wlan`-Klasse (`Wlan.h:74–96`)
- [ ] **CR-003-015** — Auskommentierter Debug-Code (`Wlan.cpp:274, 362`)
- [ ] **CR-003-016** — `getMacAddress()`/`getHostName()` allokieren `String` pro Aufruf (Heap-Churn) (`Wlan.cpp:413–420, 427–430`)
- [ ] **CR-002-008** — Hardcoded Cloud-URL `"cloud.wlanthermo.de/index.html"` (`src/API.cpp:222`)
- [ ] **CR-002-009** — `urlObj()` ist eine leere Funktion (toter Code), wird in `apiData(APIUPDATE)` aufgerufen → leeres `"url"`-Object (`src/API.cpp:353–364`)
- [ ] **CR-002-010** — Magic Numbers in `limit_float()` (`0.05`, `10`, `0.005`, `100`, `999` statt `INACTIVEVALUE`) (`src/API.cpp:572–591`)
- [ ] **CR-002-011** — Auskommentierter Code an mehreren Stellen (`src/API.cpp:313, 357–363, 378`)
- [ ] **CR-002-012** — `pitAry()` switch ohne `default` für `getType()`/`getTypeLast()` → fehlendes `typ`-Feld bei zukünftigen Pitmaster-Types (`src/API.cpp:153–172`)
- [ ] **CR-002-013** — Inkonsistenz `String("v")` (klein) vs. `String("V")` (groß) für hw_version-Felder (`src/API.cpp:55, 90, 404`)

---

## 📅 Sprint-Planung

*Vorlagen — werden vor Sprint-Start mit konkreten Issue-IDs befüllt.*

### Sprint 1: P1 Netzwerk (Kritisch) — TBD

**Ziel:** TASK-001, 001b, 003 abschließen (CRITICAL-Module mit Re-Review)

- [ ] TASK-001: WebHandler — Routing+Read
- [ ] TASK-001b: WebHandler — Write/POST
- [ ] TASK-003: WLAN
- **Geschätzter Aufwand:** TBD
- **Owner:** —

### Sprint 2: P1 Netzwerk (rest) — TBD

**Ziel:** Restliche P1-Tasks (002, 004–009)

- [ ] TASK-002: API
- [ ] TASK-004: WServer
- [ ] TASK-005: RecoveryMode
- [ ] TASK-006: Mqtt
- [ ] TASK-007: Cloud
- [ ] TASK-008: OtaUpdate
- [ ] TASK-009: Notification + Item

### Sprint 3: P2 Bluetooth — TBD

- [ ] TASK-010: bluetooth/fwu
- [ ] TASK-011: Bluetooth

### Sprint 4: P3 Sensor/Control — TBD

- [ ] TASK-012, 013, 014, 015, 016, 017, 018

### Sprint 5: P4 System — TBD

- [ ] TASK-019, 020, 021, 022, 023

### Sprint 6: P5 Peripherie — TBD

- [ ] TASK-024, 025, 026

### Sprint 7: P6 Display — TBD

- [ ] TASK-027, 028, 029, 030

### Sprint 8: P7 TFT (optional, vor Phase 5) — TBD

- [ ] TASK-031, 032, 033

---

## 📝 Notizen & Entscheidungen

### 2026-04-29: Workflow-Wechsel — Sonnet → Opus + neues Master-System

- **Kontext:** Bisherige Reviews (Initial + P1–P3) wurden mit dem `/wlanthermo-review`-Skill und Claude Sonnet durchgeführt. Findings sind in CLAUDE.md dokumentiert, Bugs als GitHub-Issues #207–#225 (P1+P2) und #228–#230 (P3) angelegt und gefixt.
- **Entscheidung:** Neuer Master-Workflow nach `ESP32_Code_Review_System.md` mit Claude Opus 4.7. Bereits gereviewte Module werden **erneut** geprüft (Re-Review), da Opus möglicherweise zusätzliche/andere Findings liefert.
- **Alternativen:** Nur die noch nicht gereviewten Module nehmen (verworfen — Re-Review-Mehrwert wahrscheinlich).
- **Auswirkungen:**
  - 33 Tasks definiert, ~21.034 LOC im Scope
  - `/wlanthermo-review`-Skill bleibt als Detail-Tool nutzbar, aber CODE_REVIEW_TASKS+TRACKER ist die Master-Quelle
  - Findings landen vorerst nur in lokalen MD-Reports — GitHub-Issues werden nachgepflegt

### 2026-04-29: Scope-Entscheidung — TFT/LVGL deferred

- **Kontext:** `src/display/tft/` (3.922 LOC) wird in Phase 5 komplett auf LVGL 9 neu geschrieben (Issue #205).
- **Entscheidung:** TFT-Tasks (031–033) als 🟢 LOW + ⏭️ DEFERRED markiert. Reviews dort nur für akute Bugs / Anti-Pattern-Dokumentation, nicht für Code-Qualität.

### 2026-04-29: TASK-001 gesplittet (Routing+Read / Write)

- **Kontext:** TASK-001 (WebHandler) hatte 1.414 LOC — über der 800-LOC-Grenze.
- **Entscheidung:** Aufgeteilt in zwei Tasks entlang der natürlichen Code-Trennung:
  - **TASK-001** (Z. 1–548, ~634 LOC inkl. Header) — Routing-Infrastruktur + alle 17 Read/Action-`handle*`-Funktionen
  - **TASK-001b** (Z. 569–1328, ~760 LOC) — Alle 10 Write-`set*`-Funktionen mit JSON-Body-Parsing
- **Begründung:** Die `set*`-Funktionen sind input-validation-kritisch (Buffer, Range, Credential), die `handle*`-Funktionen primär Output/Aggregation. Saubere Security-Trennung.
- **Tasks-Total:** 33 → 34

### 2026-04-29: Scope-Entscheidung — webui/ ausgeschlossen

- **Kontext:** Vue-2-Frontend in `webui/src/` wird in Phase 2 auf Vue 3 neu gebaut (EOL-Migration).
- **Entscheidung:** `webui/` aus Review-Scope ausgenommen — wird zu späterem Zeitpunkt separat behandelt.

---

## 📈 Metriken & Trends

### Code-Qualität

- **Initial Score (TASK-001 als Baseline):** **6/10** (Code-Qualität), **7/10** (Wartbarkeit)
- **Aktuell:** Wie Initial (1 Task gereviewt)
- **Ziel:** ≥ 8/10

### Issue-Trend

*Wird wöchentlich aktualisiert.*

```
Week  | Critical | High | Medium | Low | Total
------|----------|------|--------|-----|-------
KW-1  |    -     |  -   |   -    |  -  |   -
```

### Fix-Rate

- **Durchschnittliche Zeit pro Fix:** TBD
- **Fixes pro Sprint:** TBD

---

## 🗂️ Historie — Reviews vor 2026-04-29 (Sonnet)

> Diese Reviews fanden vor dem Workflow-Wechsel statt und sind hier nur als Referenz dokumentiert.
> Details siehe git-Log und CLAUDE.md.

### Initial-Review — 2026-04-25

- **Module:** WebHandler, API, Wlan, RecoveryMode
- **Findings gefixt:** B23–B35 (13 Bugs, im git-Log dokumentiert)
- **Status:** ✅ Abgeschlossen
- **Re-Review im neuen Workflow:** TASK-001, 002, 003, 005

### P1 — 2026-04-28 (Issue [#196](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/196))

- **Module:** Notification, Mqtt, Cloud, OtaUpdate
- **Findings gefixt:** B42–B51 (10 Issues, GitHub #207–#216)
- **Konvention eingeführt:** `SAFE_STRNCPY`-Makro, `saveConfigPending`-Flag-Pattern, `OtaProgressCalc.h` header-only mit Unit-Tests
- **Status:** ✅ Abgeschlossen
- **Re-Review im neuen Workflow:** TASK-006, 007, 008, 009

### P2 — 2026-04-28 (Issue [#197](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/197))

- **Module:** Pitmaster*, Temperature*, Bluetooth, Connect
- **Findings gefixt:** B52–B60 (9 Issues, GitHub #217–#225)
  - 4× High (Null-Deref, Buffer-Overflow, strcpy)
  - 3× Medium (Race Conditions, WDT)
  - 2× Low
- **Status:** ✅ Abgeschlossen
- **Re-Review im neuen Workflow:** TASK-011, 012, 013, 014, 015, 016, 017, 018

### P3 — 2026-04-28 (Issue [#198](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/198))

- **Module:** SystemBase, Settings, main, SerialCmd
- **Findings gefixt:** B61–B63 (3 Issues, GitHub #228–#230)
  - 0× Critical, 0× High
  - 1× Medium (language nullptr)
  - 2× Low (dead code, Serial.printf)
- **Status:** ✅ Abgeschlossen
- **Re-Review im neuen Workflow:** TASK-019, 020, 021, 022

---

## 🎯 Nächste Schritte

1. [x] `reviews/`-Verzeichnis anlegen (für Report-Files)
2. [x] **TASK-001 abgeschlossen** — 18 Findings, Code-Qualität 6/10, Wartbarkeit 7/10
3. [x] **TASK-001b abgeschlossen** — 19 Findings, Code-Qualität 5/10, Wartbarkeit 6/10
4. [x] **TASK-003 abgeschlossen** — 16 Findings, Code-Qualität 7/10, Wartbarkeit 6/10; B39-Fix sauber, aber gleiches Pattern an Setter-Pfaden offen
5. [ ] **Critical-Cluster fixen** vor weiteren Reviews — empfohlene Reihenfolge:
   - CR-001-001 + CR-001-002 (gemeinsamer `handleBody()`-Pfad)
   - CR-001b-001 + 002 + 005 (`setServerAPI` — RCE-Cluster)
   - CR-001-003 + CR-001b-003 + **CR-003-001 + CR-003-003 + CR-003-006** (alle async_tcp NVS/WiFi.begin/Restart-Anti-Patterns als ein Sammel-PR „WLAN/WebHandler async_tcp safety")
   - **CR-003-002** (`String hostName` Heap-Race) — eigenes PR, mit Phase-2-Vorbereitung koppeln
6. [x] **TASK-002 abgeschlossen** — 13 Findings (0/3/4/6), Code-Qualität 7/10, Wartbarkeit 7/10; Plain-Text-Credentials-Pattern bestätigt (CR-002-001), 2× NULL-Deref-Pfade (CR-002-002 testConfig, CR-002-003 Pitmaster-Profile)
7. [ ] **TASK-004 oder TASK-005 starten** als nächster Task — beide sind 🟠 HIGH und eligible:
   - TASK-004 (WServer, 298 LOC, 📝 NEU) — abh. TASK-001 ✅, gehört in den WebHandler-Cluster
   - TASK-005 (RecoveryMode, 407 LOC, 🔄 RE-REVIEW) — abh. TASK-003 ✅, async_tcp-Pattern beachten
   Empfehlung gemäß CODE_REVIEW_TASKS.md Phase A: TASK-004 zuerst (kleiner, gleicher Cluster wie 001)
8. [ ] **CR-002-001** sollte mit dem Settings-Cluster aus TASK-001b in einem PR landen (gleiche Maskierungs-Strategie für alle Settings-Endpoints — Backend-Maskierung zuerst, dann Frontend-Sync)
9. [ ] Bei kritischen Findings: GitHub-Issues spiegeln (gemäß Memory `feedback_github_issues.md`) — sammeln, nicht einzeln

---

## 📚 Referenzen

- **Workflow:** [`ESP32_Code_Review_System.md`](../../../Downloads/ESP32_Code_Review_System.md)
- **Task-Liste:** [`CODE_REVIEW_TASKS.md`](./CODE_REVIEW_TASKS.md)
- **Reports:** `docs/code-review/reviews/REVIEW_TASK_XXX.md` *(noch leer)*
- **Projekt-Kontext:** [`CLAUDE.md`](../../CLAUDE.md)
- **Hardware-Test-Log:** [`docs/hardware-test-log.md`](../hardware-test-log.md)
- **GitHub-Repo:** [WLANThermo-nano/WLANThermo_ESP32_Software](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software)
