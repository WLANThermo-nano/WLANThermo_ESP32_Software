# Review Results — TASK-001b

**Modul:** WebHandler — POST/Write-Endpoints (JSON-Body)
**Review-Datum:** 2026-04-30
**Reviewer:** Claude Opus 4.7
**Dateien:** `WebHandler.cpp` Z. 569–1328 + relevante `.h`-Strukturen
**LOC im Scope:** ~760
**Vorheriger Review:** Sonnet 2026-04-25 (Initial-Sammelnummer B23–B35) · **Findings damals:** 13 (Modul-übergreifend)
**Vorausgesetzte Findings aus TASK-001:** **CR-001-001** (Multi-Chunk-Body) und **CR-001-002** (NULL-Check/Size-Limit) **sind noch nicht gefixt** — neue Findings hier sind set*-spezifisch und gehen davon aus, dass der Body korrekt vorliegt. Allgemeine Aussagen zu malformed-Bodies sind in TASK-001 abgehandelt.

---

## Executive Summary

Die zehn `set*`-Handler verwenden in der Mehrzahl saubere Patterns: ArduinoJson-v7-Korrektheit, `xxxConfigSavePending`-Flags statt direkter NVS-Writes (5 von 10 Handler), `strncpy` mit Null-Term in `setIoT`/`setPush`. Allerdings finden sich **gravierende Sicherheits-Issues**, die der Sonnet-Review nicht erfasst hat: in `setServerAPI` kann ein User mit Netzwerk-Zugang **beliebige Update-URLs setzen und sofort flashen** (RCE-Vektor), in `setBluetooth` werden NVS-Writes wieder direkt aus async_tcp gemacht, ein **Buffer-Overflow** im hash-resolution-Pfad von `setPush`, und durchgängig **fehlende `deserializeJson()`-Error-Checks** (10/10 Handler).

**Gesamtbewertung:** 🔴 **3 CRITICAL · 5 HIGH · 4 MEDIUM · 7 LOW** — schlechter als TASK-001, weil hier die User-Input-Pfade liegen und Validation-Gaps direkter ausnutzbar sind. Re-Review-Mehrwert: **klar gegeben**, insbesondere die `setServerAPI`-Issues sind hochrelevant.

---

## Findings

### 🔴 CRITICAL Issues

#### CR-001b-001: `setServerAPI` — User kann beliebige OTA-URLs setzen (RCE-Vektor)

- **Datei:** `src/WebHandler.cpp:1201–1215`
- **Severity:** 🔴 CRITICAL
- **Kategorie:** Security
- **Problem:**
  ```cpp
  if (_update.containsKey(gDisplay->getUpdateName())) {
    JsonObject _fw = _update[gDisplay->getUpdateName()].as<JsonObject>();
    if (_fw.containsKey("url"))
      gSystem->otaUpdate.setDisplayUrl(_fw["url"].as<const char*>());  // Z. 1205
  }
  if (_update.containsKey("firmware")) {
    JsonObject _fw = _update["firmware"].as<JsonObject>();
    if (_fw.containsKey("url"))
      gSystem->otaUpdate.setFirmwareUrl(_fw["url"].as<const char*>()); // Z. 1213
  }
  ```
  Der `/setapi`-Endpoint nimmt URLs für Firmware- und Display-Updates **direkt aus dem Request-Body** ohne irgendeine Validierung — keine HTTPS-Pflicht, keine Domain-Whitelist, keine Signaturprüfung.

- **Impact:**
  Ein Angreifer mit Netzwerk-Zugang (lokales WLAN) kann via einen einzigen POST-Request:
  1. eine Firmware-URL auf einen Angreifer-Server setzen,
  2. `_update["force"]` mitschicken → Z. 1219 `gSystem->otaUpdate.startUpdate()` lädt **sofort** die Malware-Firmware,
  3. das Gerät flasht und rebootet in eine kompromittierte Firmware.
  Auf dem Gerät liegen WiFi-Passwords, MQTT-Credentials, Pushover/Telegram-Bot-Tokens, App-Tokens — alle unter Angreifer-Kontrolle.
  Da das WebUI typischerweise im Heim-WLAN ohne TLS / ohne Default-Auth läuft, ist die Hürde minimal.

- **Root Cause:**
  Der Endpoint wurde gebaut, damit das **WebUI** die vom offiziellen Update-Server gelieferten URLs ans Gerät durchreicht. Es wird angenommen, dass nur das offizielle WebUI mit dem Endpoint redet — keine Defense-in-Depth.

- **Lösung:**
  Entweder den Endpoint **abschaffen** (Update-URLs sollten auf dem Gerät direkt vom Update-Server abgerufen werden, das WebUI braucht nur den Trigger) oder mit Defensive-in-Depth absichern:
  ```cpp
  static bool isAllowedUpdateUrl(const char *url) {
    if (strncmp(url, "https://", 8) != 0) return false;
    static const char *allowedHosts[] = {
      "github.com",
      "objects.githubusercontent.com",
      "wlanthermo.de",
    };
    // Host extrahieren und gegen Whitelist prüfen
    // ...
  }
  if (_fw.containsKey("url")) {
    const char *url = _fw["url"].as<const char*>();
    if (!isAllowedUpdateUrl(url)) {
      Log.warning("Rejected update URL: %s" CR, url);
      return 0;
    }
    gSystem->otaUpdate.setFirmwareUrl(url);
  }
  ```
  Ergänzend: die `force`-Logik (Z. 1217–1220) sollte nur durch authentifizierte Requests triggerbar sein — `WServer::requireAuth()` für `/setapi` aktivieren.

- **Geschätzter Aufwand:** M (1 PT) — abhängig davon, ob der Endpoint tatsächlich vom WebUI gebraucht wird. Erst klären, ob er entfernt werden kann.
- **Cross-Reference:** CR-001-008 (TASK-001) — gleicher Validation-Gap bei `handleUpdate` Z. 444. Beide Stellen zusammen fixen.

---

#### CR-001b-002: `setServerAPI` — `startUpdate()` direkt aus async_tcp

- **Datei:** `src/WebHandler.cpp:1217–1220`
- **Severity:** 🔴 CRITICAL
- **Kategorie:** Performance, Logic
- **Problem:**
  ```cpp
  if (_update.containsKey("force")) {
    gSystem->otaUpdate.startUpdate();
  }
  ```
  `startUpdate()` ist eine schwergewichtige Operation: HTTPS-Download (mehrere MB), Flash-Programmierung (~10–30 s), Reboot. Hier direkt aus dem async_tcp-Body-Handler aufgerufen.
  Anti-Pattern wie CLAUDE.md beschreibt: "Restart nach async_tcp-Response: `esp_timer` (One-Shot, ≥2s) statt `delay() + ESP.restart()` im Handler-Kontext."

- **Impact:**
  - async_tcp-Task blockiert für die gesamte Update-Dauer → keine anderen Web-Requests möglich, ggf. WiFi-Disconnect, Watchdog-Reset.
  - Response wird vermutlich gar nicht mehr gesendet, weil der Download startet bevor die handleBody-Funktion zurückkehrt.
  - Reboot mitten im async_tcp-Callback → unsauberer Shutdown, evtl. NVS-Korruption.

- **Lösung:**
  Pending-Flag-Pattern wie `recoveryPending`:
  ```cpp
  // In OtaUpdate:
  void OtaUpdate::triggerStartPending() { startUpdatePending = true; }
  // In OtaUpdate::update() (MainTask oder ConnectTask):
  if (startUpdatePending) { startUpdatePending = false; startUpdate(); }
  ```
  Dann `gSystem->otaUpdate.triggerStartPending();` statt direkt `startUpdate()`.

- **Geschätzter Aufwand:** S (Pattern existiert bereits)
- **Cross-Reference:** ähnlich zu CR-001-003 (TASK-001) — gleiche Klasse von Bugs.

---

#### CR-001b-003: `setBluetooth` — direkte NVS-Writes aus async_tcp

- **Datei:** `src/WebHandler.cpp:1326–1327`
- **Severity:** 🔴 CRITICAL
- **Kategorie:** Performance
- **Problem:**
  ```cpp
  gSystem->temperatures.saveConfig();
  gSystem->bluetooth->saveConfig();
  return 1;
  ```
  Beide `saveConfig()`-Aufrufe sind blockierende NVS-Writes (typ. 5–50 ms je nach Größe). Direkt im async_tcp-Body-Handler.
  Andere `set*`-Handler (setSystem, setChannels, setPush, setPitmaster, setPID) verwenden korrekt `xxxConfigSavePending = true` — hier wurde es vergessen.

- **Impact:**
  - async_tcp-Task blockiert, andere Requests stocken.
  - Bei Frontend-Polling (`/data` alle 2 s) sichtbarer UI-Stall.
  - Inkonsistenz zur P1-Konvention (`saveConfigPending`-Pattern).

- **Lösung:**
  Pending-Flags einführen (analog zu den anderen Subsystemen):
  ```cpp
  gSystem->tempConfigSavePending = true;
  gSystem->bluetoothConfigSavePending = true;  // ggf. neu im SystemBase einführen
  ```

- **Geschätzter Aufwand:** S
- **Cross-Reference:** CR-001-006/007 (TASK-001) — gleiche Klasse von Bugs.

---

### 🟠 HIGH Priority Issues

#### CR-001b-004: `setPush` — Buffer-Overflow im hash-resolution-Pfad

- **Datei:** `src/WebHandler.cpp:931–936`
- **Severity:** 🟠 HIGH
- **Kategorie:** Security, Logic
- **Problem:**
  ```cpp
  String token;
  // ...
  else if (hasHashedToken) {
    token = gSystem->notification.getDeviceTokenFromHash(_device["token_sha256"].as<const char*>());  // Z. 933
  }
  strcpy(app.devices[deviceIndex].token, token.c_str());  // Z. 936
  ```
  Im hash-resolution-Pfad wird der von `getDeviceTokenFromHash()` zurückgegebene String **ohne Length-Check** mit `strcpy` in `app.devices[].token` (`char[255]`) kopiert.

  Direkt darüber (Z. 906–912) **gibt es** einen Length-Check für den Klartext-Token-Pfad — im Hash-Pfad fehlt er.

- **Impact:**
  - Bei einem zurückgegebenen String ≥ 255 Zeichen → Buffer-Overflow → Stack/Heap-Korruption.
  - Praktisch: `getDeviceTokenFromHash()` returnet typischerweise normale Push-Tokens (~64–152 Zeichen), aber wenn die Funktion bei Cache-Miss einen Default-/Error-String returnet, der lang sein kann, oder bei zukünftigen Token-Format-Änderungen, ist das ein latentes Risiko.

- **Lösung:**
  ```cpp
  if (hasHashedToken) {
    token = gSystem->notification.getDeviceTokenFromHash(_device["token_sha256"].as<const char*>());
    if (token.length() >= sizeof(PushAppDeviceType::token)) continue;
  }
  strncpy(app.devices[deviceIndex].token, token.c_str(), sizeof(app.devices[deviceIndex].token) - 1);
  app.devices[deviceIndex].token[sizeof(app.devices[deviceIndex].token) - 1] = '\0';
  ```
  Generell: alle vier `strcpy` in diesem Block (Z. 923, 924, 936, 945) durch `strncpy` mit explizitem Null-Term ersetzen — Defense in Depth.

- **Geschätzter Aufwand:** S

---

#### CR-001b-005: `setServerAPI` — User-controlled Cloud-URLs (SSRF / Daten-Hijacking)

- **Datei:** `src/WebHandler.cpp:1160–1175`
- **Severity:** 🟠 HIGH
- **Kategorie:** Security
- **Problem:**
  ```cpp
  if (json.containsKey("url")) {
    JsonObject _url = json["url"].as<JsonObject>();
    for (int i = 0; i < Cloud::serverurlCount; i++) {
      JsonObject _link = _url[Cloud::serverurl[i].typ].as<JsonObject>();
      if (_link.containsKey("host"))
        Cloud::serverurl[i].host = _link["host"].as<const char*>();
      if (_link.containsKey("page"))
        Cloud::serverurl[i].page = _link["page"].as<const char*>();
    }
    gSystem->cloud.saveUrl();
  }
  ```
  Cloud-Server-URLs (für Push-Notifications, Analytics, Update-Server) werden direkt aus dem Request übernommen.

- **Impact:**
  - **Daten-Hijacking:** Angreifer setzt `notification.host = "evil.com"` → alle Push-Notifications inkl. Probe-Daten/Status werden zum Angreifer-Server umgeleitet.
  - **Token-Leak:** Der Cloud-Token wird via Cloud-Push an den (jetzt vom Angreifer kontrollierten) Server gesendet → Token-Diebstahl.
  - **Persistenz:** `cloud.saveUrl()` Z. 1174 schreibt die URLs nach SPIFFS — Angriff überlebt Reboot.

- **Lösung:**
  - Whitelist auf bekannte Hosts (siehe CR-001b-001).
  - Wenn Custom-URLs grundsätzlich erlaubt sein sollen: nur via authentifizierten Endpoint, mit klarer User-Warnung im Frontend.
  - URL-Format-Validierung (kein `\r`, `\n`, `\0`, `?`, …).

- **Geschätzter Aufwand:** M

---

#### CR-001b-006: Alle `set*`-Handler — `deserializeJson()`-Return-Value wird ignoriert

- **Datei:** `src/WebHandler.cpp:575, 627, 688, 717, 746, 806, 970, 1072, 1154, 1267, 1287` (10× set + 1× addNetwork)
- **Severity:** 🟠 HIGH
- **Kategorie:** Logic
- **GitHub-Issue:** [#234](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/234)
- **Fix:** ✅ Commit `94e94aa` (2026-05-04)
- **Problem:**
  Jeder `set*`-Handler macht:
  ```cpp
  JsonDocument doc;
  deserializeJson(doc, (const char *)datas);  // ← Return-Wert verworfen
  JsonObject json = doc.as<JsonObject>();
  if (json.isNull()) return 0;
  ```
  Der `DeserializationError`-Return von `deserializeJson()` wird konsequent ignoriert. Bei JSON-Parse-Fehlern (Truncation, ungültiges Encoding, `Incomplete`-Error wegen Multi-Chunk-Body — siehe TASK-001 CR-001-001) ist `doc` ggf. nur partial-parsed: einige Felder sind gültig, andere nicht. Die `containsKey()`-Logik geht trotzdem durch.

- **Impact:**
  - Bei zerschnittenen Multi-Chunk-Bodies (CR-001-001 noch offen) werden teilweise Settings gespeichert — schwer zu debuggender Datenverlust.
  - Bei manipulierten Bodies (Trailing-Garbage, BOM, Zeichensatz-Mix) kann der Parser silent partielle Ergebnisse liefern.
  - **Mitigation für CR-001-001:** wenn der Error-Check da wäre, würde zerschnittenes JSON wenigstens als `400 Bad Request` enden statt silent partial save.

- **Lösung:**
  Standard-Pattern in jedem Handler einführen:
  ```cpp
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, (const char *)datas);
  if (err) {
    Log.warning("setXxx: JSON parse error: %s" CR, err.c_str());
    return 0;
  }
  ```
  Idealerweise als Helper-Funktion oder Makro in `WebHandler.cpp`, da Pattern 10× wiederholt.

- **Geschätzter Aufwand:** S (für Helper) + S (10× anwenden) = 0.5 PT

---

#### CR-001b-007: `setNetwork` / `addNetwork` — keine SSID/Password-Längen-Validierung

- **Datei:** `src/WebHandler.cpp:696–706, 725–735`
- **Severity:** 🟠 HIGH
- **Kategorie:** Security, Logic
- **Problem:**
  ```cpp
  if (_network.containsKey("ssid")) {
    ssid = checkString(_network["ssid"].as<const char*>());
    if (_network.containsKey("password"))
      password = _network["password"].as<const char*>();
    gSystem->wlan.addCredentials(ssid.c_str(), password.c_str());
    return 1;
  }
  ```
  WPA2-Standard: SSID max 32 Bytes, Password 8–63 Zeichen.
  Hier kein Längen-Check. `Wlan::addCredentials` muss intern die Längen prüfen — das ist nicht hier zu reviewen, aber ein klarer Defense-in-Depth-Verstoß.

- **Impact:**
  - Lange SSIDs/Passwords können in `addCredentials` overflow / silent truncation auslösen.
  - Truncated Password → Gerät kann sich nicht mehr verbinden, User merkt nicht warum.
  - Heap-Druck bei sehr langen Bodies.

- **Lösung:**
  ```cpp
  if (ssid.length() > 32 || password.length() > 63) {
    Log.warning("setNetwork: SSID or password too long" CR);
    return 0;
  }
  ```

- **Geschätzter Aufwand:** S

---

#### CR-001b-008: `setIoT` — keine MQTT-Port- und keine String-Längen-Validierung

- **Datei:** `src/WebHandler.cpp:760–798`
- **Severity:** 🟠 HIGH
- **Kategorie:** Security, Logic
- **Problem:**
  - Z. 760-761: `mqttConfig.port = _chart["PMQport"];` — Port wird direkt aus JSON gecastet (auf `uint16_t` im Struct). Werte > 65535 werden silent gewrappt; Wert 0 ist kein gültiger Port.
  - `mqttConfig.host` (Z. 757–758): `strncpy` mit Null-Term ✓ — aber der Caller nimmt JSON-`null` → leerer String → MQTT-Reconnect-Loop.
  - Z. 784, 791: `cloudConfig.cloudToken = _chart["CLtoken"].as<const char*>();` — direkter Assignment; kein Length-Check (Type ist Arduino `String`, also kein Buffer-Overflow, aber Heap-Belastung).
  - Z. 791: `cloudConfig.customUrl = _chart["CCLurl"].as<const char*>();` — wieder eine User-controlled URL für Cloud-Posts (gleiche Problematik wie CR-001b-005).

- **Impact:**
  - Ungültiger Port führt zu Connect-Loops (Heap-Druck, Watchdog-Risiko).
  - User-definierte Custom-Cloud-URLs ermöglichen Daten-Exfiltration.
  - Lange Tokens ohne Limit → Heap-Fragmentierung.

- **Lösung:**
  ```cpp
  if (_chart.containsKey("PMQport")) {
    int port = _chart["PMQport"];
    if (port < 1 || port > 65535) return 0;
    mqttConfig.port = port;
  }
  // CCLurl: gleiche Whitelist-Logik wie in CR-001b-001/005
  ```

- **Geschätzter Aufwand:** S

---

### 🟡 MEDIUM Priority Issues

#### CR-001b-009: `printRequest()` leakt Credentials im Serial-Debug-Build

- **Datei:** `src/WebHandler.cpp:572, 624, 685, 714, 743, 803, 967, 1069, 1264, 1284`
- **Severity:** 🟡 MEDIUM (steigt zu HIGH bei aktiviertem `DEBUG`)
- **Kategorie:** Security, Code-Quality
- **Problem:**
  Acht von zehn `set*`-Handlern rufen `printRequest(datas)` auf — das ist `DPRINTF("[REQUEST]\t%s\r\n", (const char *)datas)` (`WebHandler.cpp:61–64`).
  `DPRINTF` ist standardmäßig No-Op (`DbgPrint.h:59`), aber bei aktiviertem `DEBUG = SERIAL_DEBUG` werden plötzlich auf der Serial-Console geloggt:
  - **setNetwork/addNetwork** (Z. 685, 714): WiFi-Password im Klartext
  - **setIoT** (Z. 743): MQTT-User + Password, Cloud-Token, Custom-URL
  - **setPush** (Z. 803): Telegram-Bot-Token, Pushover-Bot-Token, Pushover-User-Key, FCM-Push-Tokens

  Ein Entwickler, der zur Fehlersuche `DEBUG = SERIAL_DEBUG` aktiviert (legitim, oft empfohlen), leakt damit alle gesetzten Credentials auf die Serial-Console — und diese ist bei Geräten mit USB-Anschluss / SerialMonitor mitlesbar.

- **Impact:**
  - Im Default-Build: keine direkte Leakage.
  - Bei aktiviertem Debug: alle Credentials sichtbar — vermutlich beim Crash-Reporting an WLANThermo gemeldet, evtl. in Log-Files gespeichert.
  - Anti-Pattern: "Debug-Code, der bei Aktivierung Credentials leakt" — sicherheits-kritischer Code sollte auch im Debug-Modus keine Klartext-Secrets ausgeben.

- **Lösung:**
  - In den 4 sensiblen Handlern (`setNetwork`, `addNetwork`, `setIoT`, `setPush`): `printRequest(datas)` entfernen oder durch eine maskierte Variante ersetzen, die JSON-Felder mit "password", "token", "key", "user_key" durch `***` ersetzt vor dem Drucken.
  - In den restlichen Handlern (`setSystem`, `setChannels`, `setPitmaster`, `setPID`, `setDCTest`, `setBluetooth`): unkritisch, kann bleiben.

- **Geschätzter Aufwand:** S
- **GitHub-Issue:** [#236](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/236)
- **Fix:** ✅ Commit `207c30b` (2026-05-04)

---

#### CR-001b-010: `setPitmaster` — Loop-Index `ii` vs. JSON-`id` Inkonsistenz

- **Datei:** `src/WebHandler.cpp:975–1058`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Logic
- **Problem:**
  ```cpp
  byte id, ii = 0;
  for (JsonObject _pitmaster : json) {
    Pitmaster *pm = gSystem->pitmasters[ii];   // ← Loop-Index
    // ...
    if (_pitmaster.containsKey("id")) id = _pitmaster["id"];
    else break;
    if (id >= gSystem->pitmasters.count()) break;  // ← validiert id, nicht ii
    // ... pm wird konfiguriert ...
    ii++;
  }
  ```
  `pm` wird über den Loop-Index `ii` gewählt, validiert wird aber `id` aus dem JSON-Element. Wenn JSON-Reihenfolge und Pitmaster-Index nicht 1:1 matchen, wird der falsche Pitmaster konfiguriert.
  Beispiel: JSON `[{"id":1, "set":80}, {"id":0, "set":100}]` → `pitmasters[0]` bekommt set=80 (von id=1), `pitmasters[1]` bekommt set=100 (von id=0). Vertauschte Konfig.

- **Impact:**
  Frontend-seitig wird das vermutlich durch konsistente Sortierung umgangen — aber jedes andere API-Konsument-Tool (Cordova-App, Flutter-App, Drittanbieter, Skript) kann darauf reinfallen.

- **Lösung:**
  Konsistent `id` als Index verwenden:
  ```cpp
  for (JsonObject _pitmaster : json) {
    if (!_pitmaster.containsKey("id")) break;
    byte id = _pitmaster["id"];
    if (id >= gSystem->pitmasters.count()) break;
    Pitmaster *pm = gSystem->pitmasters[id];
    if (pm == nullptr) continue;
    // ... konfiguriere pm ...
  }
  ```
  `ii` ganz entfernen.

- **Geschätzter Aufwand:** S

---

#### CR-001b-011: `setBluetooth` — Undefined Behavior durch fehlenden Bit-Shift-Upper-Bound

- **Datei:** `src/WebHandler.cpp:1308–1313`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Logic
- **Problem:**
  ```cpp
  uint8_t count = _device["count"];      // Z. 1308 — bis zu 255
  uint32_t selected = _device["selected"];
  for (uint8_t i = 0u; i < count; i++) {
    if (selected & (1 << i)) { /* ... */ }   // Z. 1313
  }
  ```
  `1 << i` ist ein `int`-Shift. Wenn `i >= 32` (oder schon `i >= 31` für signed) → **Undefined Behavior** laut C++-Standard (5.8.1).
  `count` kommt vom Client → Angreifer kann `count = 100` schicken → UB.

- **Impact:**
  - In der Praxis bei ESP32-GCC: typisch `0` (Right-Shift-Verhalten ungenau definiert) → keine Sensoren werden hinzugefügt — wäre ein DoS gegen Bluetooth-Setup, aber nicht crashend.
  - Compiler-Optimierungen können hier "interessante" Effekte produzieren.

- **Lösung:**
  ```cpp
  if (count > 32) count = 32;
  for (uint8_t i = 0u; i < count; i++) {
    if (selected & (1u << i)) { /* ... */ }   // 1u statt 1
  }
  ```

- **Geschätzter Aufwand:** S
- **GitHub-Issue:** [#237](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/237)
- **Fix:** ✅ Commit `fc0ece1` (2026-05-04)

---

#### CR-001b-012: `setServerAPI` — `cloud.saveUrl()` direkt aus async_tcp

- **Datei:** `src/WebHandler.cpp:1174`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Performance
- **Problem:**
  `Cloud::saveUrl()` schreibt vermutlich nach SPIFFS (CLAUDE.md: "SPIFFS für Cloud-URL-Cache"). SPIFFS-Writes sind blocking, ähnlich wie NVS — direkt aus async_tcp ist Anti-Pattern.

- **Impact:** wie CR-001-006 (TASK-001).

- **Lösung:** Pending-Flag-Pattern, analog zu MQTT/Cloud-Config-Saves.

- **Geschätzter Aufwand:** S

---

### 🟢 LOW Priority / Verbesserungsvorschläge

#### CR-001b-013: Mehrere `Serial.println` statt ArduinoLog

- **Datei:** `src/WebHandler.cpp:1162, 1206, 1214, 1239, 1240, 1253, 1254, 1304`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality
- **Problem:** 8 direkte `Serial.print(ln)` — verstößt gegen CLAUDE.md-Konvention "Logging uses ArduinoLog macros — not Serial.print".
  Anders als die `DPRINTF`-Calls sind diese hier **immer aktiv** (kein Debug-Macro), produzieren also auch im Production-Build Serial-Output und können für ArduinoLog-Logging-Subsysteme nicht stummgeschaltet werden.
- **Lösung:** Durch `Log.notice / verbose / warning` ersetzen.

#### CR-001b-014: `setChannels` — `byte _typ > -1` ist immer wahr (unsigned-Vergleich)

- **Datei:** `src/WebHandler.cpp:653`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality, Logic
- **Problem:**
  ```cpp
  byte _typ;
  if (_typ > -1 && _typ < temperature->getTypeCount())   // immer true, "tot"
  ```
  `byte` ist `uint8_t`, `-1` wird zu `0xFF` promoted — `_typ > 0xFF` ist nie wahr (max 255), also der Vergleich ist effektiv `_typ > 255` → immer false. Hmm, eigentlich umgekehrt: `_typ > -1` mit unsigned promotion macht `_typ > UINT_MAX-1` → immer false? Compiler-Warnung üblich.

  Gemeint war vermutlich "nicht-negativ und in Range".
- **Lösung:** Da `_typ` `byte` ist, reicht `if (_typ < temperature->getTypeCount())`.

#### CR-001b-015: `setDCTest` — Dead variable `dc`

- **Datei:** `src/WebHandler.cpp:1273`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality
- **Problem:** `bool dc = json["dc"];` — wird nie verwendet. `startDutyCycleTest(aktor, val)` (Z. 1279) hat keinen `dc`-Parameter.
- **Lösung:** Entfernen.

#### CR-001b-016: `setDCTest` — TODO-Kommentare und Frontend-Kopplung

- **Datei:** `src/WebHandler.cpp:1274–1276`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality
- **Problem:**
  ```cpp
  val /= 10;  //TODO: why is value multiplied with 10 in frontend?
  byte id = 0;  // Pitmaster0 // TODO: add id to frontend
  ```
  Zwei TODOs deuten auf unfertige Implementierung. `id = 0` heißt, `setDCTest` betrifft immer nur Pitmaster 0.
- **Lösung:** TODO-Verträge klären (Frontend-Anpassung), oder die Kommentare in dokumentierte Limits umwandeln (`// nur Pitmaster 0 unterstützt — siehe Issue #XXX`).

#### CR-001b-017: `setPitmaster` — auskommentierte/TODO-Kommentare

- **Datei:** `src/WebHandler.cpp:1007–1008, 1022`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality
- **Problem:**
  ```cpp
  //open_lid_init(); // Speicher zurücksetzen
  //TODO: clear open lid data at pitmaster
  // ...
  //Serial.println("PID-Wechsel");
  ```
- **Lösung:** Entfernen (Git-Historie / Issue-Tracker reichen).

#### CR-001b-018: `addNetwork` — deklariert aber nicht im Routing-Array

- **Datei:** `src/WebHandler.h:75`, `src/WebHandler.cpp:711–738`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality
- **Problem:**
  Die Methode `addNetwork()` ist im Header deklariert und in der `.cpp` implementiert (~28 LOC), wird aber **in `nanoWebHandlerList[]` nicht registriert**. Damit kein erreichbarer Endpoint → toter Code.
  `setNetwork` und `addNetwork` unterscheiden sich nur durch das letzte `true` an `addCredentials()` — es gibt keinen Endpoint, der die Add-only-Variante triggert.
- **Lösung:** Entweder URL hinzufügen (`/addnetwork`) wenn Frontend das nutzt, oder die Methode entfernen.

#### CR-001b-019: `setSystem` — Hostname/AP-Limit `< 14` ist undokumentiert

- **Datei:** `src/WebHandler.cpp:597, 604`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality
- **Problem:** `if (checkStringLength(_name) < 14)` — magische Zahl 14. Kein Constant, kein Kommentar warum.
  `checkStringLength` zählt UTF-8-Mehrbyte-Sequenzen als ein Zeichen — also wahrscheinlich "max 13 sichtbare Zeichen" wegen mDNS-Hostname-Limits.
- **Lösung:** `static constexpr size_t MAX_HOSTNAME_LEN = 13;` mit Kommentar.

---

## Positive Aspekte

- ✅ **ArduinoJson v7 durchgängig korrekt:** `JsonDocument`, `as<JsonObject>()`, `as<JsonArray>()`, `as<JsonObject>()` für Sub-Objects — keine v5-Reste.
- ✅ **`xxxConfigSavePending`-Pattern in 5 von 10 Handlern korrekt verwendet** (`setSystem`, `setChannels`, `setPush`, `setPitmaster`, `setPID`) — saubere Linie aus P1-Konvention.
- ✅ **`setPush` Z. 899–921:** Defensive Length-Checks **vor** den `strcpy`-Aufrufen für Klartext-Token-Pfad — vorbildliche Defensiv-Programmierung. (Schade, dass das im Hash-Pfad fehlt, siehe CR-001b-004.)
- ✅ **`setIoT` Z. 757–771:** korrekte `strncpy` + explizites Null-Term-Pattern.
- ✅ **`setPitmaster` Z. 1003:** Range-Check für `cha < 1 || cha > count()` vor Pointer-Zugriff — korrekt.
- ✅ **`setPID` Z. 1116:** `constrain(_pid["jp"].as<int>(), 10, 100)` — Eingabe-Range explizit begrenzt.
- ✅ **`setPID` Z. 1102–1109:** dito für DCmin/DCmax.

---

## Vergleich zum vorherigen Sonnet-Review

- **Damals gefundene Issues:** B23–B35 (13 Bugs Sammelnummer), Initial-Review umfasste TASK-001 + TASK-001b zusammen. Konkrete Sonnet-Findings für `set*`-Handler nicht im git-Log einzeln dokumentiert.
- **Davon noch relevant:** Der `strncpy`-Pattern in `setIoT` und `setPush` wurde sauber gesetzt (vermutlich Sonnet-Fix). ✓ steht heute.
- **Neue Findings, die Sonnet übersehen hat:**
  - **CR-001b-001** (Update-URL-Injection — schwerwiegender RCE-Vektor!) — Sonnet hat den `setServerAPI`-Handler nicht systematisch auf "wer kontrolliert die URL" geprüft.
  - **CR-001b-002** (`startUpdate()` aus async_tcp).
  - **CR-001b-003** (NVS-Writes in `setBluetooth` direkt) — Pattern wurde in den anderen Handlern korrekt eingeführt, bei `setBluetooth` vergessen.
  - **CR-001b-004** (Buffer-Overflow im hash-resolution-Pfad).
  - **CR-001b-005** (Cloud-URL-Hijacking via `host`/`page`).
  - **CR-001b-006** (durchgängig fehlender `deserializeJson()`-Error-Check).
  - **CR-001b-010** (Loop-Index vs. JSON-id Inkonsistenz).
  - **CR-001b-011** (Bit-Shift-UB).
- **Sonnet-False-Positives:** Keine identifiziert.
- **Bewertung des Sonnet-Reviews:** Hat die **API-Migrations-Themen** (v7-Pattern, `strncpy`-Sicherung) gut adressiert, aber **Security-Architektur** (URL-Injection, RCE-Vektoren, SSRF) nicht systematisch erfasst. Re-Review-Wertschöpfung **noch klarer** als bei TASK-001.

---

## Metriken

| Metrik | Wert |
|--------|------|
| **Gefundene Issues — CRITICAL** | 3 |
| **Gefundene Issues — HIGH** | 5 |
| **Gefundene Issues — MEDIUM** | 4 |
| **Gefundene Issues — LOW** | 7 |
| **Gefundene Issues — gesamt** | 19 |
| **Geschätzter Fix-Aufwand** | ~5–6 PT (CR-001b-001 ist M; Rest summiert auf ~3–4 PT; LOWs gebündelt ~0.5 PT) |
| **Code-Qualität (1–10)** | 5 — solide JSON-v7-Basis und gutes pending-Pattern, aber zu viele Validation-Lücken auf User-Input-Pfaden |
| **Wartbarkeit (1–10)** | 6 — Handler sind kompakt, aber das Pattern-Duplikat (`JsonDocument`, `deserializeJson`, `isNull`-Check, `containsKey`-Walls) verlangt nach einer Helper-Schicht |

---

## Empfohlene Nächste Schritte

1. **Sofort fixen — CR-001b-001 + CR-001b-002 + CR-001b-005** zusammen — der `setServerAPI`-Handler ist der derzeit kritischste Angriffsvektor im gesamten WebHandler. Empfehlung: Endpoint mit `requireAuth() = true` schützen, URLs auf Whitelist beschränken, `startUpdate` via Pending-Flag.
2. **CR-001b-003** (`setBluetooth` NVS-Writes) — gleicher Sprint, da gleiches Pattern wie CR-001-006/007 aus TASK-001.
3. **CR-001b-006** (Error-Check) — Helper-Funktion einführen, dann 10 Stellen aktualisieren. Mildert auch CR-001-001 (Multi-Chunk-Body) im Failure-Mode.
4. **CR-001b-004** (Hash-Path Buffer-Overflow) — schnell fixbar, hohe Defensiv-Wirkung.
5. **CR-001b-007 / 008** (SSID/Port/Length-Validierung) — Defense-in-Depth.
6. **MEDIUM/LOW** in Cleanup-PR bündeln.

---

## Anhang: Analysierte Dateien

| Datei | LOC (im Scope) | Bemerkung |
|-------|----------------|-----------|
| `src/WebHandler.cpp` (Z. 569–1328) | ~760 | alle 10 `set*`-Handler + `addNetwork` (toter Code) + 2 Helper (`checkString`, `checkStringLength`) |
| `src/Notification.h` (Z. 71–84) | — | Verifikation `PushAppDeviceType::token` Größe (255) |
| `src/DbgPrint.h` (Z. 33–65) | — | Verifikation `DPRINTF`-Aktivierungslogik |

---

## Tracker-Update

> Folgende Zeile in `CODE_REVIEW_TRACKER.md` übernehmen:

```markdown
| TASK-001b | WebHandler — Write/POST | 760 | 🔴 CRITICAL | ✅ DONE | 3/5/4/7 | reviews/REVIEW_TASK_001b.md | RCE-Vektor in setServerAPI; `setBluetooth` NVS direkt; Error-Check fehlt |
```

**Status-Übersicht aktualisieren:**
- Tasks abgeschlossen: 1 → 2 / 34 (~6 %)
- Tasks offen: 30 → 29
- Im neuen Review gefundene Issues — Critical: +3 (Total 6), High: +5 (Total 10), Medium: +4 (Total 8), Low: +7 (Total 13). **Gesamt: 37 Findings.**

**Findings-Sektion auffüllen:**
- 🔴 CRITICAL: CR-001b-001 (Update-URL-Injection), CR-001b-002 (`startUpdate` async_tcp), CR-001b-003 (`setBluetooth` NVS direkt)
- 🟠 HIGH: CR-001b-004…008
- 🟡 MEDIUM: CR-001b-009…012
- 🟢 LOW: CR-001b-013…019
