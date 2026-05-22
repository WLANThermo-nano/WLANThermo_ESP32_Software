# Review Results — TASK-005

**Modul:** RecoveryMode — Captive-Portal-AP
**Review-Datum:** 2026-05-05
**Reviewer:** Claude Opus 4.7
**Dateien:** 2 Dateien (`src/RecoveryMode.cpp` 349 LOC, `src/RecoveryMode.h` 58 LOC), ~407 Lines of Code
**Vorheriger Review:** Sonnet Initial-Review 2026-04-25 (B23–B35-Cluster, im git-Log dokumentiert)

---

## Executive Summary

`RecoveryMode` öffnet bei langem Recovery-Pin oder via `runFromApp()` einen unauthentifizierten WiFi-Access-Point (`WLANThermo-RecoveryMode` / Passwort `"12345678"`) und exponiert dort einen Webserver mit **vollen Firmware-Upload-, NVS-Write/Read- und Settings-Export-Endpoints** — alles ohne Authentifizierung. Damit ist RecoveryMode aktuell eine **vollständige RCE-Hintertür**: Jeder in WiFi-Reichweite kann arbiträre Firmware flashen, NVS-Inhalte lesen oder schreiben und alle gespeicherten Credentials (WiFi, MQTT, Cloud, Telegram, Pushover, Admin-Passwort) im Klartext exfiltrieren. Zusätzlich existiert ein klarer Logik-Bug im `/import`-Body-Handler (Zeile 214 setzt `settingsValue`/`receivedBytes` bei jedem Chunk zurück, nicht nur beim ersten), zwei `strcpy`-Buffer-Overflows in `runFromApp()` (RTC-RAM-Korruption) und ein Regression-Punkt: das `/restart`-Endpoint nutzt das alte `delay(1000) + ESP.restart()`-Pattern aus async_tcp-Kontext, das laut CLAUDE.md bereits durch `esp_timer` ersetzt worden sein sollte.

**Gesamtbewertung:**
- 🔴 4 CRITICAL (Auth-Bypass-Cluster, RTC-RAM-Overflow, /import-Logikbug, /export-Credential-Leak), 5 HIGH (PW-Default, async_tcp-Anti-Pattern-Regression, infinite-Loop, NVS-Whitelist, Filetype-Detection), 4 MEDIUM, 6 LOW. **Höchste Priorität im gesamten Review-Backlog** — Recovery-Mode betrifft jedes ausgelieferte Gerät und ist physisch über Recovery-Pin (3 s GPIO 14) jederzeit aktivierbar.

---

## Review-Checkliste — angewandt

### 1. Design & Architektur
- Kein Auth-Konzept überhaupt — Recovery-Mode war historisch als „local serial console replacement" gedacht, aber via WiFi-AP weltweit angreifbar (10–50 m Reichweite). Architektur-Lücke.
- Mehrere static `String`-Felder (`settingsKey`, `settingsValue`) für Multi-Chunk-Body — keine Synchronisation für gleichzeitige Requests.

### 2. Logik & Korrektheit
- `/import`-Body-Handler hat einen klassischen C-Statement-Falle: `if(!index)` schützt nur die erste folgende Anweisung — Rest der Zeile läuft pro Chunk. **CR-005-003**.
- `runFromApp` läuft in eine Endlosschleife, wenn die App falsche Credentials liefert. **CR-005-007**.
- `getFileType` nutzt `indexOf` für „firmware"/„.bin" — trivial bypassbar. **CR-005-009**.

### 3. Performance & Effizienz
- Forever-while-loop (Zeile 314–317) mit `delay(100)` — keine Power-Saving in Recovery-Mode, mit aktivem AP zieht das Gerät dauerhaft viel Strom. **CR-005-015**.

### 4. ESP32 / FreeRTOS
- `/restart`-Handler nutzt `delay(1000) + WiFi.disconnect() + ESP.restart()` direkt in async_tcp-Callback — Regression gegen die in CLAUDE.md dokumentierte Konvention („esp_timer One-Shot ≥2 s"). `/uploadfile` zeigt im selben File das korrekte Pattern (Zeile 241–246). **CR-005-006**.

### 5. Security / Input-Validation
- **CRITICAL — CR-005-001:** Kein Auth-Check auf irgendeinem Endpoint.
- **CRITICAL — CR-005-002:** `strcpy(wifiName, paramWifiName)` ohne Längencheck → Overflow in adjacent RTC-DATA `wifiPassword`.
- **CRITICAL — CR-005-004:** `/export` dumpt gesamte NVS-Settings (inkl. Credentials) unauthentifiziert.
- **HIGH — CR-005-005:** AP-Passwort hartkodiert „12345678" — bekanntes Default + global identische Firmware.
- **HIGH — CR-005-008:** `/import` erlaubt arbiträres `Settings::write(<jeder NVS-Key>, <beliebiger Wert>)` und `Settings::remove(<jeder NVS-Key>)`.
- **MEDIUM — CR-005-010:** `usize`-Param ohne Range-Check → `Update.begin(uploadFileSize)` mit beliebiger Größe.
- **MEDIUM — CR-005-012:** Keine Firmware-Signaturprüfung — `Update.h` unterstützt RSA-signierte Updates, aber unbenutzt.

### 6. Code-Qualität
- `RMPRINTF` mit Klartext-PW (Zeile 131). **CR-005-014**.
- `boolean fromApp` statt `bool`. **CR-005-017**.
- Toter Code: `RECOVERY_RESET_THRESHOLD`-Define unused, condition auskommentiert. **CR-005-018**.
- Memory-Leak `new AsyncWebServer(80)` (Zeile 152) — akademisch, weil run-forever-Loop, aber Code-Smell. **CR-005-019**.

---

## Findings

> Konvention: `CR-005-NNN`, fortlaufend (Critical → High → Medium → Low).

---

### 🔴 CRITICAL Issues (Blocker)

#### CR-005-001: Vollständiger Auth-Bypass auf allen Recovery-Mode-Endpoints — RCE via AP ⏳ OFFEN

- **Datei:** `src/RecoveryMode.cpp:154-310` (alle 9 Handler)
- **Severity:** 🔴 CRITICAL
- **Kategorie:** Security
- **Problem:**
  Recovery-Mode öffnet einen Open-Access-Webserver auf 192.168.66.1:80 (AP-Mode) bzw. STA-IP (App-Mode), der ohne jede Authentifizierung erlaubt:
  - **`/uploadfile`** (Zeile 221–305) — beliebige Firmware/SPIFFS/Nextion-Binary uploaden + flashen
  - **`/import`** (Zeile 203–219) — beliebigen NVS-Key überschreiben oder löschen
  - **`/export`** (Zeile 194–201) — gesamten Settings-Dump (inkl. Credentials)
  - **`/reset`** (Zeile 178–181) — alle NVS-Settings löschen (`Settings::clear()`)
  - **`/cleanpush`** (Zeile 188–192) — Push-Notification-Settings löschen
  - **`/restart`** (Zeile 168–176) — Gerät rebooten

- **Impact:**
  Vollständige Kompromittierung jedes Geräts, das in Recovery-Mode-Reichweite steht (3 s GPIO-14-LOW physisch oder via gültiger STA-Trigger via `runFromApp`). Angreifer kann:
  1. Über `/export` alle gespeicherten Credentials auslesen (siehe CR-005-004)
  2. Beliebige Firmware flashen (Persistent Backdoor, nutzt Geräte-WiFi nach Wiederherstellung der STA-Verbindung als Pivot ins Heimnetzwerk)
  3. NVS-Settings manipulieren (gefälschte WiFi-AP, Cloud-Endpoint-Hijacking)

- **Root Cause:**
  Recovery-Mode war historisch für serielle/lokale Wartung gedacht. WiFi-AP wurde später hinzugefügt, ohne Auth-Modell zu ergänzen.

- **Lösung:**
  - Mindestschutz: HTTP-Basic-Auth mit Geräte-individueller PIN, die auf dem Display/Serial während Recovery angezeigt wird. Neue 6-stellige PIN bei jedem Recovery-Start (RTC RNG).
  - Ideale Lösung: TLS + Client-Certificate (zertifikatsgepinnter App-Trigger).
  - Mittelfristig: AP-Passwort gerätespezifisch (siehe CR-005-005).

- **Code-Beispiel:**
  ```cpp
  // ❌ Aktuell:
  webServer->on("/uploadfile", HTTP_POST, [](AsyncWebServerRequest *request) {
    // ... beliebigen User direkt akzeptieren
  });

  // ✅ Empfohlen (HTTP-Basic-Auth-Wrapper):
  static String recoveryPin;   // beim AP-Start zufällig generieren + auf Display zeigen
  static auto requireRecoveryPin = [](AsyncWebServerRequest *request) -> bool {
    if (!request->authenticate("recovery", recoveryPin.c_str(), "WLANThermo Recovery")) {
      request->requestAuthentication("WLANThermo Recovery");
      return false;
    }
    return true;
  };
  webServer->on("/uploadfile", HTTP_POST,
    [](AsyncWebServerRequest *request) {
      if (!requireRecoveryPin(request)) return;
      // ...
    });
  ```

- **Geschätzter Aufwand:** L (~3 PT — PIN-Generator, Display-Anzeige für jede Hardware-Variante, alle 9 Endpoints umstellen, dokumentieren)
- **GitHub-Issue:** [#250](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/250)

---

#### CR-005-002: `strcpy` ohne Längencheck in `runFromApp()` → RTC-RAM-Buffer-Overflow ✅ GEFIXT

- **Datei:** `src/RecoveryMode.cpp:63-75`, `src/RecoveryMode.h:54-55`
- **Severity:** 🔴 CRITICAL
- **Kategorie:** Memory Safety
- **Problem:**
  ```cpp
  static RTC_DATA_ATTR char wifiName[33];      // 32 chars + null
  static RTC_DATA_ATTR char wifiPassword[64];  // 63 chars + null

  void RecoveryMode::runFromApp(const char *paramWifiName, const char *paramWifiPassword) {
    fromApp = true;
    strcpy(wifiName, paramWifiName);         // ← keine Längenprüfung
    strcpy(wifiPassword, paramWifiPassword); // ← keine Längenprüfung
    ...
  }
  ```
  `runFromApp` wird via `Wlan::setRecoveryPending()`-Flag aus dem normalen WebHandler-Pfad getriggert (siehe CLAUDE.md). Dort kommen die Strings aus JSON-Body — nutzergesteuert.

- **Impact:**
  - 33+ Byte SSID → schreibt ins adjacent `wifiPassword`-Feld (Layout-abhängig) → korrupte Recovery-Credentials.
  - 64+ Byte PW → schreibt in `fromApp`-Variable bzw. weitere RTC-DATA-Bereiche → unvorhersagbares Recovery-Verhalten nach Deep-Sleep, möglicher Boot-Loop.
  - Übler Worst-Case: gezielter Overflow korrumpiert RTC-noinit-Region (`resetCounter`) → Recovery-Mode-Trigger nach normalem Boot, Lock-out des Users.

- **Root Cause:**
  Klassisches Anti-Pattern, das von der `SAFE_STRNCPY`-Konvention (CLAUDE.md, `Notification.cpp`) ausgenommen wurde — RTC-RAM-Sektion fühlt sich „statisch und sicher" an.

- **Lösung:**
  ```cpp
  // ✅ Empfohlen:
  void RecoveryMode::runFromApp(const char *paramWifiName, const char *paramWifiPassword) {
    if (paramWifiName == nullptr || paramWifiPassword == nullptr) return;
    if (strlen(paramWifiName) >= sizeof(wifiName) ||
        strlen(paramWifiPassword) >= sizeof(wifiPassword)) {
      Log.error("RecoveryMode::runFromApp: oversized credentials, abort" CR);
      return;
    }
    fromApp = true;
    strncpy(wifiName, paramWifiName, sizeof(wifiName) - 1);
    wifiName[sizeof(wifiName) - 1] = '\0';
    strncpy(wifiPassword, paramWifiPassword, sizeof(wifiPassword) - 1);
    wifiPassword[sizeof(wifiPassword) - 1] = '\0';
    ...
  }
  ```
  Oder `SAFE_STRNCPY` aus `Notification.cpp` reuseen.

- **Geschätzter Aufwand:** S
- **GitHub-Issue:** [#249](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/249)
- **Fix:** ✅ Commit `c39ac8f` (2026-05-22)

---

#### CR-005-003: `/import`-Body-Handler — `if(!index)` schützt nur erste Anweisung ✅ GEFIXT; `settingsValue` wird pro Chunk reset → Multi-Chunk-Body ergibt nie korrekt zusammengesetzten Wert

- **Datei:** `src/RecoveryMode.cpp:210-219`
- **Severity:** 🔴 CRITICAL
- **Kategorie:** Logic
- **Problem:**
  ```cpp
  if(!index) settingsKey = request->header("xKey"); settingsValue = ""; receivedBytes = 0u;
  ```
  `if(!index)` bezieht sich nur auf das nächste Statement (`settingsKey = ...`). Die folgenden zwei Statements (`settingsValue = ""` und `receivedBytes = 0u`) **laufen bei jedem Chunk** und resetten den Akkumulator-Zustand.

- **Impact:**
  - Bei einem Multi-Chunk-Body (typisch bei Settings-Import > 1460 Byte / TCP-MSS) wird `settingsValue` in jedem Chunk komplett verworfen → am Ende landet nur der **letzte Chunk** in der NVS, nicht der vollständige Body.
  - `receivedBytes` startet jeden Chunk bei 0 → die Bedingung `receivedBytes == total` (Zeile 219) wird nur dann wahr, wenn der letzte einzelne Chunk gleichzeitig dem `total` entspricht — sehr fragiler Zufall.
  - In der Praxis: Settings-Import zerstört oder vergisst beim Restore den Großteil der ursprünglichen Konfiguration.

- **Root Cause:**
  Klassischer C/C++-Statement-Trap — fehlende geschweifte Klammern um den `if`-Block. Sonnet-Review hat das nicht gefangen.

- **Lösung:**
  ```cpp
  // ❌ Aktuell:
  if(!index) settingsKey = request->header("xKey"); settingsValue = ""; receivedBytes = 0u;
  memset(s.get(), 0, len + 1u);
  memcpy(s.get(), data, len);
  settingsValue += s.get();
  receivedBytes += len;
  if(receivedBytes == total) Settings::write(settingsKey, settingsValue);

  // ✅ Empfohlen:
  if (!index) {
    settingsKey   = request->header("xKey");
    settingsValue = "";
    receivedBytes = 0u;
  }
  memset(s.get(), 0, len + 1u);
  memcpy(s.get(), data, len);
  settingsValue += s.get();
  receivedBytes += len;
  if (receivedBytes == total) {
    Settings::write(settingsKey, settingsValue);
  }
  ```

- **Geschätzter Aufwand:** S — aber Coverage-Test-Pflicht (mehrchunkiger Import).
- **GitHub-Issue:** [#249](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/249)
- **Fix:** ✅ Commit `c39ac8f` (2026-05-22)

---

#### CR-005-004: `/export` dumpt alle Settings inkl. Credentials unauthentifiziert ⏳ OFFEN

- **Datei:** `src/RecoveryMode.cpp:194-201`
- **Severity:** 🔴 CRITICAL
- **Kategorie:** Security / Credential-Leak
- **Problem:**
  Der Endpoint liefert `Settings::exportFile()` als Plaintext-Download. Recovery-Settings enthalten alle gespeicherten NVS-Konfigurationen — inkl. WiFi-Credentials (`kWlan`), MQTT-Passwort (`kMqtt`), Cloud-Token (`kCloud`), Telegram-/Pushover-/App-Tokens (`kPush`), Admin-Passwort (`kServer`). Alles ohne Authentifizierung abrufbar.

- **Impact:**
  - Vollständige Credential-Exfiltration in einem einzigen GET-Request, sobald Angreifer im Recovery-AP ist (PW „12345678" — siehe CR-005-005).
  - Cross-Service-Compromise: Cloud-Token + WiFi-Credentials → Lateral-Movement ins Home-Netzwerk + Cloud-Account-Übernahme.
  - DSGVO-/Privacy-Issue (App-Tokens identifizieren Nutzer-Smartphones).

- **Root Cause:**
  Endpoint historisch als „Backup vor Restore" gedacht — Auth-Bedarf nie reflektiert.

- **Lösung:**
  - Mit CR-005-001 zusammen: Auth-Pflicht.
  - Zusätzlich: Credential-Felder in `Settings::exportFile()` maskieren (`"password": "***"`, `"token": "***"`) — analog CR-001-005-Strategie. Backup soll Konfiguration restorebar machen, aber Secrets nicht im Klartext exportieren.
  - Alternativ: Symmetrisches Pre-Shared-Key-Encryption mit Recovery-PIN als Schlüssel.

- **Geschätzter Aufwand:** M (~1 PT — Maskierung + Auth-Wrapper)
- **GitHub-Issue:** [#250](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/250)

---

### 🟠 HIGH Priority Issues

#### CR-005-005: Hartkodiertes Recovery-AP-Passwort `"12345678"` — global identisch ⏳ OFFEN

- **Datei:** `src/RecoveryMode.cpp:41`
- **Severity:** 🟠 HIGH
- **Kategorie:** Security
- **Problem:**
  ```cpp
  #define RECOVERY_AP_PASSWORD "12345678"
  ```
  Da die Firmware öffentlich (GitHub) ist, kennt jeder das Passwort. Kombiniert mit CR-005-001 ist Recovery-Mode trivial angreifbar, sobald jemand „WLANThermo-RecoveryMode" SSID findet (WiGLE-Mapping, Drive-by, Nachbar-Reichweite).

- **Impact:**
  Public-known default credentials → effektiv „offene SSID" bei Auth-Bypass. Selbe Klasse wie CR-003-008 (Wlan AP-Default), aber mit dramatisch höherem Impact-Faktor wegen RCE-Möglichkeit dahinter.

- **Root Cause:**
  Statisch im Source — keine pro-Gerät-Generierung.

- **Lösung:**
  - Pro-Gerät zufälliges PW (vom Bootloader generiert + im NVS persistiert), beim Recovery-Start auf Display zeigen.
  - Fallback bei Geräten ohne Display (NanoV3): PW = Geräte-Serial (`gSystem->getSerialNumber()`) + 4 Random-Hex aus RTC-RNG; Anzeige via Serial.
  - Bei MiniV2/V3: Display-Output „PW: A4F7C2" (6 hex chars + checksum).

- **Geschätzter Aufwand:** M (~1 PT — pro Hardware-Variante anpassen)
- **GitHub-Issue:** [#250](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/250)

---

#### CR-005-006: `/restart` benutzt `delay(1000) + WiFi.disconnect() + ESP.restart()` aus async_tcp — Regression gegen CLAUDE.md-Konvention ✅ GEFIXT

- **Datei:** `src/RecoveryMode.cpp:168-176`
- **Severity:** 🟠 HIGH
- **Kategorie:** FreeRTOS-Async-Safety / Regression
- **Problem:**
  ```cpp
  webServer->on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = ...;
    request->send(response);
    WiFi.disconnect();        // killt TCP-Verbindung VOR Response-Flush
    delay(1000);              // blockiert async_tcp
    gSystem->restart();       // direkter Restart
  });
  ```
  Genau das Anti-Pattern, das CLAUDE.md explizit verbietet:
  > **Restart nach async_tcp-Response:** `esp_timer` (One-Shot, ≥2 s) statt `delay() + ESP.restart()` im Handler-Kontext. `delay()` in async_tcp-Callback blockiert den Task; `WiFi.disconnect()` unmittelbar nach `request->send()` killt die TCP-Verbindung vor dem Flush.

  Im selben File (Zeile 241–246, `/uploadfile`) wird das korrekte `esp_timer`-Pattern bereits verwendet — d.h. der Fix war bekannt, wurde hier aber nicht angewendet. Auch im `WServer.cpp` ist mittlerweile `restartDeferred()` Standard.

- **Impact:**
  - Browser sieht oft keinen Response (TCP-RST vor Flush) → User unsicher, ob Restart erfolgte.
  - `delay(1000)` blockiert async_tcp-Task → andere parallele Requests laufen ins Timeout.

- **Root Cause:**
  Sonnet hat beim Initial-Review die `/uploadfile`-Restart-Stelle gefixt, `/restart`-Endpoint übersehen.

- **Lösung:**
  ```cpp
  webServer->on("/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(...);
    response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);

    esp_timer_handle_t timer;
    esp_timer_create_args_t args = {};
    args.callback = [](void *) { ESP.restart(); };
    args.name = "rec_rst";
    if (esp_timer_create(&args, &timer) == ESP_OK)
      esp_timer_start_once(timer, 2000000);   // 2 s
  });
  ```
  Oder `gSystem->restartDeferred()` verwenden, falls hier verfügbar (in `RecoveryMode::run()` ist `gSystem` allerdings nicht initialisiert — `RecoveryMode` ist Pre-Boot). Alternativ einen lokalen `restartDeferred`-Helper im File definieren.

- **Geschätzter Aufwand:** S
- **GitHub-Issue:** [#249](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/249)
- **Fix:** ✅ Commit `c39ac8f` (2026-05-22)

---

#### CR-005-007: `runFromApp` hängt in Endlos-Loop, wenn WiFi-STA nicht verbinden kann ✅ GEFIXT

- **Datei:** `src/RecoveryMode.cpp:133-137`
- **Severity:** 🟠 HIGH
- **Kategorie:** Logic / Robustness
- **Problem:**
  ```cpp
  while (WiFi.isConnected() == false)
  {
    RMPRINTLN("Wifi not connected");
    delay(1000);
  }
  ```
  Kein Timeout, kein Retry-Limit. Wenn die App ein falsches PW liefert, das STA-Netzwerk außer Reichweite ist, oder das AP weg ist, hängt das Gerät dauerhaft im Recovery-Mode-Loop.

- **Impact:**
  - User muss physisch Reset drücken oder Power-Cycle, um aus dem Recovery-Mode zurückzukehren.
  - Gerät ist während dieser Zeit nicht erreichbar (kein Fallback auf AP-Mode-Recovery).

- **Root Cause:**
  Naive Wartebedingung; keine Connection-Retry-Logik.

- **Lösung:**
  Timeout (z.B. 60 s) + Fallback auf AP-Mode:
  ```cpp
  uint32_t waitStart = millis();
  while (WiFi.isConnected() == false) {
    if (millis() - waitStart > 60000u) {
      Log.error("RecoveryMode: STA timeout, falling back to AP mode" CR);
      fromApp = false;
      WiFi.disconnect(true);
      // Fall through zum AP-Mode-Block
      break;
    }
    delay(1000);
  }
  ```

- **Geschätzter Aufwand:** S
- **GitHub-Issue:** [#249](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/249)
- **Fix:** ✅ Commit `c39ac8f` (2026-05-22)

---

#### CR-005-008: `/import` erlaubt arbiträres `Settings::write` und `Settings::remove` — keine Whitelist ⏳ OFFEN

- **Datei:** `src/RecoveryMode.cpp:204-219`
- **Severity:** 🟠 HIGH
- **Kategorie:** Security / Input-Validation
- **Problem:**
  - `Settings::remove(request->header("xKey"))` (Zeile 208) — User-controlled `xKey`-Header bestimmt, welche NVS-Konfiguration gelöscht wird.
  - `Settings::write(settingsKey, settingsValue)` (Zeile 219) — User-controlled Key + User-controlled Value → beliebige NVS-Manipulation.

  Auch unabhängig von CR-005-001 (Auth) ist die fehlende Whitelist eine zweite Verteidigungslinie gegen unbeabsichtigte/missbräuchliche Imports.

- **Impact:**
  Angreifer (auch versehentlich „malicious" Backup-File) kann beliebige NVS-Keys schreiben — z.B. neuen Cloud-Endpoint einsetzen, MQTT-Broker auf eigenen Server umlenken, gefakte Pitmaster-Profile mit Out-of-Range-PID-Parametern, Fakten-Frei.

- **Root Cause:**
  `Settings::write(String key, String value)` (Settings.cpp:115) ist ein Generic-Setter ohne Schema-Check.

- **Lösung:**
  - Whitelist erlaubter Keys (`kWlan`, `kPitmasters`, `kBattery`, …) — Reject mit 400 für unbekannte Keys.
  - Schema-Validation des Werts gegen ArduinoJson-Schema je Key (deserialize, validate, re-serialize).
  - Mit CR-005-001 als Auth-Wrapper kombinieren.

- **Geschätzter Aufwand:** M (~1 PT)
- **GitHub-Issue:** *(offen — kein Issue angelegt)*

---

#### CR-005-009: `getFileType()` Filename-basierte Detection — Substring-Match bypassbar ✅ GEFIXT

- **Datei:** `src/RecoveryMode.cpp:325-349`
- **Severity:** 🟠 HIGH
- **Kategorie:** Security / Logic
- **Problem:**
  ```cpp
  if ((fileName.indexOf("firmware") >= 0) && (fileName.indexOf(".bin") >= 0))
    retFileType = UploadFileType::Firmware;
  else if ((fileName.indexOf("spiffs") >= 0) && (fileName.indexOf(".bin") >= 0))
    retFileType = UploadFileType::SPIFFS;
  ```
  Substring-Matches sind trivial spielbar:
  - `"my-firmware-backup.bin.txt"` → match Firmware (indexOf prüft nicht das Suffix)
  - `"firmwarebackup.bin"` → match Firmware
  - `"spiffs-firmware.bin"` → match **Firmware** (erste Bedingung greift zuerst), nicht SPIFFS — falscher Flash-Slot.

  Kein Magic-Number-Check, keine Header-Validation auf das Binary selbst.

- **Impact:**
  - Falsche Flash-Partition (Firmware-Image in SPIFFS-Slot oder umgekehrt) → Brick nach Reboot.
  - Bypass auf Filetype-basierte Sicherheits-Annahmen (sofern jemals welche eingeführt werden).

- **Root Cause:**
  Naive Filename-Heuristik statt Header-Inspection. ESP32-Firmware hat Magic-Byte 0xE9 am Anfang — leicht prüfbar.

- **Lösung:**
  - Magic-Byte-Check beim ersten Chunk: Firmware muss mit `0xE9` starten, SPIFFS hat eigenes Magic.
  - Filename-Match nur als Hint, nicht als Trust-Anker.
  - Suffix-Check präziser (`endsWith(".bin")`).

- **Geschätzter Aufwand:** S
- **GitHub-Issue:** [#249](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/249)
- **Fix:** ✅ Commit `c39ac8f` (2026-05-22)

---

### 🟡 MEDIUM Priority Issues

#### CR-005-010: `/uploadfile` `usize`-Parameter ohne Range-/Sanity-Check ✅ GEFIXT

- **Datei:** `src/RecoveryMode.cpp:222-227`, `Update.begin(uploadFileSize)` Zeile 255, 272
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Input-Validation
- **Problem:**
  `String usize = request->arg("usize"); uploadFileSize = usize.toInt();`
  - Negative Werte → `int` → `size_t`-Cast → riesig.
  - `0` → `Update.begin(0)` schlägt fehl (Behandlung vorhanden), aber sollte sofort 400 zurückgeben.
  - `INT_MAX` → `Update.begin` allokiert Partition-Größe, kann aber endlos hängen wenn Datei nicht kommt.

- **Impact:**
  - DoS-Vektor: gestartete Update-Sessions, die nie finalisiert werden, können Flash-Layout beschädigen oder Boot-Loops triggern.

- **Lösung:**
  - Range-Check: `if (uploadFileSize < 1024 || uploadFileSize > MAX_FIRMWARE_SIZE) return 400;`
  - `MAX_FIRMWARE_SIZE` = Partition-Größe, aus `esp_partition` ableitbar.

- **Geschätzter Aufwand:** S
- **GitHub-Issue:** [#249](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/249)
- **Fix:** ✅ Commit `c39ac8f` (2026-05-22)

---

#### CR-005-011: Race auf `static String settingsKey`/`settingsValue` ⏳ OFFEN zwischen parallelen `/import`-Requests

- **Datei:** `src/RecoveryMode.cpp:51-52, 214, 217, 219`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Race Condition / Logic
- **Problem:**
  Beide Felder sind `static String` (heap-realloc bei `+=`). Wenn zwei `/import`-Requests parallel laufen (was im Recovery-AP-Setup zwar unwahrscheinlich, aber nicht verhindert ist), interleaven sie ihre Chunks → Settings-Mix. Außerdem analog CR-004-006 / CR-003-002 ein Heap-Race-Pattern.

- **Impact:**
  Settings-Korruption, undefined NVS-Zustand.

- **Lösung:**
  - Lokale Variablen pro Upload-Session (kontext-gebunden via Userdata in AsyncWebServer).
  - Oder explizite Lock (Semaphore) auf den Import-Pfad — nur ein Import gleichzeitig.

- **Geschätzter Aufwand:** S
- **GitHub-Issue:** *(offen — kein Issue angelegt)*

---

#### CR-005-012: Keine Firmware-Signaturprüfung ⏳ OFFEN — `Update`-API unterstützt RSA, ungenutzt

- **Datei:** `src/RecoveryMode.cpp:255, 272`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Security
- **Problem:**
  `Update.begin(uploadFileSize)` ohne `setMD5()` / `setSignature()`. ArduinoOTA-`Update`-API kennt RSA-Signaturen via `Update.installSignature()`, aber WLANThermo nutzt sie nicht.

- **Impact:**
  Auch mit gefixtem Auth (CR-005-001) bleibt die Firmware-Integrity ungeschützt — kompromittierte Build-Pipeline / MITM auf OTA-URL kann manipulierte Firmware ausliefern. Cloud-Update-Pfad teilt das gleiche Defizit (TASK-008).

- **Lösung:**
  - RSA-Public-Key in Firmware embedden, signierte Updates erzwingen.
  - Mindestens MD5-Check via Header.

- **Geschätzter Aufwand:** L (~3 PT — Build-Pipeline-Anpassung, Key-Management)
- **GitHub-Issue:** *(offen — kein Issue angelegt)*

---

#### CR-005-013: `nexUpload` als `static void *` — Type-Confusion-Potenzial ⏳ OFFEN

- **Datei:** `src/RecoveryMode.h:50`, `src/RecoveryMode.cpp:48, 230, 292-300`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Code-Quality / Memory Safety
- **Problem:**
  `void *nexUpload` wird zu `(ESPNexUpload *)` gecastet. Wenn der Pointer durch unerwartete Logik-Pfade (z.B. parallele Uploads, frühzeitiger Abbruch) in inkonsistentem Zustand bleibt, ergibt der nächste Cast undefined behavior.

- **Impact:**
  Crash-Potenzial bei Edge-Cases (Upload-Abbruch durch Connection-Drop). Aktuell nicht ausgelöst, aber Code-Smell.

- **Lösung:**
  `static ESPNexUpload *nexUpload` (oder besser `std::unique_ptr<ESPNexUpload>`). Forward-declare in Header, vollständig include in `.cpp`.

- **Geschätzter Aufwand:** S
- **GitHub-Issue:** *(offen — kein Issue angelegt)*

---

### 🟢 LOW Priority / Verbesserungsvorschläge

#### CR-005-014: SSID + Passwort im Klartext in Serial-Log (Privacy) ✅ GEFIXT

- **Datei:** `src/RecoveryMode.cpp:131`
- **Severity:** 🟢 LOW
- **Kategorie:** Security / Privacy
- **Problem:** `RMPRINTF("Recovery Mode starting Wifi STA. SSID: %s, PW: %s\n", wifiName, wifiPassword);` — gleiche Klasse wie CR-003-012.
- **Lösung:** PW maskieren (`***`); SSID-Logging optional + abschaltbar.
- **Aufwand:** S
- **Fix:** ✅ Commit `c39ac8f` (2026-05-22)

---

#### CR-005-015: `delay(100)` Busy-Loop am Ende von `run()` — kein Power-Saving ✅ GEFIXT

- **Datei:** `src/RecoveryMode.cpp:314-317`
- **Severity:** 🟢 LOW
- **Kategorie:** Performance
- **Problem:** `while (true) { delay(100); }` — 10×/s Wakeup, AsyncTCP läuft eh per Event. Auf Akku-Geräten signifikanter Stromverbrauch.
- **Lösung:** `vTaskDelay(portMAX_DELAY)` (FreeRTOS-Idle) oder `esp_sleep`-Light-Sleep zwischen Requests, aber AsyncTCP muss Events liefern können. Alternativ Recovery-Mode-Timeout (z.B. 30 min ohne HTTP-Aktivität → automatischer Restart in Normal-Mode).
- **Aufwand:** S
- **Fix:** ✅ Commit `c39ac8f` (2026-05-22) — `delay(100)` → `vTaskDelay(5000 ms)`

---

#### CR-005-016: Recovery-Pin-Detection ohne Debouncing ✅ GEFIXT

- **Datei:** `src/RecoveryMode.cpp:108-115`
- **Severity:** 🟢 LOW
- **Kategorie:** Robustness
- **Problem:** `digitalRead(RECOVERY_PIN)` in Loop ohne Debounce. GPIO-14-Noise könnte fehlerhafte Recovery-Mode-Triggers auslösen — relevant bei industriellen EMI-Quellen oder schlechten Tastern.
- **Lösung:** Sample-and-hold (z.B. 3× lesen, alle 3 müssen LOW sein); oder Hardware-Debounce-Cap.
- **Aufwand:** S
- **Fix:** ✅ Commit `c39ac8f` (2026-05-22) — `delay(10)` im Poll-Loop

---

#### CR-005-017: `boolean fromApp` (Arduino-Typ) statt `bool` ✅ GEFIXT

- **Datei:** `src/RecoveryMode.h:56`, `src/RecoveryMode.cpp:53, 65, 108, 123`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality
- **Problem:** Konsistenz mit modernem C++; analog CR-004-011.
- **Aufwand:** S
- **Fix:** ✅ Commit `c39ac8f` (2026-05-22)

---

#### CR-005-018: Toter Code — `RECOVERY_RESET_THRESHOLD` und auskommentierte Bedingung ✅ GEFIXT

- **Datei:** `src/RecoveryMode.cpp:37, 108`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality
- **Problem:**
  ```cpp
  #define RECOVERY_RESET_THRESHOLD 10u   // ungenutzt
  while (... && (!fromApp) /*&& (resetCounter < RECOVERY_RESET_THRESHOLD)*/) { ... }
  ```
  Reset-Counter-Logik in `run()` (Zeile 85–97) baut den Counter auf, aber er wird nirgendwo zum Trigger genutzt. Analog CR-001-014 / CR-001b-017.
- **Lösung:** Entfernen oder Feature aktivieren — Entscheidung dokumentieren.
- **Aufwand:** S
- **Fix:** ✅ Commit `c39ac8f` (2026-05-22)

---

#### CR-005-019: Memory-Leak `new AsyncWebServer(80)` ohne `delete` ⏳ OFFEN

- **Datei:** `src/RecoveryMode.cpp:152`
- **Severity:** 🟢 LOW (akademisch — Run-forever-Loop)
- **Kategorie:** Code-Quality
- **Problem:** `AsyncWebServer *webServer = new AsyncWebServer(80);` — die Funktion endet nie (infinite while), also „leakt" der Pointer nur im theoretischen Sinn. Code-Smell.
- **Lösung:** Lokales `AsyncWebServer webServer(80);` — Stack/BSS-Allokation, kein Heap.
- **Aufwand:** S
- **Hinweis:** Nicht gefixt — Stack-Allocation für AsyncWebServer-Objekt birgt unbekanntes Risiko bzgl. setup()-Task-Stack-Größe.

---

## Positive Aspekte

- ✅ `/uploadfile` benutzt korrekt `esp_timer`-One-Shot statt `delay()+restart()` (Zeile 241–246) — gewünschtes Pattern, aus dem Initial-Sonnet-Review-Fix.
- ✅ RTC-RAM-Validation via Magic-Key (`NoInitRamValidationKey`) verhindert resetCounter-Garbage nach Cold-Boot — solide defensive Programmierung.
- ✅ `ResetCounterType` mit Magic-Key + `memcpy(validationKey, ...)`-Re-Init bei Mismatch — gutes Pattern für RTC-NoInit-Felder.
- ✅ `WiFi.persistent(false) + WiFi.disconnect(true)` vor AP-/STA-Start (Zeile 120–121) — verhindert NVS-Pollution durch arduino-esp32-2.x WiFi-Stack (B39-Fix-Pattern).
- ✅ `runFromApp` benutzt deep-sleep + RTC-DATA für State-Transfer — sauberer Reset-Path.
- ✅ Multi-Chunk-Upload-Pfad in `/uploadfile` ist korrekt implementiert (`Update.write(data, len)` pro Chunk + `final` für `Update.end`) — im Gegensatz zu `/import`, das den Multi-Chunk-Bug hat.

---

## Vergleich zum vorherigen Sonnet-Review

- **Damals gefundene Issues:** Initial-Review 2026-04-25 hat RecoveryMode mitabgedeckt (B23–B35-Cluster). Konkrete Fixes laut CLAUDE.md:
  - „Recovery-Mode-Trigger aus async_tcp-Kontext: `Wlan::setRecoveryPending()` (Flag) statt direktem `RecoveryMode::runFromApp()`-Aufruf" — sauber gefixt, betrifft den Trigger-Pfad in `Wlan::update()`.
  - „Restart nach async_tcp-Response: `esp_timer` (One-Shot, ≥2 s) statt `delay() + ESP.restart()`" — **nur teilweise** gefixt: `/uploadfile` korrekt (Zeile 241), `/restart` (Zeile 168–176) nutzt weiterhin altes Pattern → **CR-005-006**.

- **Davon noch relevant:** Patterns oben sind richtig dokumentiert; an genannten Stellen kein Regress, an `/restart` noch offen.

- **Neue Findings, die Sonnet übersehen hat:**
  - **CR-005-001** — Auth-Bypass auf allen 9 Endpoints (kompletter Architektur-Issue, von Sonnet nicht aufgegriffen).
  - **CR-005-002** — `strcpy`-Buffer-Overflow auf RTC-Felder (Sonnet hat Buffer-Overflows in `Notification.cpp` gefangen, hier nicht).
  - **CR-005-003** — `/import`-Statement-Trap (klassischer C-Bug, aber Sonnet hat ihn nicht gesehen — möglicher Anhaltspunkt: nur eine Zeile, sieht „kompakt" aus).
  - **CR-005-004** — `/export`-Credential-Leak (Sonnet hat in P1-Review die Credential-Maskierung in `Mqtt`/`Cloud`/`Notification` gefixt, das `/export`-Loch übersehen — gleiche Klasse wie CR-001-005, dort auch noch offen).
  - **CR-005-007** — Endlos-Loop-Risiko in `runFromApp` (defensiver Robustheits-Punkt, eher übersehbar).
  - **CR-005-009** — Filename-basierte Filetype-Detection (Sonnet hätte das fangen können — Substring statt Suffix-Match ist ein Standard-Pattern).
  - **CR-005-012** — Fehlende Firmware-Signaturprüfung (über das übliche Review-Scope hinausgehende Härtung).

- **Sonnet-False-Positives (jetzt verworfen):** Keine identifiziert.

- **Bewertung des Sonnet-Fixes:**
  - Async-TCP-Trigger-Pattern: ✅ vollständig
  - Restart-Pattern: ⚠️ unvollständig (`/restart`-Endpoint übersehen, **CR-005-006**)
  - Auth, Buffer-Overflow, Logik-Bugs in `/import`: ❌ nicht erkannt — hier hat Opus deutlichen Mehrwert.

---

## Metriken

| Metrik | Wert |
|--------|------|
| **Gefundene Issues — CRITICAL** | 4 |
| **Gefundene Issues — HIGH** | 5 |
| **Gefundene Issues — MEDIUM** | 4 |
| **Gefundene Issues — LOW** | 6 |
| **Geschätzter Fix-Aufwand** | ~7 PT (Auth-Layer + Hardening + Bug-Fixes) |
| **Code-Qualität (1–10)** | 4/10 |
| **Wartbarkeit (1–10)** | 5/10 |

---

## Empfohlene Nächste Schritte

1. **CR-005-001 + CR-005-004 + CR-005-005 als Sicherheits-PR „Recovery Mode Hardening"** — Auth-Layer, AP-PW-Generator, Credential-Maskierung im Export. Showstopper für jede produktive Auslieferung.
2. **CR-005-002 + CR-005-003 als Bug-Fix-PR** — `strncpy`-Migration in `runFromApp`, `if`-Block-Klammern in `/import`. Beide sind triviale Fixes mit hoher Severity.
3. **CR-005-006** — `/restart`-Endpoint auf `esp_timer` umstellen; konsistent mit `/uploadfile`-Pattern. Kann mit dem Sammel-PR „WLAN/WebHandler async_tcp safety" (CR-001-003 / CR-003-001-Cluster) ko-bundled werden.
4. **CR-005-007 + CR-005-008 + CR-005-009 + CR-005-010** als „Recovery Mode Robustness"-PR (Timeout, Whitelist, Magic-Byte-Filetype, usize-Range).
5. **CR-005-012 (Firmware-Signaturprüfung)** als separater, mittelfristiger Sprint zusammen mit TASK-008 (OtaUpdate-Cloud-Pfad) — gleiche Härtung im Cloud-Update-Path nötig.
6. **Empfehlung:** RecoveryMode in einem reinen RTOS-Task pinnen (kein Pre-Boot-`run()`-Modus mehr), damit auch normale Watchdogs greifen — strukturelle Refactoring-Idee, separat zu evaluieren.

---

## Anhang: Analysierte Dateien

| Datei | LOC | Bemerkung |
|-------|-----|-----------|
| `src/RecoveryMode.cpp` | 349 | Pre-Boot-Recovery-Logik; AsyncWebServer mit 9 Endpoints; FW/SPIFFS/Nextion-Upload-Multiplex; `/import`/`/export`. |
| `src/RecoveryMode.h` | 58 | Klasse `RecoveryMode`; `RTC_DATA_ATTR`-Felder für Cross-Boot-State; `ResetCounterType`. |

---

## Tracker-Update

> Diese Zeile nach dem Review in `CODE_REVIEW_TRACKER.md` übernehmen:

```markdown
| TASK-005 | RecoveryMode | 407 | 🟠 HIGH | ✅ DONE | 4/5/4/6 | [reviews/REVIEW_TASK_005.md](./reviews/REVIEW_TASK_005.md) | RecoveryMode = unauthentifizierte RCE-Hintertür (Auth-Bypass + AP-PW-Default + /export-Credential-Leak); zwei strcpy-Overflows + /import-Logik-Bug; /restart-Pattern-Regression |
```

**Findings-Counts in Status-Übersicht aktualisieren:**
- Critical: +4
- High: +5
- Medium: +4
- Low: +6
