# Review Results — TASK-004

**Modul:** WServer — HTTP-Server-Setup
**Review-Datum:** 2026-05-05
**Reviewer:** Claude Opus 4.7
**Dateien:** 2 Dateien (`src/WServer.cpp` 251 LOC, `src/WServer.h` 47 LOC), ~298 Lines of Code
**Vorheriger Review:** keiner (📝 NEU)

---

## Executive Summary

`WServer` initialisiert den AsyncWebServer und registriert eine Reihe von „Wartungs-Endpoints" (`/info`, `/setbattmin`, `/stop`, `/restart`, `/newtoken`, `/clientlog`, `/settestmode`, `/rr`) **direkt** über `webServer.on()`. Diese Pfade umgehen die Auth-Middleware in `NanoWebHandler::canHandle()` (`WebHandler.cpp:124,181`) komplett — auch bei gesetztem Admin-Passwort sind sie für jeden im LAN aufrufbar. Mehrere dieser Handler führen zusätzlich direkte NVS-Writes (`battery->saveConfig()`, `pitmasters.saveConfig()`, `cloud.saveConfig()`, `WServer::saveConfig()`) aus dem async_tcp-Kontext aus — gleiches Anti-Pattern, das in CR-001b-012 (Cloud) und CR-003-001 (Wlan) bereits dokumentiert wurde, hier an mindestens vier weiteren Stellen offen.

**Gesamtbewertung:**
- 🔴 1 CRITICAL Auth-Bypass auf destruktiven Endpoints (Reboot, Pitmaster-Stop, Token-Rotation), 6 HIGH (4× NVS-Anti-Pattern, Info-Disclosure, String-Race), 3 MEDIUM, 7 LOW. WServer ist neben WebHandler die zweite Achillesferse der HTTP-Schicht. Sammel-Fix mit dem WLAN/WebHandler-async_tcp-PR sinnvoll.

---

## Review-Checkliste — angewandt

### 1. Design & Architektur
- Auth-Konzept inkonsistent: Middleware nur in `NanoWebHandler` (`WebHandler.cpp`), nicht im AsyncWebServer-Layer → alle direkt registrierten Endpoints in `WServer.cpp` sind ungeschützt. Architekturfehler — gehört in eine zentrale Filter-/Auth-Schicht. Siehe **CR-004-001**.

### 2. Logik & Korrektheit
- `loadConfig()` schluckt JSON-Type-Mismatch silent (NULL aus `as<const char*>()` → leeres Passwort statt erwartetem). **CR-004-010**.
- `/setbattmin` hat keine Untergrenze — wiederholtes Aufrufen kann `min` ins Negative treiben. **CR-004-009**.

### 3. Performance & Effizienz
- `/info` chained 8+ Arduino-`String`-Konkatenationen → Heap-Fragmentierung pro Request. **CR-004-008**.

### 4. ESP32 / FreeRTOS
- 4 Endpoints rufen Settings-`saveConfig()` direkt aus async_tcp-Callback → NVS-Write blockiert Core 1 für Flash-Write-Dauer. **CR-004-002, -003, -004, -007**.
- `requireAuth()` / `getPassword()` lesen unsynchronisiert aus statischem `String password`, der von `setPassword()` aus einem anderen async_tcp-Request geschrieben werden kann (Heap-Realloc). Gleiches Pattern wie CR-003-002. **CR-004-006**.

### 5. Security / Input-Validation
- **CRITICAL — CR-004-001:** Mehrere state-modifying Endpoints umgehen die Auth-Middleware komplett. `ON_STA_FILTER` ≠ Authentication.
- `/info` exponiert SSID, MAC, Serial, Item, Heap, SPIFFS, FlashSize ohne Auth → Device-Fingerprinting. **CR-004-005**.
- `/setbattmin` ohne Range-Check / ohne Auth → triviale State-Manipulation. **CR-004-009**.

### 6. Code-Qualität
- `boolean` (Arduino) statt `bool` (CR-004-011), hardkodierter `username = "admin"` (CR-004-012), TODO-/auskommentierter Code (CR-004-013), `IPRINTPLN` statt `Log.notice` (CR-004-014), `WEB_VUE_ROUTER_PATHS_MAX` ohne Compile-Time-Konsistenz-Check (CR-004-015), Endpoints ohne HTTP-Method-Filter (CR-004-016), linearer O(n)-Lookup `vueRouterPaths` (CR-004-017, akzeptabel — nur dokumentiert).

---

## Findings

> Konvention: `CR-004-NNN`, fortlaufend (Critical → High → Medium → Low).

---

### 🔴 CRITICAL Issues (Blocker)

#### CR-004-001: Auth-Middleware umgangen — destruktive Endpoints unauthentifiziert erreichbar

- **Datei:** `src/WServer.cpp:69-154` (alle direkt via `webServer.on()` registrierten Endpoints)
- **Severity:** 🔴 CRITICAL
- **Kategorie:** Security
- **Problem:**
  Die Auth-Middleware ist als `request->authenticate(...)`-Aufruf im `NanoWebHandler::canHandle()` (`WebHandler.cpp:124-130, 181-187`) implementiert und greift **nur** für die in `nanoWebHandlerList[]` registrierten Pfade. Alle Endpoints, die `WServer::init()` mit `webServer.on(...)` direkt registriert (`/info`, `/setbattmin`, `/settestmode`, `/stop`, `/clientlog`, `/restart`, `/ping`, `/newtoken`, `/rr`, `/help`, `/`, `/favicon.ico`), umgehen diese Middleware komplett. `setFilter(ON_STA_FILTER)` (Zeilen 72, 138) ist **kein** Auth-Mechanismus — er beschränkt nur auf STA-Mode (kein AP), prompt aber niemals nach Credentials.

- **Impact:**
  Auch bei gesetztem Admin-Passwort (`requireAuth() == true`) kann jeder im LAN:
  - `/restart` → Gerät rebooten (DoS, Datenverlust laufende Aufzeichnung)
  - `/stop` → alle Pitmasters auf `pm_off` setzen + persistieren (Sabotage während eines Cooks!)
  - `/setbattmin` → Battery-Min-Schwelle dauerhaft verschieben
  - `/newtoken` → Cloud-Token rotieren (Lock-out vom Cloud-Account)
  - `/settestmode` → Cloud-Interval auf 3 s drücken (Daten-Leak via Cloud)
  - `/clientlog` → Cloud-Debug-Log aktivieren
  - `/info` / `/rr` → Device-Fingerprint, Reset-Reasons (siehe CR-004-005)

- **Root Cause:**
  Auth-Logik ist in einem einzelnen Handler-Subclass eingebaut statt zentral als Server-Filter. Beim Hinzufügen von Endpoints in `WServer.cpp` über `webServer.on()` wird sie nicht automatisch angezogen.

- **Lösung:**
  Zentralisieren — entweder eine kleine Lambda-Helper-Funktion `requireAuthOrFail(request)`, die in jedem destructive-Endpoint am Anfang aufgerufen wird, oder `nanoWebHandlerList[]` um diese Pfade erweitern und den Lambda-Code in dedizierte `handle*`-Member verschieben. Die Liste der schreibenden Endpoints muss **immer** Auth verlangen, sobald `requireAuth() == true`.

- **Code-Beispiel:**
  ```cpp
  // ❌ Aktuell (problematisch):
  webServer.on("/restart", [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(...);
        request->send(response);
        gSystem->restartDeferred();
      })
      .setFilter(ON_STA_FILTER);   // ← KEIN Auth, nur STA-only

  webServer.on("/stop", [](AsyncWebServerRequest *request) {
    for (uint8_t i = 0u; i < gSystem->pitmasters.count(); i++) {
      Pitmaster *pm = gSystem->pitmasters[i];
      if (pm != NULL) pm->setType(pm_off);
    }
    gSystem->pitmasters.saveConfig();   // ← unauthentifiziert!
    request->send(200, TEXTPLAIN, "Stop pitmaster");
  });

  // ✅ Empfohlen:
  static auto requireAuthOrFail = [](AsyncWebServerRequest *request) -> bool {
    if (WServer::requireAuth() &&
        !request->authenticate(WServer::getUsername().c_str(),
                               WServer::getPassword().c_str(),
                               WServer::getRealm())) {
      request->requestAuthentication(WServer::getRealm());
      return false;
    }
    return true;
  };

  webServer.on("/restart", [](AsyncWebServerRequest *request) {
        if (!requireAuthOrFail(request)) return;
        // ... weiter wie gehabt ...
        gSystem->restartDeferred();
      })
      .setFilter(ON_STA_FILTER);

  webServer.on("/stop", [](AsyncWebServerRequest *request) {
    if (!requireAuthOrFail(request)) return;
    // ... pitmaster-stop logic ...
  });
  ```

- **Geschätzter Aufwand:** M (~1 PT — 9 Endpoints umstellen, eine Helper-Lambda, manuell mit gesetztem Passwort verifizieren)
- **GitHub-Issue:** ⏳ OFFEN (#251 enthält nur S-Findings; 001 deferred)
- **Fix-Status:** ⏳ OFFEN (M-Aufwand, bewusst zurückgestellt)

---

### 🟠 HIGH Priority Issues

#### CR-004-002: `/setbattmin` ruft `battery->saveConfig()` direkt aus async_tcp

- **Datei:** `src/WServer.cpp:96-103`
- **Severity:** 🟠 HIGH
- **Kategorie:** Performance / FreeRTOS-Async-Safety
- **Problem:**
  Der Handler ruft `gSystem->battery->saveConfig()` (= `Settings::write(kBattery, json)` → `prefs.putString(...)` → synchroner NVS-Flash-Write, typisch 50–200 ms) im async_tcp-Callback. Identisches Anti-Pattern wie CR-001b-012 (Cloud) und CR-003-001 (Wlan).

- **Impact:**
  Async-TCP-Task wird für die Flash-Write-Dauer blockiert → andere parallele HTTP-Requests werden verzögert; bei Last bzw. Häufung Risk eines Task-Watchdog-Resets.

- **Root Cause:**
  `Battery` hat (im Gegensatz zu `Cloud`/`Mqtt`) noch keinen `saveConfigPending`-Flag. Hot-Path direkt im Handler.

- **Lösung:**
  `saveConfigPending`-Flag in `Battery` analog zu Cloud/Mqtt einführen, im `Battery::update()` (1×/s) verarbeiten. Im Handler nur das Flag setzen.

- **Code-Beispiel:**
  ```cpp
  // ❌ Aktuell:
  webServer.on("/setbattmin", [](AsyncWebServerRequest *request) {
    if (gSystem->battery) {
      gSystem->battery->min -= 100;
      gSystem->battery->saveConfig();   // NVS-Write aus async_tcp
    }
    request->send(200, TEXTPLAIN, "Done");
  });

  // ✅ Empfohlen:
  // Battery.h:  bool saveConfigPending = false;
  // Battery.cpp::update():  if (saveConfigPending) { saveConfig(); saveConfigPending = false; }
  webServer.on("/setbattmin", [](AsyncWebServerRequest *request) {
    if (gSystem->battery) {
      gSystem->battery->min -= 100;
      gSystem->battery->saveConfigPending = true;
    }
    request->send(200, TEXTPLAIN, "Done");
  });
  ```

- **Geschätzter Aufwand:** S (~0,5 PT)
- **GitHub-Issue:** [#251](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/251)
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23) — `saveConfigPending` in `Battery` eingeführt; `Battery::update()` verarbeitet Flag; Handler setzt nur Flag

---

#### CR-004-003: `/stop` ruft `pitmasters.saveConfig()` direkt aus async_tcp

- **Datei:** `src/WServer.cpp:112-122`
- **Severity:** 🟠 HIGH
- **Kategorie:** Performance / FreeRTOS-Async-Safety
- **Problem:**
  Handler iteriert alle Pitmaster auf `pm_off` und persistiert dann via `gSystem->pitmasters.saveConfig()` (`PitmasterGrp.cpp:131-179` → `Settings::write(kPitmasters, ...)`). NVS-Write aus async_tcp.

- **Impact:**
  Wie CR-004-002 — blockiert async_tcp für Flash-Write-Dauer; bei Pitmaster-Profile-Größe (verschachtelt PIDs) tendenziell länger als kBattery, weil mehr serialisiert wird.

- **Root Cause:**
  Kein Pending-Pattern in `PitmasterGrp`. Hot-Path direkt aus Handler.

- **Lösung:**
  Variante A: `saveConfigPending`-Flag in `PitmasterGrp` analog Cloud/Mqtt; in `PitmasterGrp::update()` verarbeiten. Variante B: Da `Pitmaster::setType()` bereits `settingsChanged = true` setzt (`Pitmaster.cpp:127`), könnte `update()` selbst NVS-Persistierung triggern. Variante A ist konsistenter mit dem bestehenden Pattern.

- **Geschätzter Aufwand:** S (~0,5 PT)
- **GitHub-Issue:** [#251](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/251)
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23) — `saveConfigPending` in `PitmasterGrp` eingeführt; Handler setzt nur Flag

---

#### CR-004-004: `/newtoken` ruft `cloud.saveConfig()` direkt — Pending-Pattern existiert bereits

- **Datei:** `src/WServer.cpp:144-147`
- **Severity:** 🟠 HIGH
- **Kategorie:** Performance / FreeRTOS-Async-Safety
- **Problem:**
  `Cloud` besitzt seit Fix CR-001b-012 (Commit `100a3f1`) den `saveConfigPending`-Flag (`Cloud.h:101`, `Cloud.cpp:153-156, 291`). `Cloud::setConfig()` benutzt ihn bereits. Diese Aufrufstelle wurde beim Roll-out übersehen — `cloud.saveConfig()` ist hier weiterhin synchron.

- **Impact:**
  Identisch zu CR-001b-012 — Flash-Write blockiert async_tcp.

- **Root Cause:**
  Roll-out unvollständig; Cross-Module-Aufrufer von `saveConfig()` nicht systematisch migriert.

- **Lösung:**
  ```cpp
  // ❌ Aktuell:
  webServer.on("/newtoken", [](AsyncWebServerRequest *request) {
    request->send(200, TEXTPLAIN, gSystem->cloud.newToken());
    gSystem->cloud.saveConfig();   // ← synchron, NVS-Write
  });

  // ✅ Empfohlen:
  webServer.on("/newtoken", [](AsyncWebServerRequest *request) {
    request->send(200, TEXTPLAIN, gSystem->cloud.newToken());
    gSystem->cloud.saveConfigPending = true;
  });
  ```
  Zusätzlich `grep -n "cloud\.saveConfig()\|->cloud->saveConfig()"` über `src/` ziehen, um weitere Aufrufer zu finden.

- **Geschätzter Aufwand:** S (~0,25 PT)
- **GitHub-Issue:** [#251](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/251)
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23) — `Cloud::requestSaveConfig()` eingeführt; Handler ruft `cloud.requestSaveConfig()` statt `cloud.saveConfig()`

---

#### CR-004-005: `/info` exponiert SSID/MAC/Serial unauthentifiziert

- **Datei:** `src/WServer.cpp:74-94`
- **Severity:** 🟠 HIGH
- **Kategorie:** Security / Privacy
- **Problem:**
  `/info` liefert SSID (Klartext), MAC-Adresse, Serial-Number, Item-Modell, Battery-Voltages, WiFi-Mode, FlashSize, Heap und SPIFFS-Größen — alles **ohne Auth-Check**. Mit CR-004-001 ist der Endpoint öffentlich auf jedem Gerät, das im STA-Mode ist.

- **Impact:**
  - Device-Fingerprinting via Serial/MAC/Item
  - WiFi-Reconnaissance (SSID-Leak)
  - Heap/FlashSize → Hilft Angreifern bei Heap-Exploits-Sizing
  - Privacy-Issue analog zu CR-003-012

- **Root Cause:**
  Endpoint historisch als Debug-Tool gedacht, ohne Sicherheitsbetrachtung released.

- **Lösung:**
  - Mit CR-004-001 zusammen: Auth-Pflicht.
  - SSID-Maskierung: `WiFi.SSID()` nur bei explizitem Debug-Flag oder Substring `"<SSID hidden>"` standardmäßig.
  - Erwägen, `/info` ganz hinter `requireAuth() && DEBUG_BUILD` zu stellen.

- **Geschätzter Aufwand:** S (~0,25 PT — gemeinsam mit CR-004-001)
- **GitHub-Issue:** ⏳ OFFEN (abhängig von CR-004-001)
- **Fix-Status:** ⏳ OFFEN (Auth-Teil abhängig von CR-004-001)

---

#### CR-004-006: Heap-Race auf statischem `String password` (concurrent setPassword/requireAuth)

- **Datei:** `src/WServer.h:39, 45`, `src/WServer.cpp:58, 232, 241-249`
- **Severity:** 🟠 HIGH
- **Kategorie:** Logic / Race Condition
- **Problem:**
  `WServer::password` ist ein static `String` (Heap-allokiert via Arduino-`String`). `setPassword(String)` (aufgerufen aus `WebHandler::handleAdmin` `WebHandler.cpp:442` im async_tcp-Kontext) macht eine Heap-Realloc. `requireAuth()` (`WServer.h:39 inline`) und `getPassword()` werden **aus jeder eingehenden Authentifizierungs-Anfrage** in `NanoWebHandler::canHandle()` (`WebHandler.cpp:126,183`) aufgerufen — d.h. parallel aus weiteren async_tcp-Callbacks.

- **Impact:**
  Während `setPassword()` den Heap-Buffer realloziert, kann ein paralleler Auth-Check `password.length()` lesen → undefined: kann altes `length` mit neuem Pointer kombinieren oder umgekehrt → use-after-free / NULL-Deref / falscher Auth-Erfolg möglich. Identisch zur Pattern-Klasse von CR-003-002 (`Wlan::hostName`).

- **Root Cause:**
  Arduino-`String` ist nicht thread-safe; statischer Member ohne Mutex/atomarer Swap.

- **Lösung:**
  Migration auf `char password[MAX_PWD+1]`-Buffer (analog dem geplanten Fix für CR-003-002), Setzen über `strncpy` mit kurzem `portENTER_CRITICAL`/`portEXIT_CRITICAL` oder `taskENTER_CRITICAL`. Alternativ: Mutex (`SemaphoreHandle_t`) um die statischen Member.

- **Code-Beispiel:**
  ```cpp
  // ❌ Aktuell:
  // WServer.h
  static String password;
  static boolean requireAuth() { return password.length() > 0u; }
  // WServer.cpp
  void WServer::setPassword(String newPassword) { password = newPassword; }

  // ✅ Empfohlen:
  // WServer.h
  static constexpr size_t kPasswordMax = 11;   // Frontend-Limit ist 10
  static char password[kPasswordMax];
  static SemaphoreHandle_t passwordMutex;
  static bool requireAuth();    // nicht inline — muss locken
  // WServer.cpp
  bool WServer::requireAuth() {
    xSemaphoreTake(passwordMutex, portMAX_DELAY);
    bool r = password[0] != '\0';
    xSemaphoreGive(passwordMutex);
    return r;
  }
  ```

- **Geschätzter Aufwand:** M (~1 PT — `String → char[]`-Migration, Mutex einbauen, alle Lese-/Schreibstellen anpassen, mit Phase-2/CR-003-002-Cleanup koppeln)
- **GitHub-Issue:** ⏳ OFFEN (M-Aufwand, mit CR-003-002-Cleanup koppeln)
- **Fix-Status:** ⏳ OFFEN (M-Aufwand, bewusst zurückgestellt)

---

#### CR-004-007: `WServer::saveConfig()` führt Settings::write synchron aus — kein Pending-Pattern

- **Datei:** `src/WServer.cpp:215-221`, Aufrufer `src/WebHandler.cpp:443` (`handleAdmin` POST aus async_tcp)
- **Severity:** 🟠 HIGH
- **Kategorie:** Performance / FreeRTOS-Async-Safety
- **Problem:**
  `WServer::saveConfig()` ruft direkt `Settings::write(kServer, json)` (NVS-Flash) auf. Die einzige bekannte Aufrufstelle (`handleAdmin`) läuft im async_tcp-Kontext.

- **Impact:**
  Setzen/Ändern des Admin-Passworts blockiert async_tcp für Flash-Write-Dauer.

- **Root Cause:**
  WServer hat (analog Battery, PitmasterGrp) noch keinen `saveConfigPending`-Flag implementiert.

- **Lösung:**
  `saveConfigPending`-Flag in `WServer` einführen, in einer `update()`-Methode (wenn nicht vorhanden, an System-Tick andocken) verarbeiten. `WServer::saveConfig()` als private Implementierung halten, `setPassword()` setzt das Flag.

- **Geschätzter Aufwand:** S (~0,25 PT — kann mit CR-004-002/003 kombiniert in einem Sammel-PR „WServer/Battery/PitmasterGrp saveConfigPending roll-out" landen)
- **GitHub-Issue:** [#251](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/251)
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23) — `saveConfigPending`+`update()` in `WServer`; `gWebServer.update()` in `ConnectTask`

---

### 🟡 MEDIUM Priority Issues

#### CR-004-008: `/info` chained 8+ Arduino-`String`-Konkatenationen

- **Datei:** `src/WServer.cpp:87, 90, 92`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Performance / Heap-Fragmentation
- **Problem:**
  Jeder `/info`-Request baut eine 10–15-fach konkatenierte Arduino-`String` aus weiteren `String(...)`-Casts auf — jede Konkatenation kann eine Heap-Realloc auslösen. Latent Heap-Fragmentation analog M2/M3-Cluster.

- **Impact:**
  Einzelner Request unkritisch (manuell aufgerufen, nicht 1×/s). Aber via DoS-Amplifikation durch viele parallele `/info`-Requests in CR-004-001-Szenario zusätzliche Heap-Belastung.

- **Lösung:**
  `snprintf` in einen ausreichend großen `char[]`-Buffer; oder `JsonDocument` + `serializeJson` (konsistent mit anderen API-Endpoints).

- **Geschätzter Aufwand:** S (~0,5 PT)
- **GitHub-Issue:** [#251](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/251)
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23) — `snprintf` in 512-Byte-Stack-Buffer; tote Zeilen entfernt

---

#### CR-004-009: `/setbattmin` ohne Range-/Sanity-Check — `min` kann ins Negative laufen

- **Datei:** `src/WServer.cpp:99`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Input-Validation / Logic
- **Problem:**
  `gSystem->battery->min -= 100;` ohne Untergrenze. Repeated Calls (mit CR-004-001 ohne Auth!) können `min` auf negative `int`-Werte treiben — Battery-Logik vergleicht aber Voltages als unsigned/positiv. Verhalten dann undefined: false-positive „leer" oder „voll".

- **Impact:**
  Battery-Detection unzuverlässig nach Manipulation; führt zu Fehl-Power-Off bzw. -Warnungen. Erholt sich nicht ohne `saveConfig`-Reset.

- **Lösung:**
  Mindest-Untergrenze (z.B. 2800 mV für Li-Ion) sicherstellen: `if (battery->min - 100 >= BATTERY_MIN_FLOOR) battery->min -= 100;` — oder Endpoint ganz entfernen, wenn er nur Debug-Werkzeug ist.

- **Geschätzter Aufwand:** S
- **GitHub-Issue:** [#251](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/251)
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23) — Floor-Check `min - 100 >= 2800` mV

---

#### CR-004-010: `loadConfig()` schluckt JSON-Type-Mismatch silent → Passwort heimlich leer

- **Datei:** `src/WServer.cpp:223-234`
- **Severity:** 🟡 MEDIUM
- **Kategorie:** Logic / Security-Degradation
- **Problem:**
  `json["password"].as<const char*>()` gibt **NULL** zurück, wenn das NVS-Feld zu einem nicht-String-Typ deserialisiert wird (Korruption, manuell geänderte NVS, Migrations-Mismatch). Arduino-`String::operator=(const char*)` mit NULL ruft intern `invalidate()` → leerer String. → `requireAuth()` liefert silent `false`. Der User glaubt, ein Passwort sei gesetzt.

- **Impact:**
  Silent Security-Degradation: Authentication komplett deaktiviert, ohne Log. Bei NVS-Corruption oder Downgrade-Scenarios ein realer Angriffsvektor.

- **Lösung:**
  ```cpp
  if (json.containsKey("password")) {
    const char *pw = json["password"].as<const char*>();
    if (pw != nullptr) {
      this->password = pw;
    } else {
      Log.error("WServer::loadConfig: password key has unexpected type" CR);
    }
  }
  ```

- **Geschätzter Aufwand:** S
- **GitHub-Issue:** [#251](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/251)
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23) — Null-Check + `Log.error` bei Type-Mismatch

---

### 🟢 LOW Priority / Verbesserungsvorschläge

#### CR-004-011: `boolean` (Arduino-Typ) statt `bool` in API

- **Datei:** `src/WServer.h:39`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality
- **Problem:** SKILL.md flag in ESPAsyncWebServer-3.10.x-Sektion. Konsistenz mit moderner API.
- **Lösung:** `static bool requireAuth() { ... }`.
- **Aufwand:** S
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23)

---

#### CR-004-012: Hartkodierter `username = "admin"`

- **Datei:** `src/WServer.cpp:57`
- **Severity:** 🟢 LOW
- **Kategorie:** Security
- **Problem:** Username nicht konfigurierbar. Gleiche Klasse wie CR-003-008 (hardcoded AP-PW). Ergibt mit CR-004-001 sehr leichten Default-Credential-Brute-Force.
- **Lösung:** Mit Phase 3 (App-Store-Onboarding) zusammen lösen — username editierbar.
- **Aufwand:** M
- **Fix-Status:** ⏳ OFFEN (M-Aufwand, mit Phase 3 koppeln)

---

#### CR-004-013: TODO-/auskommentierter Code in `/info`-Handler

- **Datei:** `src/WServer.cpp:79-85`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality
- **Problem:** 6 Zeilen toter Code (auskommentierter SSID-Loop) seit historischer Migration. Identisches Pattern wie CR-001-014 / CR-001b-017.
- **Lösung:** Entfernen — git speichert die Historie.
- **Aufwand:** S
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23)

---

#### CR-004-014: `IPRINTPLN` (Serial.print) statt `Log.notice` für Server-Start

- **Datei:** `src/WServer.cpp:212`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality / Logging
- **Problem:** Inkonsistent mit CR-003-011-Konvention. `IPRINTPLN` wrappt `Serial.println` (`DbgPrint.h`).
- **Lösung:** `Log.notice("HTTP server started" CR);`.
- **Aufwand:** S
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23)

---

#### CR-004-015: `WEB_VUE_ROUTER_PATHS_MAX` (8) ohne Compile-Time-Konsistenzcheck

- **Datei:** `src/WServer.cpp:37, 54-55`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality / Robustness
- **Problem:** `vueRouterPaths[]` wird mit 8 Strings initialisiert, `WEB_VUE_ROUTER_PATHS_MAX = 8` als Loop-Bound (`onNotFound`, `WServer.cpp:181`). Beim Hinzufügen eines Pfades ohne Anpassung der Konstante → out-of-bounds-Read in `onNotFound`.
- **Lösung:** `static constexpr size_t kVueRouterPaths = sizeof(vueRouterPaths) / sizeof(vueRouterPaths[0]);` — oder `std::array` und `.size()`.
- **Aufwand:** S
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23) — `kVueRouterPaths` via `sizeof` eingeführt

---

#### CR-004-016: Kein HTTP-Method-Filter auf den meisten Endpoints

- **Datei:** `src/WServer.cpp:74, 96, 105, 112, 124, 144, 149`
- **Severity:** 🟢 LOW
- **Kategorie:** Code-Quality / Robustness
- **Problem:** `webServer.on("/info", lambda)` ohne Method-Filter akzeptiert GET/POST/PUT/DELETE/OPTIONS gleichermaßen. Sollte auf `HTTP_GET` (lesend) oder `HTTP_POST` (schreibend wie `/setbattmin`) eingeschränkt werden.
- **Lösung:** `webServer.on("/info", HTTP_GET, lambda); webServer.on("/setbattmin", HTTP_POST, lambda);` etc. — bessere Doku der Intent + verhindert versehentliche Method-Confusion.
- **Aufwand:** S
- **Fix-Status:** ✅ GEFIXT — Commit `795b3b0` (2026-05-23) — `HTTP_GET` auf alle Maintenance-Endpoints gesetzt

---

#### CR-004-017: Linearer O(n) Lookup `vueRouterPaths` in `onNotFound`

- **Datei:** `src/WServer.cpp:181-188`
- **Severity:** 🟢 LOW (akzeptabel — nur dokumentiert)
- **Kategorie:** Performance
- **Problem:** Bei n=8 unkritisch. Analog CR-001-017.
- **Lösung:** Aktuell keiner — bei wachsender Pfadliste auf `std::find` / Hash-Set wechseln.
- **Aufwand:** S
- **Fix-Status:** ⏳ OFFEN (akzeptabel — nur dokumentiert)

---

## Positive Aspekte

- ✅ `/restart` benutzt korrekt `gSystem->restartDeferred()` (Fix CR-001-003) und schickt vorab eine Response — async_tcp-Anti-Pattern hier sauber vermieden.
- ✅ Vue-Router-SPA-Fallback in `onNotFound` korrekt implementiert (Vue-Routes liefern `index.html`, alle anderen 404).
- ✅ `Cloud::setConfig()` wird genutzt (`/settestmode`), nicht direkter NVS-Write — d.h. das `saveConfigPending`-Pattern wird **dort** korrekt verwendet (im Gegensatz zu `/newtoken`, siehe CR-004-004).
- ✅ ArduinoJson-v7-Pattern in `saveConfig`/`loadConfig` korrekt (`JsonDocument`, `to<JsonObject>()`, `as<const char*>()`).
- ✅ Static-File-Serving via `beginResponse_P` mit `Content-Encoding: gzip` korrekt (Embedded-Asset-Pattern).
- ✅ `OPTIONS`-Preflight in `onNotFound` mit 200 beantwortet (CORS-Compliance).

---

## Vergleich zum vorherigen Sonnet-Review

> Entfällt — Task ist 📝 NEU, kein Vorgänger-Review.

---

## Metriken

| Metrik | Wert |
|--------|------|
| **Gefundene Issues — CRITICAL** | 1 |
| **Gefundene Issues — HIGH** | 6 |
| **Gefundene Issues — MEDIUM** | 3 |
| **Gefundene Issues — LOW** | 7 |
| **Geschätzter Fix-Aufwand** | ~3 PT (Sammel-PR mit CR-004-001 + 4× Pending-Roll-out) |
| **Code-Qualität (1–10)** | 5/10 |
| **Wartbarkeit (1–10)** | 6/10 |

---

## Empfohlene Nächste Schritte

1. **CR-004-001 sofort fixen** — Auth-Bypass auf destruktiven Endpoints ist ein Showstopper. Helper-Lambda zentralisieren, alle `webServer.on()`-Pfade durchgehen.
2. **Sammel-PR „WServer/Battery/PitmasterGrp saveConfigPending roll-out"** — CR-004-002, CR-004-003, CR-004-004, CR-004-007 in einem Pass; selbe Logik wie CR-001b-012-Fix.
3. **CR-004-006** mit Phase-2-/CR-003-002-Cleanup koppeln — `String password` → `char[]`+Mutex parallel zur `Wlan::hostName`-Migration.
4. **CR-004-005 + CR-004-010** als Security-Hardening-PR — `/info` hinter Auth, SSID-Maskierung, JSON-Type-Mismatch defensiv.
5. **LOW-Cluster (CR-004-011 bis -017)** als sammeln-und-fegen Cleanup-PR (~1 PT).

---

## Anhang: Analysierte Dateien

| Datei | LOC | Bemerkung |
|-------|-----|-----------|
| `src/WServer.cpp` | 251 | Server-Init, 12 Endpoint-Handler, `saveConfig`/`loadConfig`, `onNotFound`. |
| `src/WServer.h` | 47 | Klasse `WServer`; static `username`/`password`, inline `requireAuth()`. |

---

## Tracker-Update

> Diese Zeile nach dem Review in `CODE_REVIEW_TRACKER.md` übernehmen:

```markdown
| TASK-004 | WServer | 298 | 🟠 HIGH | ✅ DONE | 1/6/3/7 | [reviews/REVIEW_TASK_004.md](./reviews/REVIEW_TASK_004.md) | Auth-Middleware umgangen auf 9 Endpoints; 4× direktes saveConfig() aus async_tcp; gehört in Sammel-PR mit CR-001b-012/CR-003-001 |
```

**Findings-Counts in Status-Übersicht aktualisieren:**
- Critical: +1
- High: +6
- Medium: +3
- Low: +7
