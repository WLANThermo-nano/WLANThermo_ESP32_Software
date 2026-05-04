# Code-Review — Findings-Übersicht

> **Automatisch gepflegt** — nicht manuell bearbeiten.
> Wird von `/review-next` (neue Tasks) und `/fix-next` (Status-Updates) aktualisiert.
> `/review-list` liest nur diese Datei.

**Stand:** 2026-05-04 | 66 Findings gesamt · 12 ✅ DONE · 54 ⏳ PENDING

---

### TASK-001 · WebHandler — Routing+Read (18 Findings)

| Sev | ID | Finding | Status |
|-----|----|---------|--------|
| 🔴 | CR-001-001 | `handleBody()` ignoriert Multi-Chunk-Reassembly für POST-Bodies | ✅ DONE |
| 🔴 | CR-001-002 | `new uint8_t[]` ohne NULL-Check und Size-Limit (OOM-Crash + DoS) | ✅ DONE |
| 🔴 | CR-001-003 | `gSystem->restart()` direkt aus async_tcp-Callback | ✅ DONE |
| 🟠 | CR-001-004 | Memory Leak in `handleWifiResult` bei `request == NULL`-Pfad | ✅ DONE |
| 🟠 | CR-001-005 | Telegram/Pushover-Tokens unverschlüsselt im `/getpush`-Response | ⏳ PENDING |
| 🟠 | CR-001-006 | `handleConfigReset` — NVS-Writes direkt aus async_tcp | ⏳ PENDING |
| 🟠 | CR-001-007 | `handleRotate` — `gDisplay->saveConfig()` direkt aus async_tcp | ⏳ PENDING |
| 🟠 | CR-001-008 | Schwache Version-String-Validierung in `handleUpdate` | ⏳ PENDING |
| 🟡 | CR-001-009 | Falsche HTTP-Status-Codes (500 statt 405/404) | ⏳ PENDING |
| 🟡 | CR-001-010 | JSON-Validation-Fehler liefert HTTP 200 + `false`-Body | ⏳ PENDING |
| 🟡 | CR-001-011 | `handleAdmin` erlaubt 0/1-Zeichen-Passwörter | ⏳ PENDING |
| 🟡 | CR-001-012 | Doppelte String-Allokation pro Request in `handleData` (1×/s) | ⏳ PENDING |
| 🟢 | CR-001-013 | Toter Code: `public_list[]` | ⏳ PENDING |
| 🟢 | CR-001-014 | Auskommentierter Code in `handleDeviceId` | ⏳ PENDING |
| 🟢 | CR-001-015 | `Serial.println(version)` statt ArduinoLog | ✅ DONE |
| 🟢 | CR-001-016 | `DPRINTLN` bei jedem `handleUpdateStatus` → Log-Spam während Update | ⏳ PENDING |
| 🟢 | CR-001-017 | Linearer Loop in `canHandle()` — 29 Vergleiche pro Request | ⏳ PENDING |
| 🟢 | CR-001-018 | Heap-Fragmentierung durch String-Konkatenation im WiFi-SSID-Loop | ⏳ PENDING |

---

### TASK-001b · WebHandler — Write/POST (19 Findings)

| Sev | ID | Finding | Status |
|-----|----|---------|--------|
| 🔴 | CR-001b-001 | `setServerAPI` — User kann beliebige OTA-URLs setzen (RCE-Vektor!) | ⏳ PENDING |
| 🔴 | CR-001b-002 | `setServerAPI` — `startUpdate()` direkt aus async_tcp | ⏳ PENDING |
| 🔴 | CR-001b-003 | `setBluetooth` — direkte NVS-Writes aus async_tcp | ⏳ PENDING |
| 🟠 | CR-001b-004 | Buffer-Overflow im hash-resolution-Pfad von `setPush` (`strcpy`) | ⏳ PENDING |
| 🟠 | CR-001b-005 | Cloud-URL-Hijacking via `setServerAPI` host/page (SSRF + Persistenz) | ⏳ PENDING |
| 🟠 | CR-001b-006 | `deserializeJson()` Return-Value in allen 10 `set*`-Handlern ignoriert | ✅ DONE |
| 🟠 | CR-001b-007 | `setNetwork`/`addNetwork` ohne SSID/Password-Längen-Validierung | ⏳ PENDING |
| 🟠 | CR-001b-008 | `setIoT` ohne Port-Validierung und ohne Custom-URL-Whitelist | ⏳ PENDING |
| 🟡 | CR-001b-009 | `printRequest()` leakt Credentials im Serial-Debug-Build | ✅ DONE |
| 🟡 | CR-001b-010 | `setPitmaster` Loop-Index `ii` vs. JSON-`id` Inkonsistenz | ⏳ PENDING |
| 🟡 | CR-001b-011 | `setBluetooth` Undefined Behavior durch fehlenden Bit-Shift-Upper-Bound | ✅ DONE |
| 🟡 | CR-001b-012 | `setServerAPI` `cloud.saveUrl()` direkt aus async_tcp (SPIFFS-Write) | ✅ DONE |
| 🟢 | CR-001b-013 | `Serial.println` statt ArduinoLog in `setServerAPI`/`setBluetooth` | ✅ DONE |
| 🟢 | CR-001b-014 | `setChannels` `byte _typ > -1` immer wahr (unsigned-Vergleich, toter Guard) | ⏳ PENDING |
| 🟢 | CR-001b-015 | `setDCTest` Dead variable `dc` | ⏳ PENDING |
| 🟢 | CR-001b-016 | `setDCTest` TODO-Kommentare und ungeklärte Frontend-Kopplung | ⏳ PENDING |
| 🟢 | CR-001b-017 | `setPitmaster` auskommentierter Code und TODOs | ⏳ PENDING |
| 🟢 | CR-001b-018 | `addNetwork` deklariert, aber nicht im Routing-Array (toter Code) | ⏳ PENDING |
| 🟢 | CR-001b-019 | `setSystem` magische 14-Zeichen-Hostname-Limit ohne Dokumentation | ⏳ PENDING |

---

### TASK-002 · API — JSON-Schema & Aggregation (13 Findings)

| Sev | ID | Finding | Status |
|-----|----|---------|--------|
| 🟠 | CR-002-001 | Plain-Text-Credentials in `iotObj()` + `notificationObj()` (MQTT/Telegram/Pushover) | ⏳ PENDING |
| 🟠 | CR-002-002 | `notificationObj()` `memcpy` ohne NULL-Check auf `testConfig` → NULL-Deref | ⏳ PENDING |
| 🟠 | CR-002-003 | `pitAry()` `getAssignedProfile()->id` ohne NULL-Check → Crash via MQTT 1×/s | ⏳ PENDING |
| 🟡 | CR-002-004 | `apiData()` Heap-Allokation 1×/s über MQTT/Cloud (M2-Cluster) | ⏳ PENDING |
| 🟡 | CR-002-005 | `pidAry()` Parameter `cc` wird ignoriert (irreführender API-Vertrag) | ⏳ PENDING |
| 🟡 | CR-002-006 | `channelAry()` Underflow auf `i = -1` bei `cc == 0` (latenter Bug) | ⏳ PENDING |
| 🟡 | CR-002-007 | Race zwischen async_tcp/ConnectTask und MainTask auf Sensor-Iteration | ⏳ PENDING |
| 🟢 | CR-002-008 | Hardcoded Cloud-URL `"cloud.wlanthermo.de/index.html"` im Code | ⏳ PENDING |
| 🟢 | CR-002-009 | `urlObj()` ist leere Funktion (toter Code) | ⏳ PENDING |
| 🟢 | CR-002-010 | Magic Numbers in `limit_float()` (0.05, 999, 10 etc.) | ⏳ PENDING |
| 🟢 | CR-002-011 | Auskommentierter Code an 3 Stellen in `API.cpp` | ⏳ PENDING |
| 🟢 | CR-002-012 | `pitAry()` switch ohne `default` → fehlendes `typ`-Feld bei neuen Types | ⏳ PENDING |
| 🟢 | CR-002-013 | Inkonsistenz `"v"` vs. `"V"` für hw_version-Felder in 3 Funktionen | ⏳ PENDING |

---

### TASK-003 · WLAN — WiFi/AsyncMQTT-Connect (16 Findings)

| Sev | ID | Finding | Status |
|-----|----|---------|--------|
| 🔴 | CR-003-001 | `addCredentials(force)`/`clearCredentials()` — NVS-Write direkt aus async_tcp | ✅ DONE |
| 🔴 | CR-003-002 | Race auf `String hostName`/`accessPointName` → Use-After-Free möglich | ⏳ PENDING |
| 🟠 | CR-003-003 | `addCredentials()` ruft `WiFi.disconnect()`+`WiFi.begin()` aus async_tcp (~300 ms) | ⏳ PENDING |
| 🟠 | CR-003-004 | `loadConfig()` Type-Confusion: `host`/`ap` ohne NULL-Check → Boot-Crash | ⏳ PENDING |
| 🟠 | CR-003-005 | `WIFI_ALL_CHANNEL_SCAN` wird nach `WiFi.begin()` gesetzt — wirkt nicht | ⏳ PENDING |
| 🟠 | CR-003-006 | Race auf `wifiState` zwischen async_tcp und ConnectTask | ⏳ PENDING |
| 🟡 | CR-003-007 | `stopAllRadio()` lässt Pending-Flags aktiv → MDNS/PS auf gestoppter Radio | ⏳ PENDING |
| 🟡 | CR-003-008 | Hardcoded AP-Password `"12345678"` (bekanntes Default, öffentliche Firmware) | ⏳ PENDING |
| 🟡 | CR-003-009 | Race auf `newWlanCredentials.ssid` in `onWifiConnect()` → False-Negative-Match | ⏳ PENDING |
| 🟡 | CR-003-010 | `esp_wifi_get_config()`/`esp_wifi_set_config()` Rückgabewerte ignoriert | ⏳ PENDING |
| 🟢 | CR-003-011 | 15+ `Serial.print/printf` statt ArduinoLog in `Wlan.cpp` | ✅ DONE |
| 🟢 | CR-003-012 | SSID im Klartext im Serial-Log (Privacy-Issue) | ⏳ PENDING |
| 🟢 | CR-003-013 | `WiFi.softAPConfig()`/`WiFi.softAP()` Return-Werte ignoriert | ⏳ PENDING |
| 🟢 | CR-003-014 | Inkonsistentes static/instance-Pattern in `Wlan`-Klasse | ⏳ PENDING |
| 🟢 | CR-003-015 | Auskommentierter Debug-Code (`Wlan.cpp:274, 362`) | ⏳ PENDING |
| 🟢 | CR-003-016 | `getMacAddress()`/`getHostName()` allokieren `String` pro Aufruf | ⏳ PENDING |

---

**Gesamt: 66 Findings · 11 ✅ DONE · 55 ⏳ PENDING**
