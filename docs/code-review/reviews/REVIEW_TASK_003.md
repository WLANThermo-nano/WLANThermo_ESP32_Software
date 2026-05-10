# Review Results — TASK-003

**Modul:** WLAN — WiFi/AsyncMQTT-Connect
**Review-Datum:** 2026-04-30
**Reviewer:** Claude Opus 4.7
**Dateien:** `src/Wlan.cpp` (461 LOC), `src/Wlan.h` (101 LOC) — gesamt ~562 LOC
**Vorheriger Review:** Sonnet 2026-04-25 (Initial-Sammelnummer B23–B35) + B39 (2026-04-27, Issue #200) · **Findings damals:** 1 dedizierter (B39 — TG1WDT_SYS_RESET) + B23–B35 modul-übergreifend

---

## Executive Summary

Der B39-Fix (`wifiModePsPending`-Flag + `WiFi.persistent(false)`) ist sauber implementiert: WiFi-Mode-Switch und PS-Konfig sind aus dem Event-Callback in `update()` (ConnectTask) verschoben — Pattern ist konsistent (`mdnsUpdatePending`, `wlanSaveConfigPending`, `recoveryPending`). Allerdings fehlt das gleiche Pattern an mehreren externen Aufruf-Pfaden: `addCredentials(force=true)` und `clearCredentials()` schreiben **NVS direkt aus async_tcp-Kontext** (gleiche Klasse von Anti-Pattern wie CR-001-003 / CR-001-006), `addCredentials()` ruft zusätzlich `WiFi.disconnect()` + `WiFi.begin()` aus dem async_tcp-Body-Handler. Eine **Race-Condition auf der statischen `String hostName`** (heap-managed, async-Schreiber gegen ConnectTask-`MDNS.begin()`-Leser) ist heap-corruption-fähig.

**Gesamtbewertung:** 🔴 **2 CRITICAL · 4 HIGH · 4 MEDIUM · 6 LOW** — der reine WiFi-Event-Pfad ist solide, aber alle externen Eintrittspunkte (`set*`/`add*`/`clear*`/`stop*`) sind nicht konsequent async-tcp-safe gemacht. Das B39-Pattern wurde lokal angewendet, nicht systematisch ausgerollt. Re-Review-Mehrwert: **klar gegeben** — Sonnet hat B39 gefunden und gefixt, die strukturell gleichen Probleme an den Setter-Pfaden aber übersehen.

---

## Review-Checkliste — angewandt

### 1. Design & Architektur
- Inkonsistentes static/non-static-Klassenmember-Pattern (s. CR-003-014). Modularisierung sonst sauber.

### 2. Logik & Korrektheit
- Race auf `wifiState` und `newWlanCredentials.ssid` (async_tcp ↔ WiFi-Event-Task ↔ ConnectTask), s. CR-003-006/-009.
- `loadConfig()` Type-Confusion-Crash, s. CR-003-004.

### 3. Performance & Effizienz
- `String`-Heap-Churn in `getMacAddress()`/`getHostName()` (CR-003-016). Sonst `✅ ok`.

### 4. ESP32 / FreeRTOS
- `WIFI_ALL_CHANNEL_SCAN` zu spät gesetzt (CR-003-005).
- `esp_wifi_*_config()` Rückgabewerte ignoriert (CR-003-010).
- `stopAllRadio()` deaktiviert Radio, lässt aber Pending-Flags aktiv (CR-003-007).
- B39-Pattern: ✅ konsistent in `onWifiConnect()`.

### 5. Security / Input-Validation
- NVS-Writes aus async_tcp (CR-003-001) — Anti-Pattern.
- `WiFi.disconnect/begin` aus async_tcp (CR-003-003).
- Hardcoded AP-Password "12345678" (CR-003-008).
- `String`-Race (CR-003-002) — heap-corruption-Risiko.

### 6. Code-Qualität
- 15+ `Serial.print/printf` statt ArduinoLog (CR-003-011), SSID im Klartext im Log (CR-003-012), auskommentierter Debug-Code (CR-003-015), Return-Values im Init ignoriert (CR-003-013).

---

## Findings

---

### 🔴 CRITICAL Issues

#### CR-003-001: `addCredentials(force=true)` und `clearCredentials()` schreiben NVS direkt aus async_tcp

- **Datei:** `src/Wlan.cpp:192–196` (`addCredentials`), `src/Wlan.cpp:149–158` (`clearCredentials`)
- **Aufrufer (async_tcp):** `src/WebHandler.cpp:732` (`addNetwork`, ruft `addCredentials(..., true)`), `src/WebHandler.cpp:365` (`handleClearWifi`)
- **Severity:** 🔴 CRITICAL
- **Kategorie:** Performance, Logic
- **Problem:**
  ```cpp
  // addCredentials (Wlan.cpp:192–196) — force-Pfad
  if (force)
  {
    saveConfig();   // → Settings::write(kWifi, json) → NVS-Write
  }

  // clearCredentials (Wlan.cpp:149–158)
  void Wlan::clearCredentials()
  {
    for (uint8_t i = 0; i < NUM_OF_WLAN_CREDENTIALS; ++i) {
      wlanCredentials[i].ssid[0] = '\0';
      wlanCredentials[i].password[0] = '\0';
    }
    saveConfig();   // → NVS-Write
  }
  ```
  Beide Funktionen werden direkt aus async_tcp-Body-Handlern getriggert (`addNetwork`, `handleClearWifi`). `Settings::write()` öffnet `Preferences`, schreibt JSON-serialisierten Inhalt in den NVS-Sektor und schließt den Namespace — dabei werden Flash-Sektoren erased & geschrieben. Auf ESP32 blockiert das den aufrufenden Task für **typisch 30–200 ms**, im Extremfall (Sektor-Garbage-Collection) auch 500+ ms.

- **Impact:**
  Identisches Anti-Pattern zu **CR-001-006** (`handleConfigReset`) und **CR-001b-003** (`setBluetooth`): async_tcp blockiert während des NVS-Writes → Response wird nicht rechtzeitig geflusht, Folgerequests stauen sich, im Worst-Case Watchdog-Reset oder TCP-Verbindungsabbruch. Im Setup-Pfad (User gibt im Recovery-Captive-Portal das WLAN-Passwort ein → `addCredentials(..., true)`) ist genau das ein realer Hotpath.

- **Root Cause:**
  B39-Fix wurde nur am WiFi-Event-Callback durchgeführt. `wlanSaveConfigPending`-Flag existiert bereits (Z. 39) und wird in `update()` korrekt verarbeitet (Z. 258–262), aber die externen Aufruf-Pfade nutzen es nicht — sie rufen `saveConfig()` direkt auf.

- **Lösung:**
  Statt `saveConfig()` direkt rufen, das bereits existierende `wlanSaveConfigPending`-Flag setzen:
  ```cpp
  // ❌ Aktuell:
  if (force) {
    saveConfig();
  }

  // ✅ Empfohlen — Pending-Flag-Pattern, identisch zu B39-Fix:
  if (force) {
    wlanSaveConfigPending = true;
  }
  ```
  Analog `clearCredentials()`:
  ```cpp
  void Wlan::clearCredentials()
  {
    for (uint8_t i = 0; i < NUM_OF_WLAN_CREDENTIALS; ++i) {
      wlanCredentials[i].ssid[0] = '\0';
      wlanCredentials[i].password[0] = '\0';
    }
    wlanSaveConfigPending = true;
  }
  ```
  Nebeneffekt: Aufrufer aus dem `SerialCmd`-Pfad (`SerialCmd.cpp:159`) sehen den Save dann erst eine Tick-Periode später — das ist hier akzeptabel (User tippt am Serial, kein zeitkritischer Pfad). Falls doch ein synchroner Save erforderlich ist, eine separate `saveConfigSync()`-Variante einführen, die explizit nur aus MainTask/SerialCmd-Kontext aufgerufen werden darf.

- **Code-Beispiel:**
  ```cpp
  // ❌ Aktuell (async_tcp blockt 30–500 ms):
  if (force) saveConfig();

  // ✅ Empfohlen (Flag → in update() pickup, ConnectTask):
  if (force) wlanSaveConfigPending = true;
  ```

- **Geschätzter Aufwand:** S (~30 min) — zwei Stellen, Flag existiert bereits.
- **Cross-Reference:** CR-001-006 (`handleConfigReset`) und CR-001b-003 (`setBluetooth`) — gleicher Anti-Pattern-Cluster, gemeinsam fixen.
- **GitHub-Issue:** [#241](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/241)
- **Fix:** ✅ Commit `d1b2c47` (2026-05-04)

---

#### CR-003-002: Race auf statische `String hostName`/`accessPointName` (potenziell Use-After-Free)

- **Datei:** `src/Wlan.cpp:35–36, 82, 84, 130–131, 371, 432–449` · `src/Wlan.h:88–89`
- **Severity:** 🔴 CRITICAL
- **Kategorie:** Logic, Security (Memory Corruption)
- **Problem:**
  `Wlan::hostName` und `Wlan::accessPointName` sind statische Arduino-`String`-Objekte. Sie werden:
  - **gelesen** von `updateMdns()` (ConnectTask via `update()`) als `hostName.c_str()` (Z. 371) — die Hand-rohe-Pointer-Referenz lebt durch alle nachfolgenden `MDNS.addService...`-Aufrufe.
  - **geschrieben** von `setHostName()` (Z. 432–439, aufgerufen aus `WebHandler.cpp:598` → async_tcp, und aus `SystemBase::loadConfig()`).
  - **geschrieben** von `loadConfig()` selbst (Z. 82, 84) — aus `init()` (MainTask) bzw. nicht relevant aktuell, aber theoretisch konkurrent.

  Arduino-`String`-Zuweisung (`this->hostName = hostName;`) gibt den alten Heap-Buffer frei und allokiert einen neuen. Wenn ConnectTask gerade in `MDNS.begin(hostName.c_str())` einen Pointer auf den alten Buffer hält und async_tcp parallel reassigniert, wird auf einen freigegebenen Heap-Block zugegriffen → Heap-Korruption oder Crash.

- **Impact:**
  Reproduzierbar bei einem Bedienfehler-Szenario: User drückt im WebUI „Hostname ändern" während gleichzeitig der `mdnsUpdatePending`-Tick durchläuft. Das Zeitfenster ist klein (MDNS.begin() läuft ~100 ms, Reassignment <1 ms), aber existiert. Heap-Korruption auf ESP32 endet typischerweise im `LoadProhibited`-Hard-Fault, kann aber auch silent zu späteren Bugs führen (verzögerte Crashes, korrupte Settings beim nächsten NVS-Write).

  Sekundär: `getHostName()` (Z. 427–430) gibt eine **Kopie** zurück — das ist heap-safe für den Caller, aber wenn der Copy-Construktor die statische Source liest und die parallel verändert wird, kann auch der Konstruktor crashen.

- **Root Cause:**
  Statische `String`-Member sind ein Refactoring-Artefakt — die Klasse benutzt teilweise Instanz-, teilweise Klassen-State (s. CR-003-014). Multi-Task-Reads/Writes auf heap-managed Strings ohne Synchronisation sind grundsätzlich unsicher.

- **Lösung:**
  Drei Optionen, in Reihenfolge der Empfehlung:
  1. **Beste Lösung:** `char hostName[HOSTNAME_MAX_LEN]` statt `String` — fixed-size Buffer, keine Heap-Reallokation, Reads/Writes sind atomarer (für strncpy-Sequenzen reicht ein einzelnes „Pending"-Flag).
  2. **Mittelweg:** `setHostName()` setzt nur ein neues `pendingHostName`-Buffer + `hostNameUpdatePending`-Flag, in `update()` wird der eigentliche `hostName` und `MDNS.begin()` synchron im ConnectTask-Kontext umgestellt.
  3. **Notfall-Fix (minimal):** `setHostName()` darf nur in `init()` (MainTask, vor Multi-Task-Phase) aufgerufen werden — `WebHandler.cpp:598` muss umgebaut werden, sodass der WebHandler nur ein Flag setzt und `update()` den Wert übernimmt.

- **Code-Beispiel:**
  ```cpp
  // ❌ Aktuell — concurrent read in updateMdns() und write in setHostName():
  void Wlan::setHostName(String hostName) {
    if (hostName.length() && hostName != this->hostName) {
      this->hostName = hostName;       // Heap-Realloc, async_tcp-Kontext!
      mdnsUpdatePending = true;
    }
  }
  void Wlan::updateMdns() {
    if (!MDNS.begin(hostName.c_str()))   // c_str() greift auf gleichen Heap zu
      ...
  }

  // ✅ Empfohlen (char[]-Buffer):
  static char hostName[HOSTNAME_MAX_LEN];

  void Wlan::setHostName(const char *newName) {
    if (newName && *newName && strcmp(newName, hostName) != 0) {
      strncpy(hostName, newName, sizeof(hostName) - 1);
      hostName[sizeof(hostName) - 1] = '\0';
      mdnsUpdatePending = true;
    }
  }
  ```

- **Geschätzter Aufwand:** M (1 PT) — Signaturen-Anpassung, Settings-Mapping, Cross-Caller-Anpassung in `WebHandler` und `SystemBase`.
- **Cross-Reference:** Gleiches Pattern bei `accessPointName` (Z. 36, 446–450) und ggf. anderen statischen `String`-Membern in der Codebasis. Bei Phase-2 / Phase-4c-Folge-Cleanup mitnehmen.
- **GitHub-Issue:** [#242](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/242)
- **Fix:** ✅ Commit `1366239` (2026-05-04)

---

### 🟠 HIGH Priority Issues

#### CR-003-003: `addCredentials()` ruft `WiFi.disconnect()` + `WiFi.begin()` direkt aus async_tcp

- **Datei:** `src/Wlan.cpp:198–209`
- **Aufrufer:** `src/WebHandler.cpp:703, 732` (`setNetwork`, `addNetwork`)
- **Severity:** 🟠 HIGH
- **Kategorie:** Performance, Logic
- **Problem:**
  ```cpp
  if (isConnected())
    WiFi.disconnect();             // Z. 199 — interne WiFi-Mutexe, ~20–100 ms

  wifiState = WifiState::AddCredentials;
  WiFi.persistent(false);
  WiFi.begin(ssid, password);      // Z. 203 — WiFi-Stack-Reinit, ~100–300 ms
  wifi_config_t wifi_cfg;
  esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg);
  wifi_cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
  esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
  ```
  Aufgerufen aus dem async_tcp-Kontext über die `setNetwork`/`addNetwork`-Handler in `WebHandler.cpp`. `WiFi.disconnect()` und `WiFi.begin()` sind blocking-Operationen (interne `xSemaphoreTake()` mit `portMAX_DELAY` für den WiFi-Mutex) — sie blockieren den async_tcp-Task für mehrere hundert Millisekunden.

  Das **ist nicht direkt B39-Pattern** (B39 betraf `WiFi.mode()`/`esp_wifi_set_ps()` im **Event-Callback**, nicht im async_tcp). Es ist aber das gleiche Klasse von Problem, das CLAUDE.md unter „Recovery-Mode-Trigger aus async_tcp-Kontext" beschreibt: „`runFromApp()` enthält `delay()` → würde async_tcp blockieren und Response-Flush verhindern."

- **Impact:**
  - Browser-Request hängt mehrere hundert Millisekunden, bis der WiFi-Reconnect die Sekundärphase erreicht und async_tcp die Response flushen kann. Bei `addNetwork` (force=true) ist der Hotpath: User klickt im Captive-Portal „Verbinden" → Browser bekommt Response zu spät → Captive-Portal-Heuristik (z.B. iOS) wirft die Verbindung weg.
  - Nach `WiFi.disconnect()` feuert ein WiFi-Event (`onWifiDisconnect`) — wenn der noch nicht abgeschlossen ist, kann das interagieren mit dem nachfolgenden `WiFi.begin()`.
  - In Kombination mit CR-003-001 (force=true → saveConfig auch noch synchron) ergibt sich auf dem `addNetwork`-Pfad eine Gesamtblockade von potenziell **>500 ms** im async_tcp-Task.

- **Root Cause:**
  Wie CR-003-001: Pending-Flag-Pattern wurde nicht systematisch auf alle Setter-Pfade angewendet.

- **Lösung:**
  `WlanCredentials newWlanCredentials` ist bereits eine „Pending-Slot"-Struktur (s. Z. 42, 122). Deren Lifecycle ausnutzen: Im async_tcp-Pfad nur Daten ablegen + Flag setzen, in `update()` (ConnectTask) den `WiFi.disconnect()`/`begin()`-Tanz durchführen.
  ```cpp
  // ❌ Aktuell (Wlan.cpp:198–209):
  if (isConnected()) WiFi.disconnect();
  wifiState = WifiState::AddCredentials;
  WiFi.persistent(false);
  WiFi.begin(ssid, password);
  // … esp_wifi_set_config …

  // ✅ Empfohlen — strikt async-frei in addCredentials():
  strcpy(newWlanCredentials.ssid, ssid);
  strcpy(newWlanCredentials.password, password);
  if (force) wlanSaveConfigPending = true;
  newCredentialsPending = true;        // neues Flag

  // In update() (ConnectTask):
  if (newCredentialsPending) {
    newCredentialsPending = false;
    if (isConnected()) WiFi.disconnect();
    wifiState = WifiState::AddCredentials;
    WiFi.persistent(false);
    WiFi.begin(newWlanCredentials.ssid, newWlanCredentials.password);
    wifi_config_t wifi_cfg;
    if (esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg) == ESP_OK) {
      wifi_cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
      esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
    }
    connectTimeout = CONNECT_TIMEOUT;
  }
  ```

- **Geschätzter Aufwand:** M (1 PT) — neues Flag, Refactoring von `addCredentials()`, Test mit Recovery-Captive-Portal.
- **Cross-Reference:** CR-003-001 (gleicher Aufruf-Pfad, gemeinsam fixen).
- **GitHub-Issue:** [#243](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/243)
- **Fix:** ✅ Commit `74230ce` (2026-05-04)

---

#### CR-003-004: `loadConfig()` Type-Confusion-Crash bei JSON-Type-Mismatch

- **Datei:** `src/Wlan.cpp:81–85, 93–94`
- **Severity:** 🟠 HIGH
- **Kategorie:** Logic, Security
- **Problem:**
  ```cpp
  if (json.containsKey("host"))
    hostName = json["host"].as<const char*>();        // Z. 82
  if (json.containsKey("ap"))
    accessPointName = json["ap"].as<const char*>();   // Z. 84
  // …
  const char *ssid = wifiEntry["SSID"].as<const char*>();
  const char *pass = wifiEntry["PASS"].as<const char*>();
  if (!ssid || !pass || strlen(ssid) >= ...)          // Z. 95 — NULL-Check vorhanden
  ```
  Die Top-Level-Felder `host`/`ap` haben **keinen NULL-Check**: `containsKey("host")` gibt nur an, dass der Schlüssel existiert — wenn der Wert kein String ist (z.B. `"host": 1234` oder `"host": null` durch eine korrupte NVS-Migration), gibt `as<const char*>()` `nullptr` zurück. Die Zuweisung `hostName = nullptr;` an einen Arduino-`String` führt zu undefiniertem Verhalten / Crash.

  Innerhalb der Wifi-Liste ist der NULL-Check vorhanden (Z. 95) — der Type-Check fehlt also nur am Top-Level.

- **Impact:**
  - Aktueller Trigger-Pfad: NVS-Inhalt vor dem Phase-4c-Migrations-Lauf könnte numerische `host`-Werte enthalten haben (kein bekannter Fall, aber möglich).
  - Future-Trigger: Korrupte NVS-Settings nach Update / Brown-Out / Settings-Schema-Änderung.
  - Resultat: Boot-Crash in `Wlan::init()` → Bricked-Gerät bis Recovery-Mode.

- **Root Cause:**
  ArduinoJson v7 `as<const char*>()` ist explizit dokumentiert: gibt `nullptr` bei Type-Mismatch oder Null-Wert zurück. Die SKILL.md weist explizit darauf hin: „`.as<const char*>()` (kann NULL sein — prüfen!)".

- **Lösung:**
  ```cpp
  // ✅ Empfohlen:
  if (json.containsKey("host")) {
    const char *h = json["host"].as<const char*>();
    if (h) hostName = h;
  }
  if (json.containsKey("ap")) {
    const char *a = json["ap"].as<const char*>();
    if (a) accessPointName = a;
  }
  ```
  Alternativ ArduinoJson-v7-idiomatisch via Default-Wert:
  ```cpp
  const char *h = json["host"] | "";
  if (*h) hostName = h;
  ```

- **Geschätzter Aufwand:** S (~10 min).
- **GitHub-Issue:** [#242](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/242)
- **Fix:** ✅ Commit `1366239` (2026-05-04) — mitgefixt durch CR-003-002 (`char[]`-Migration)

---

#### CR-003-005: `WIFI_ALL_CHANNEL_SCAN` wird **nach** `WiFi.begin()` gesetzt — wirkt nicht auf laufenden Verbindungsversuch

- **Datei:** `src/Wlan.cpp:202–207, 341–346`
- **Severity:** 🟠 HIGH
- **Kategorie:** Logic, Performance
- **Problem:**
  ```cpp
  WiFi.begin(ssid, password);                         // Z. 203 / 342
  wifi_config_t wifi_cfg;
  esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg);        // Z. 205 / 344
  wifi_cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
  esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);        // Z. 207 / 346
  ```
  `WiFi.begin()` startet bereits den Scan (typischerweise Fast-Scan: nur der zuletzt verwendete Kanal). Erst danach wird `WIFI_ALL_CHANNEL_SCAN` gesetzt. Das bedeutet:
  - **Erster Verbindungsversuch:** läuft mit Default-`WIFI_FAST_SCAN`, sucht nur einen Kanal → wenn AP nicht der zuletzt verwendete ist (z.B. nach Roaming, AP-Channel-Hop), schlägt der erste Versuch fehl.
  - **Folgende Versuche:** würden mit `WIFI_ALL_CHANNEL_SCAN` laufen — aber `WiFi.begin()` wird in `connectToKnownStations()` bei jedem Reconnect erneut aufgerufen, und die Reihenfolge ist immer `begin()` → `set_config()`.

  Der CLAUDE.md-Hinweis zu B23–B35 erwähnt: „WiFi fix (`WIFI_ALL_CHANNEL_SCAN` — connect to strongest AP) is set at runtime in `src/Wlan.cpp` after each `WiFi.begin()` call". Die Intention war richtig, die Reihenfolge ist aber ineffektiv.

- **Impact:**
  Verlängerte Initial-Connection-Time bei Mehr-AP-Setups (Mesh, Repeater) und nach AP-Channel-Wechsel. Im Worst-Case verbindet sich das Gerät mit dem schwächsten AP (Cached-AP) statt mit dem stärksten — exakt das, was `WIFI_ALL_CHANNEL_SCAN` verhindern soll.

- **Root Cause:**
  Übernommene Reihenfolge aus historischer arduino-esp32 1.x-Workaround. ESP-IDF 4.4.x bietet `wifi_sta_config_t.scan_method` direkt — sollte vor `begin()` gesetzt werden.

- **Lösung:**
  ```cpp
  // ✅ Empfohlen — Config VOR begin() setzen:
  wifi_config_t wifi_cfg;
  esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg);
  wifi_cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
  wifi_cfg.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;  // optional: stärkster AP zuerst
  esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
  WiFi.persistent(false);
  WiFi.begin(ssid, password);
  ```
  Oder den Wlan-State in `init()` einmalig auf All-Channel-Scan + Sort-by-Signal konfigurieren — `esp_wifi_set_config()` persistiert bis zum Reboot.

- **Geschätzter Aufwand:** S (~30 min, plus Verifikations-Test mit zwei APs gleichen SSID).
- **GitHub-Issue:** [#244](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/244)
- **Fix:** ✅ Commit `a1797e7` (2026-05-04)

---

#### CR-003-006: Race auf `wifiState` zwischen async_tcp (`addCredentials`/`setStopRequest`) und ConnectTask (`update`)

- **Datei:** `src/Wlan.cpp:201, 459` (Writes aus async_tcp) vs. Z. 276–296 (Read in `update()`); auch `DisplayOled.cpp:721` (Read aus MainTask)
- **Severity:** 🟠 HIGH
- **Kategorie:** Logic
- **Problem:**
  `wifiState` ist ein Instanz-`enum class WifiState`, atomar lesbar/schreibbar (≤4 Byte) — also nicht „Word-Tearing"-anfällig. Aber:
  - **Logik-Race:** `addCredentials()` schreibt `wifiState = WifiState::AddCredentials` **nach** `WiFi.begin()` (Z. 201–203). Wenn `update()` zwischen `WiFi.begin()` und `wifiState = ...` läuft, sieht die State-Machine den **alten** Zustand und ruft ggf. erneut `connectToKnownStations()` → konkurrierende `WiFi.begin()`-Aufrufe.
  - **Same-Class für `setStopRequest()`:** schreibt `wifiState = WifiState::StoppingRadio` aus async_tcp — ConnectTask kann während `update()` mitten im Switch-Statement vom State Y in den Read-Pfad von State StoppingRadio wechseln. Hier weniger kritisch (StopRadio idempotent), aber strukturell fragwürdig.
  - Reads aus MainTask (`DisplayOled.cpp:721`) sind purely-informational, OK.

- **Impact:**
  Im Realbetrieb: User legt im WebUI ein neues Netz an → Zeitfenster ~10–100 µs zwischen `WiFi.begin()` und `wifiState = AddCredentials`. ConnectTask läuft 1× pro Sekunde (gemäß `update()`-Cadence im ConnectTask) → Wahrscheinlichkeit der Race ist niedrig, aber nicht null. Konsequenz wäre ein zusätzlicher `WiFi.begin()` aus `connectToKnownStations()`, der die User-Credentials überstößt.

- **Root Cause:**
  `wifiState`-Schreiber außerhalb der „natürlichen" Domäne (ConnectTask). In Kombination mit CR-003-003 (Fix verlagert `WiFi.begin()` ohnehin in den ConnectTask) löst sich diese Race auf.

- **Lösung:**
  Mit CR-003-003-Fix verschwindet der `addCredentials`-Race-Teil. Für `setStopRequest()` reicht ein zusätzliches `stopRequestPending`-Flag, das in `update()` zur State-Transition führt:
  ```cpp
  // ✅ Empfohlen — alle wifiState-Writes nur aus update() (ConnectTask):
  void Wlan::setStopRequest() {
    stopRequestPending = true;
  }
  void Wlan::update() {
    if (stopRequestPending) {
      stopRequestPending = false;
      wifiState = WifiState::StoppingRadio;
    }
    // …
  }
  ```

- **Geschätzter Aufwand:** S (~30 min, zusammen mit CR-003-003).

---

### 🟡 MEDIUM Priority Issues

#### CR-003-007: `stopAllRadio()` lässt `mdnsUpdatePending`/`wifiModePsPending` aktiv → MDNS/PS auf gestoppter Radio

- **Datei:** `src/Wlan.cpp:360–367`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Logic
- **Problem:**
  ```cpp
  void Wlan::stopAllRadio() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    btStop();
    wifiState = WifiState::Stopped;
  }
  ```
  Wenn `setStopRequest()` aufgerufen wird, während noch `mdnsUpdatePending`, `wifiModePsPending` oder `wlanSaveConfigPending` gesetzt sind, würde im **nächsten** `update()`-Tick `wifiModePsPending` zuerst geprüft (Z. 245–250) — und dann auf der bereits gestoppten Radio:
  - `WiFi.mode(WIFI_STA)` re-aktiviert STA → undoes `WIFI_OFF`
  - `esp_wifi_set_ps()` versucht PS auf gestoppter Radio
  Reihenfolge im aktuellen Code: erst Pending-Flags abarbeiten, dann switch(wifiState) → Pending-Flags „gewinnen" gegen `StoppingRadio`-State.

  Auch: `MDNS.begin()` auf gestoppter Radio gibt zwar einen Fehler zurück (geloggt), aber MDNS-Stack bleibt in undefiniertem Zustand.

- **Impact:**
  Im Pfad „User macht im WebUI Power-Off-Toggle, gleichzeitig läuft MDNS-Pending-Trigger durch": WiFi wird kurz aus- und wieder angeschaltet — kein Crash, aber sichtbares Flicker in der Konnektivität.

- **Root Cause:**
  Pending-Flags und `StoppingRadio`-State sind zwei orthogonale Mechanismen ohne Kollisions-Schutz.

- **Lösung:**
  In `update()` zuerst `wifiState == Stopped/StoppingRadio` prüfen und Pending-Flags entweder ignorieren oder vor `stopAllRadio()` zurücksetzen:
  ```cpp
  void Wlan::stopAllRadio() {
    mdnsUpdatePending = false;
    wifiModePsPending = false;
    wlanSaveConfigPending = false;   // optional: vor Stop einmal speichern
    recoveryPending = false;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    btStop();
    wifiState = WifiState::Stopped;
  }
  ```

- **Geschätzter Aufwand:** S (~10 min).

---

#### CR-003-008: Hardcoded AP-Password "12345678" — bekanntes Default

- **Datei:** `src/Wlan.cpp:31, 66`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Security
- **Problem:**
  ```cpp
  #define APPASSWORD "12345678"
  ...
  WiFi.softAP(this->accessPointName.c_str(), APPASSWORD, 5);
  ```
  Das WPA2-Passwort des Recovery/Soft-AP-Modus ist im Klartext im Source und zudem trivial (8× Ziffer). Jeder, der die Firmware kennt (Open-Source, in Reichweite), kommt in den Recovery-AP und damit in den Setup-Pfad — wo man Cloud-Credentials, Pushover-Tokens etc. auslesen oder per `setServerAPI` (CR-001b-001 — RCE-Vektor!) flashen kann.

- **Impact:**
  Niedrige Hürde für lokale Angriffe in der Reichweite des Geräts. Insbesondere wenn das Gerät dauerhaft im Soft-AP-Modus läuft (z.B. nach WiFi-Disconnect mehrere Minuten lang), wird der AP regelmäßig sichtbar.

- **Root Cause:**
  Default-Password aus Usability-Gründen (User muss es nicht eingeben). Bekanntes Trade-off, aber sollte gegen Phase-3-Setup-Wizard-Sicherung abgewogen werden.

- **Lösung:**
  Optionen:
  1. **Per-Device-Random:** AP-Password aus `DeviceId` ableiten (z.B. SHA256 von MAC-Adresse + Salt, ersten 8 Stellen base32). Im UI „Setup-Anleitung" das Password aus dem QR-Code (auf dem Gerät) entnehmen.
  2. **Off-by-default:** AP-Modus nur für ≤5 Minuten nach Boot ohne STA-Connect aktiv → reduziert Angriffsfenster.
  3. **AP nur bei explizitem Trigger:** User-Button (lange drücken) aktiviert Soft-AP. Erfordert UX-Anpassung.

- **Geschätzter Aufwand:** M (1 PT für Per-Device-Random + UI-Anzeige).
- **Hinweis:** Stark mit Phase 3 (Flutter-App, App-Store) verknüpft — Setup-Wizard sollte da ohnehin neu gedacht werden.

---

#### CR-003-009: `onWifiConnect()` Race auf `newWlanCredentials.ssid`

- **Datei:** `src/Wlan.cpp:386–400` vs. `src/Wlan.cpp:110–125` (`saveConfig`) und `src/Wlan.cpp:188–193` (`addCredentials`)
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Logic
- **Problem:**
  ```cpp
  // onWifiConnect (WiFi-Event-Task):
  if (WiFi.SSID() == newWlanCredentials.ssid) {        // Z. 393 — Read
    wlanSaveConfigPending = true;
  }

  // addCredentials (async_tcp):
  strcpy(newWlanCredentials.ssid, ssid);               // Z. 190 — Multi-Byte-Write

  // saveConfig (ConnectTask via update()):
  newWlanCredentials.ssid[0] = '\0';                   // Z. 122 — Write nach Save
  ```
  Drei Tasks schreiben/lesen `newWlanCredentials.ssid` ohne Synchronisation. Konkrete Race-Szenarien:
  - User legt Netz an (async_tcp `strcpy`), während WiFi-Event gerade einen vorherigen STA-Connect signalisiert → `onWifiConnect` liest halben SSID.
  - Save läuft (`ssid[0]='\0'`), während WiFi-Event den Vergleich macht → False-Negative-Match.

- **Impact:**
  - Bei False-Negative wird `wlanSaveConfigPending` nicht gesetzt → die neuen Credentials werden nicht persistiert. Nach Reboot ist das Netz weg, User muss erneut anlegen. Ärgerlich, aber nicht datenverlierend (Settings sind nicht korrupt, nur unvollständig).
  - Heap-Korruption ist hier ausgeschlossen (fixed-size Array, kein Heap).

- **Root Cause:**
  Save-Logik räumt `newWlanCredentials` als Indikator für „bereits persistiert" — Indikator-Variable wird konkurrent geschrieben.

- **Lösung:**
  Ein dediziertes Flag `newCredentialsValid` einführen, das nur in `update()` (ConnectTask) gesetzt/zurückgesetzt wird:
  ```cpp
  if (newCredentialsValid && WiFi.SSID() == newWlanCredentials.ssid) {
    wlanSaveConfigPending = true;
  }
  ```
  In Kombination mit CR-003-003-Fix (`addCredentials` setzt nur Pending-Flag, eigentliche Logik in `update()`) löst sich auch diese Race auf.

- **Geschätzter Aufwand:** S (~20 min, zusammen mit CR-003-003).

---

#### CR-003-010: `esp_wifi_get_config()` / `esp_wifi_set_config()` Rückgabewerte ignoriert

- **Datei:** `src/Wlan.cpp:205, 207, 344, 346`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Logic
- **Problem:**
  ```cpp
  esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg);   // Returns esp_err_t — ignoriert
  wifi_cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
  esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);   // Returns esp_err_t — ignoriert
  ```
  Beide Aufrufe können `ESP_ERR_WIFI_NOT_INIT`, `ESP_ERR_WIFI_IF` oder `ESP_ERR_INVALID_ARG` zurückgeben. Wenn `get_config()` fehlschlägt, enthält `wifi_cfg` undefinierten Stack-Inhalt — und `set_config()` schreibt diesen zurück. Resultat: WiFi-Stack-Konfiguration korrupt.

- **Impact:**
  In normalen Boot-Pfaden tritt das nicht auf (WiFi ist initialisiert). Bei Recovery-Pfaden oder kurz nach `stopAllRadio()` aber möglich → silent corruption der STA-Config.

- **Root Cause:**
  Fehlende Defensive-Programmierung. Pattern existiert in der Codebasis (z.B. `SystemBase.cpp` für `esp_pm_configure()` mit Error-Check), wurde hier nicht angewendet.

- **Lösung:**
  ```cpp
  if (esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg) == ESP_OK) {
    wifi_cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
    if (err != ESP_OK)
      Log.warning("esp_wifi_set_config failed: %d" CR, err);
  } else {
    Log.warning("esp_wifi_get_config failed" CR);
  }
  ```

- **Geschätzter Aufwand:** S (~10 min).

---

### 🟢 LOW Priority / Verbesserungsvorschläge

#### CR-003-011: 15+ `Serial.print/printf` statt ArduinoLog

- **Datei:** `src/Wlan.cpp:69, 70, 97, 103, 142, 184, 189, 347, 373, 388, 389, 404, 410`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Qualität
- **Problem:**
  CLAUDE.md: „Logging uses `ArduinoLog` macros (`Log.verbose`, `Log.notice`, `Log.error`) — not `Serial.print`." Dieses Modul nutzt fast ausschließlich `Serial.printf` für Debug-Output. Nur Z. 391 nutzt `Log.notice`.
- **Lösung:** Schrittweise Konversion. Beispiel:
  ```cpp
  // ❌
  Serial.printf("STA: %s\n", WiFi.SSID().c_str());
  // ✅
  Log.notice("STA: %s" CR, WiFi.SSID().c_str());
  ```
- **Aufwand:** S (~30 min, mechanisches Refactoring).

---

#### CR-003-012: SSID im Klartext im Serial-Log (Privacy)

- **Datei:** `src/Wlan.cpp:103, 142, 189, 347, 388`
- **Severity:** 🟢 LOW
- **Kategorie:** Security (Defense in Depth)
- **Problem:**
  Während Passwords brav als `***` ausgegeben werden, wandert die SSID immer im Klartext über die serielle Konsole (auch in Production-Builds, da kein `#ifdef DEBUG`-Guard). Bei seriellem Log-Mitschnitt (z.B. Issue-Reports) leakt der Heim-Netz-Name.
- **Lösung:** Entweder SSID hinter `#ifdef DEBUG_WLAN` verstecken, oder zumindest in `Log.verbose` (nicht Production-Default) statt im immer aktiven `Serial.printf`.
- **Aufwand:** S (~10 min).

---

#### CR-003-013: `WiFi.softAPConfig()`/`WiFi.softAP()` Return-Werte ignoriert in `init()`

- **Datei:** `src/Wlan.cpp:65–66`
- **Severity:** 🟢 LOW
- **Kategorie:** Logic
- **Problem:**
  ```cpp
  WiFi.softAPConfig(local_IP, gateway, subnet);   // returns bool, ignoriert
  WiFi.softAP(this->accessPointName.c_str(), APPASSWORD, 5);  // returns bool, ignoriert
  ```
  Wenn der Soft-AP nicht startet (Brown-Out, RF-Hardware-Fehler), läuft das Gerät stillschweigend ohne Recovery-Pfad weiter.
- **Lösung:** Return-Werte loggen, ggf. Retry oder Fail-Safe.
- **Aufwand:** S (~10 min).

---

#### CR-003-014: Inkonsistentes static/non-static-Pattern

- **Datei:** `src/Wlan.h:74–96`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Qualität, Architecture
- **Problem:**
  Die Klasse `Wlan` mischt statische Klassen-Member (`hostName`, `wlanCredentials`, `mdnsUpdatePending`, …) und Instanz-Member (`rssi`, `connectTimeout`, `wifiState`). Aufrufer verstehen nicht intuitiv, ob `gSystem->wlan.foo()` oder `Wlan::foo()` korrekt ist (bei `clearCredentials()` z.B. via `gSystem->wlan.clearCredentials()`, obwohl sie statisch ist).
- **Lösung:** Entweder vollständig instance-based (alle statischen Member zu Instanz-Member machen, da es nur ein `Wlan`-Objekt im `gSystem` gibt) oder vollständig static (Singleton-Pattern). Empfehlung: instance-based, weil `gSystem`-Pattern bereits den Singleton-Zugriff regelt.
- **Aufwand:** M (~2–3 h Cleanup, niedrige Priorität).

---

#### CR-003-015: Auskommentierter Debug-Code

- **Datei:** `src/Wlan.cpp:274, 362`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Qualität
- **Problem:**
  Z. 274: `//Serial.printf("Wlan::update: wifiState = %d\n", wifiState);`
  Z. 362: `//Serial.println("** Stopping WiFi+BT");`
  Toter Kommentar-Code. CLAUDE.md: „Don't write multi-line comment blocks", auskommentierter Code ist generell zu entfernen.
- **Lösung:** Löschen oder hinter `Log.verbose` setzen.
- **Aufwand:** S (~2 min).

---

#### CR-003-016: `getMacAddress()`/`getHostName()` allokieren `String` pro Aufruf

- **Datei:** `src/Wlan.cpp:413–420, 427–430, 441–444`
- **Severity:** 🟢 LOW
- **Kategorie:** Performance
- **Problem:**
  ```cpp
  String Wlan::getMacAddress() {
    char macStr[18] = {0};
    WiFi.macAddress(mac);
    sprintf(macStr, "%02X:%02X:...", ...);
    return String(macStr);                    // Heap-Allokation pro Aufruf
  }
  ```
  `getMacAddress()` wird aus `updateMdns()` (in `MDNS.addServiceTxt`-Aufruf) und potenziell aus weiteren Stellen aufgerufen → jedes Mal ~18 Byte Heap-Allokation. Im normalen Pfad einmal pro `updateMdns`-Trigger, also nicht in heißem Loop, aber dennoch unnötig.
- **Lösung:** Statischer Buffer oder MAC einmalig in `init()` formatieren:
  ```cpp
  static char macStr[18];   // einmal in init() befüllen
  const char *Wlan::getMacAddress() { return macStr; }
  ```
- **Aufwand:** S (~15 min).

---

## Positive Aspekte

- ✅ **B39-Fix-Pattern sauber implementiert** — alle WiFi-Mode/PS-Operationen in `onWifiConnect()` als Pending-Flag deferred (Z. 395, 398, 399). Die Hand-Translation aus dem ursprünglichen Crash-Report ist exakt das richtige Pattern.
- ✅ **`WiFi.persistent(false)` redundant defensiv** — sowohl in `init()` (Z. 59) als auch vor jedem `WiFi.begin()` (Z. 202, 341) — verhindert versehentliche NVS-Persistierung der SSID/Password durch ESP-IDF.
- ✅ **`recoveryPending`-Flag korrekt von WebHandler genutzt** — `WebHandler.cpp:379` ruft `Wlan::setRecoveryPending()` (Inline-Setter, async-safe), nicht direkt `RecoveryMode::runFromApp()`.
- ✅ **Längen-Validierung in `loadConfig()` Wifi-Liste vorhanden** (Z. 95) — `>=`-Check ist semantisch korrekt (Buffer-Größe inkl. Null-Term).
- ✅ **`wlanSaveConfigPending`-Flag existiert und wird korrekt vom WiFi-Event-Pfad benutzt** (Z. 395, abgearbeitet in `update()` Z. 258–262) — Infrastruktur ist da, fehlt nur der Roll-out auf alle Setter-Pfade.
- ✅ **`numOfAPClients()` defensiv** (`isAP()`-Check vor `softAPgetStationNum()`).

---

## Vergleich zum vorherigen Sonnet-Review

- **Damals gefundene Issues (Wlan-bezogen):** B39 — TG1WDT_SYS_RESET nach WiFi-Connect auf nanoV3 (Issue #200, gefixt 2026-04-27 via `wifiModePsPending`-Flag + `WiFi.persistent(false)`). B23–B35 sind modul-übergreifend, das Modul-Wlan-spezifische daraus war: AP-Password-Vermerkung im Plain-Source (jetzt CR-003-008), Pending-Flag-Pattern.
- **Davon noch relevant:** B39 ist sauber gefixt — nicht reopen. AP-Password-Issue (CR-003-008) bleibt offen, aber als bekanntes Trade-off dokumentiert.
- **Neue Findings, die Sonnet übersehen hat:**
  - **CR-003-001** (NVS aus async_tcp via `addCredentials`/`clearCredentials`) — die strukturell gleiche Klasse wie B39, aber an anderem Aufrufpfad. Sonnet hat das Event-Callback-Pattern korrekt erkannt, aber nicht systematisch nach allen ähnlichen Pfaden gesucht.
  - **CR-003-002** (`String hostName` Heap-Race) — Heap-managed-Multi-Task-Patterns sind ein bekannter Sonnet-Blindfleck (auch in TASK-001b kein Vorkommen erkannt, dort aber kein Trigger).
  - **CR-003-003** (`WiFi.disconnect/begin` aus async_tcp) — gleicher Aufrufpfad-Cluster wie CR-003-001.
  - **CR-003-004** (JSON Type-Confusion in `loadConfig`) — Top-Level-Felder ohne NULL-Check.
  - **CR-003-005** (`WIFI_ALL_CHANNEL_SCAN` Reihenfolge) — der CLAUDE.md-Eintrag zur Phase-1-Migration beschreibt den Workaround, ohne die Reihenfolge zu hinterfragen.
- **Sonnet-False-Positives (jetzt verworfen):** keine.
- **Bewertung des Sonnet-Fixes (B39):** ✅ **vollständig** — der Fix ist im Code sauber sichtbar (Z. 395–399 Event-Callback nur Flag-Setting, Z. 245–250 Pickup in ConnectTask). Keine Regression.

---

## Metriken

| Metrik | Wert |
|--------|------|
| **Gefundene Issues — CRITICAL** | 2 |
| **Gefundene Issues — HIGH** | 4 |
| **Gefundene Issues — MEDIUM** | 4 |
| **Gefundene Issues — LOW** | 6 |
| **Geschätzter Fix-Aufwand** | ~3–4 PT (CR-003-001/-003/-006 zusammen 1 PT, CR-003-002 1 PT, CR-003-008 1 PT, Rest LOW/MEDIUM in Sammel-PR) |
| **Code-Qualität (1–10)** | **7/10** — saubere State-Machine, B39-Fix exemplarisch, aber Setter-Pfade nicht systematisch async-safe |
| **Wartbarkeit (1–10)** | **6/10** — inkonsistentes static/instance-Pattern, Mix von ArduinoLog und Serial, ältere Debug-Kommentare |

---

## Empfohlene Nächste Schritte

1. **CR-003-001 + CR-003-003 + CR-003-006 als Cluster fixen** (~1 PT) — alle drei drehen sich um „async_tcp-Aufrufe von Wlan-Settern". Pending-Flag-Pattern konsequent ausrollen, `newCredentialsPending` einführen. Zusammen mit CR-001-006 (`handleConfigReset`) und CR-001b-003 (`setBluetooth`) als Sammel-PR „WLAN/WebHandler async_tcp safety".
2. **CR-003-002 (`String hostName` Race)** parallel zu Phase-2-Vorbereitung (~1 PT) — Migration zu `char[]`-Buffer, beim Phase-4c-Sweep eigentlich bereits angepackt sein müssen. Zusammen mit Cleanup der statischen vs. Instanz-Member (CR-003-014) angehen.
3. **CR-003-004 (JSON Type-Confusion)** als Hotfix in `develop` (~10 min) — risikoarm, sofort.
4. **CR-003-005 (`WIFI_ALL_CHANNEL_SCAN` Reihenfolge)** — Hardware-Test vor Merge: zwei APs gleicher SSID in unterschiedlichen Kanälen, nach Reboot muss Stärkster gewählt werden.
5. **CR-003-008 (Default-AP-Password)** mit Phase 3 (Flutter-Setup-Wizard) bündeln. Bis dahin nicht im Hotfix-Sprint.

> **Kein Split nötig:** Mit ~562 LOC liegt der Task deutlich unter der 800-LOC-Schwelle. Der Hinweis in CODE_REVIEW_TASKS.md erwähnt keinen Split.

---

## Anhang: Analysierte Dateien

| Datei | LOC | Bemerkung |
|-------|-----|-----------|
| `src/Wlan.cpp` | 461 | Hauptdatei, alle Findings hier verortet |
| `src/Wlan.h` | 101 | Klassen-Definition; CR-003-014 betrifft static/instance-Mix |

Cross-referenzierte Aufrufer (nicht im Review-Scope, aber für Context gelesen):
- `src/WebHandler.cpp:315, 365, 379, 598, 605, 703, 732` — async_tcp-Aufrufe
- `src/SerialCmd.cpp:159` — `clearCredentials()`-Aufruf aus MainTask
- `src/system/SystemBase.cpp:210–211` — `setHostName`/`setAccessPointName` in Boot-Pfad
- `src/display/DisplayOled.cpp:719–739` — `getWifiState()` aus MainTask

---

## Tracker-Update

> Diese Zeilen nach dem Review in `CODE_REVIEW_TRACKER.md` übernehmen:

```markdown
| TASK-003 | WLAN | 562 | 🔴 CRITICAL | ✅ DONE | 2/4/4/6 | [reviews/REVIEW_TASK_003.md](./reviews/REVIEW_TASK_003.md) | B39 sauber gefixt; gleiche async_tcp-Patterns aber an Setter-Pfaden offen (NVS, WiFi.begin), `String`-Heap-Race |
```

**Findings-Counts in Status-Übersicht aktualisieren:**
- Critical: +2
- High: +4
- Medium: +4
- Low: +6
