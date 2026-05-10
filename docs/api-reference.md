# WLANThermo ESP32 — API-Referenz

## Übersicht

Die Firmware stellt eine **REST-API** über HTTP bereit (Port 80, ESPAsyncWebServer). Es gibt keine WebSocket-Endpoints.

**Basis-URL:** `http://<geräte-ip>/` oder `http://<hostname>.local/`

### Authentifizierung

HTTP Basic Auth, Realm `WLANThermo Device`, Benutzername `admin`.  
Auth ist nur aktiv, wenn ein Passwort gesetzt ist (`/setadmin`). Ist kein Passwort gesetzt, werden alle Requests ohne Auth akzeptiert.

In der Tabelle unten steht bei **Auth** ob der Endpoint geschützt ist:
- **Nein** — kein Passwort erforderlich
- **Ja (POST)** — nur POST-Requests erfordern Auth
- **Ja** — alle Methoden erfordern Auth

---

## GET-Endpoints

### `GET /data`

Liefert den aktuellen Gerätezustand (Temperaturen, Pitmaster, System).

**Auth:** Nein

**Response** `application/json`:

```json
{
  "system": {
    "time": 1746700800,
    "unit": "C",
    "soc": 85,
    "charge": false,
    "rssi": -62,
    "online": 2
  },
  "channel": [
    {
      "number": 1,
      "name": "Kerntemperatur",
      "typ": 0,
      "temp": 75.3,
      "min": 50.0,
      "max": 90.0,
      "alarm": 0,
      "color": "#E54304",
      "fixed": false,
      "connected": true
    }
  ],
  "pitmaster": {
    "type": ["off", "manual", "auto"],
    "pm": [
      {
        "id": 0,
        "channel": 1,
        "pid": 0,
        "value": 45,
        "set": 120.0,
        "typ": "auto",
        "typ_last": "manual",
        "set_color": "#ff0000",
        "value_color": "#000000"
      }
    ]
  }
}
```

**`system`-Felder:**

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `time` | int | Unix-Timestamp (Systemzeit) |
| `unit` | string | Temperatureinheit: `"C"` oder `"F"` |
| `soc` | int | Akkuladestand in % (nur bei Geräten mit Akku) |
| `charge` | bool | Akku wird geladen (nur bei Geräten mit Akku) |
| `rssi` | int | WiFi-Signalstärke in dBm |
| `online` | int | Cloud-Verbindungsstatus: `0`=offline, `1`=verbunden, `2`=aktiv |

**`channel[]`-Felder:**

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `number` | int | Kanalnummer (1-basiert) |
| `name` | string | Kanalname (max. 10 Zeichen) |
| `typ` | int | Fühlertyp-Index |
| `temp` | float | Aktuelle Temperatur (1 Nachkommastelle); `999` = nicht verbunden |
| `min` | float | Untere Alarmgrenze |
| `max` | float | Obere Alarmgrenze |
| `alarm` | int | Alarmeinstellung: `0`=aus, `1`=Minalarm, `2`=Maxalarm, `3`=beides |
| `color` | string | Farbe als Hex-String (z.B. `"#E54304"`) |
| `fixed` | bool | `true` = Sensor fest eingebaut (nicht tauschbar) |
| `connected` | bool | Sensor aktuell verbunden |

**`pitmaster.pm[]`-Felder:**

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `id` | int | Pitmaster-Index (0-basiert) |
| `channel` | int | Zugewiesener Kanal (1-basiert) |
| `pid` | int | Zugewiesenes PID-Profil (`id`) |
| `value` | int | Aktueller Ausgabewert in % (0–100) |
| `set` | float | Solltemperatur |
| `typ` | string | Aktueller Modus: `"off"`, `"manual"`, `"auto"` |
| `typ_last` | string | Letzter aktiver Modus (für UI-Umschaltung) |
| `set_color` | string | Farbe der Solltemperatur-Anzeige |
| `value_color` | string | Farbe der Ausgabewert-Anzeige |

---

### `GET /settings`

Liefert Gerätekonfiguration (Systemeinstellungen, Sensoren, PID-Profile, IoT).

**Auth:** Nein

**Response** `application/json`:

```json
{
  "system": {
    "time": 1746700800,
    "unit": "C",
    "ap": "WLANThermo-Mini",
    "host": "wlanthermo",
    "language": "de",
    "version": "v3.5.0",
    "getupdate": "v3.5.1",
    "autoupd": true,
    "prerelease": false,
    "crashreport": true,
    "hwversion": "V3"
  },
  "hardware": ["V3"],
  "api": { "version": "2.0.0" },
  "sensors": [
    { "type": 0, "name": "Maverick", "fixed": false }
  ],
  "features": {
    "bluetooth": true,
    "pitmaster": true
  },
  "pid": [
    {
      "id": 0,
      "name": "WLANThermo",
      "aktor": 0,
      "Kp": 3.74,
      "Ki": 0.02,
      "Kd": 26.0,
      "DCmmin": 0.0,
      "DCmmax": 100.0,
      "opl": 35,
      "SPmin": 0.0,
      "SPmax": 100.0,
      "link": 0,
      "tune": false,
      "jp": 100
    }
  ],
  "aktor": ["SSR", "FAN", "SERVO"],
  "display": {
    "updname": "",
    "orientation": 0
  },
  "iot": {
    "PMQhost": "mqtt.example.com",
    "PMQport": 1883,
    "PMQuser": "user",
    "PMQpass": "pass",
    "PMQqos": 0,
    "PMQon": false,
    "PMQint": 5,
    "CLon": false,
    "CLtoken": "",
    "CLint": 30,
    "CLurl": "cloud.wlanthermo.de/index.html",
    "CCLon": false,
    "CCLint": 60,
    "CCLurl": ""
  }
}
```

**`system`-Felder (settings-Modus):**

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `ap` | string | SSID des Access-Points im AP-Modus |
| `host` | string | mDNS-Hostname (max. 13 Zeichen) |
| `language` | string | Sprache: `"de"`, `"en"`, etc. |
| `version` | string | Aktuelle Firmware-Version |
| `getupdate` | string | Verfügbare Update-Version |
| `autoupd` | bool | Automatische Updates aktiv |
| `prerelease` | bool | Pre-Release-Updates einschließen |
| `crashreport` | bool | Crash-Reports aktiviert |
| `hwversion` | string | Hardware-Version (z.B. `"V3"`) |

**`pid[]`-Felder:**

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `id` | int | Profil-Index |
| `name` | string | Profilname |
| `aktor` | int | Aktuator-Typ: `0`=SSR, `1`=FAN, `2`=SERVO, `3`=DAMPER |
| `Kp` / `Ki` / `Kd` | float | PID-Parameter (2 Nachkommastellen) |
| `DCmmin` / `DCmmax` | float | Duty-Cycle Min/Max in % |
| `opl` | int | Open-Lid-Schwelle in % |
| `SPmin` / `SPmax` | float | Servo-Pulsweiten-Grenzen in ms |
| `link` | int | Pitmaster-Verknüpfung |
| `tune` | bool | Autotune aktiv |
| `jp` | int | Jump-Power in % (10–100) |

**`iot`-Felder:**

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `PMQhost` | string | MQTT-Broker-Hostname |
| `PMQport` | int | MQTT-Port |
| `PMQuser` / `PMQpass` | string | MQTT-Zugangsdaten |
| `PMQqos` | int | MQTT Quality of Service (0/1/2) |
| `PMQon` | bool | MQTT aktiviert |
| `PMQint` | int | MQTT-Sendeintervall in Sekunden |
| `CLon` | bool | WLANThermo Cloud aktiviert |
| `CLtoken` | string | Cloud API-Token |
| `CLint` | int | Cloud-Sendeintervall in Sekunden |
| `CLurl` | string | Cloud-URL (fest: `cloud.wlanthermo.de/index.html`) |
| `CCLon` | bool | Custom Cloud aktiviert |
| `CCLint` | int | Custom-Cloud-Sendeintervall in Sekunden |
| `CCLurl` | string | Custom-Cloud-Endpoint-URL |

---

### `GET /networklist`

Liefert WLAN-Scan-Ergebnisse und aktuellen Verbindungsstatus. Zuerst `GET /networkscan` aufrufen.

**Auth:** Nein

**Response** `application/json`:

```json
{
  "Connect": true,
  "SSID": "MeinNetz",
  "BSSID": "AA:BB:CC:DD:EE:FF",
  "IP": "192.168.1.100",
  "Mask": "255.255.255.0",
  "Gate": "192.168.1.1",
  "RSSI": -62,
  "Enc": 3,
  "Scan": [
    { "SSID": "MeinNetz", "BSSID": "AA:BB:CC:DD:EE:FF", "RSSI": -62, "Enc": 3 },
    { "SSID": "Nachbar", "BSSID": "11:22:33:44:55:66", "RSSI": -80, "Enc": 4 }
  ]
}
```

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `Connect` | bool | Aktuell mit WLAN verbunden |
| `SSID` | string | Verbundenes Netz (oder AP-SSID) |
| `IP` | string | IP-Adresse |
| `RSSI` | int | Signalstärke des verbundenen Netzes |
| `Enc` | int | Verschlüsselungstyp (Arduino-Enum) |
| `Scan[]` | array | Gefundene Netze |

---

### `GET /networkscan`

Startet einen asynchronen WLAN-Scan. Ergebnisse anschließend mit `GET /networklist` abrufen.

**Auth:** Nein  
**Response:** `200 OK`

---

### `GET /checkupdate`

Löst eine Prüfung auf verfügbare Firmware-Updates aus.

**Auth:** Nein  
**Response:** `200 true`

---

### `GET /getbluetooth`

Liefert den BLE-Status und gefundene Geräte.

**Auth:** Nein

**Response** `application/json`:

```json
{
  "enabled": true,
  "devices": [
    {
      "name": "iBBQ",
      "address": "AA:BB:CC:DD:EE:FF",
      "count": 4,
      "selected": 3
    }
  ]
}
```

`selected` ist ein Bitfield: Bit 0 = Sonde 1, Bit 1 = Sonde 2, etc.

---

### `GET /getdeviceid`

Liefert die eindeutige Geräte-ID.

**Auth:** Nein  
**Response:** `text/plain` — Geräte-ID als String

---

### `GET /getpush`

Liefert die aktuelle Push-Benachrichtigungskonfiguration.

**Auth:** Nein

**Response** `application/json`:

```json
{
  "telegram": {
    "enabled": false,
    "token": "123456:ABC...",
    "chat_id": "-100123456"
  },
  "pushover": {
    "enabled": false,
    "token": "azGDORePK8gMaC0QOYAMyEEuzJnyUi",
    "user_key": "uQiRzpo4DXghDmr9QzzfQu27cmVRsG",
    "priority": 0
  },
  "app": {
    "enabled": false,
    "max_devices": 5,
    "devices": [
      {
        "name": "Mein Handy",
        "id": "abc123",
        "token_sha256": "sha256-hash",
        "sound": 0
      }
    ]
  }
}
```

---

### `GET /log`

Liefert den internen Log-Ringpuffer.

**Auth:** Ja  
**Response:** `text/plain` — Logzeilen

---

### `GET /recovery`

Zeigt die Recovery-Seite an und leitet das Gerät in den Recovery-Modus (wird asynchron via `Wlan::update()` ausgeführt).

**Auth:** Ja  
**Response:** `text/html` (gzip) — Recovery-Seite

---

### `GET /configreset`

Zeigt ein HTML-Bestätigungsformular zum Zurücksetzen aller Einstellungen.

**Auth:** Nein  
**Response:** `text/html`

---

### `GET /clearwifi`

Zeigt ein HTML-Bestätigungsformular zum Löschen der WLAN-Zugangsdaten.

**Auth:** Nein  
**Response:** `text/html`

---

### `GET /setadmin`

Zeigt ein HTML-Formular zum Setzen des Web-UI-Passworts.

**Auth:** Nein  
**Response:** `text/html`

---

### `GET /update`

Zeigt ein HTML-Formular zur manuellen Firmware-Versionseingabe.

**Auth:** Nein  
**Response:** `text/html`

---

### `GET /stopwifi`

Trennt die WLAN-Verbindung.

**Auth:** Nein  
**Response:** `200 true`

---

## POST-Endpoints

### `POST /setchannels`

Ändert die Konfiguration eines Temperaturkanals.

**Auth:** Ja  
**Content-Type:** `application/json`

**Request-Body:**

```json
{
  "number": 1,
  "name": "Kerntemperatur",
  "typ": 0,
  "min": 50.0,
  "max": 90.0,
  "alarm": 3,
  "color": "#E54304"
}
```

| Feld | Typ | Pflicht | Beschreibung |
|------|-----|---------|--------------|
| `number` | int | Ja | Kanalnummer (1-basiert) |
| `name` | string | Nein | Name (max. 10 Zeichen, UTF-8) |
| `typ` | int | Nein | Fühlertyp-Index |
| `min` | float | Nein | Untere Alarmgrenze |
| `max` | float | Nein | Obere Alarmgrenze |
| `alarm` | int | Nein | `0`=aus, `1`=min, `2`=max, `3`=beides |
| `color` | string | Nein | Hex-Farbe |

**Response:** `200 true` / `200 false`

---

### `POST /setsystem`

Ändert Systemeinstellungen.

**Auth:** Ja  
**Content-Type:** `application/json`

**Request-Body:**

```json
{
  "unit": "C",
  "language": "de",
  "host": "wlanthermo",
  "ap": "WLANThermo-Mini",
  "autoupd": true,
  "prerelease": false,
  "crashreport": true
}
```

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `unit` | string | `"C"` oder `"F"` |
| `language` | string | Sprachcode |
| `host` | string | mDNS-Hostname (max. 13 Zeichen) |
| `ap` | string | AP-SSID (max. 13 Zeichen) |
| `autoupd` | bool | Auto-Update aktivieren |
| `prerelease` | bool | Pre-Releases einschließen |
| `crashreport` | bool | Crash-Reports aktivieren |

**Response:** `200 true` / `200 false`

---

### `POST /setpitmaster`

Setzt Pitmaster-Konfiguration (Array mit einem Eintrag pro Pitmaster).

**Auth:** Ja  
**Content-Type:** `application/json`

**Request-Body:**

```json
[
  {
    "id": 0,
    "channel": 1,
    "pid": 0,
    "set": 120.0,
    "typ": "auto",
    "value": 0
  }
]
```

| Feld | Typ | Pflicht | Beschreibung |
|------|-----|---------|--------------|
| `id` | int | Ja | Pitmaster-Index |
| `channel` | int | Ja | Kanal (1-basiert) |
| `pid` | int | Ja | PID-Profil-Index |
| `set` | float | Ja | Solltemperatur |
| `typ` | string | Ja | `"off"`, `"manual"`, `"auto"` |
| `value` | int | Bei `typ=manual` | Ausgabewert 0–100 % |

**Response:** `200 true` / `200 false`

---

### `POST /setpid`

Ändert PID-Profile (Array mit einem Eintrag pro Profil).

**Auth:** Ja  
**Content-Type:** `application/json`

**Request-Body:**

```json
[
  {
    "id": 0,
    "name": "WLANThermo",
    "aktor": 1,
    "Kp": 3.74,
    "Ki": 0.02,
    "Kd": 26.0,
    "DCmmin": 0.0,
    "DCmmax": 100.0,
    "opl": 35,
    "SPmin": 0.0,
    "SPmax": 100.0,
    "link": 0,
    "tune": false,
    "jp": 100
  }
]
```

Alle Felder außer `id` sind optional. Nur gesendete Felder werden übernommen.

**Response:** `200 true` / `200 false`

---

### `POST /setIoT`

Ändert MQTT- und Cloud-Konfiguration.

**Auth:** Ja  
**Content-Type:** `application/json`

**Request-Body:**

```json
{
  "PMQhost": "mqtt.example.com",
  "PMQport": 1883,
  "PMQuser": "user",
  "PMQpass": "pass",
  "PMQqos": 0,
  "PMQon": true,
  "PMQint": 5,
  "CLon": false,
  "CLtoken": "",
  "CLint": 30,
  "CCLon": false,
  "CCLurl": "https://mein-server.de/api",
  "CCLint": 60
}
```

Alle Felder optional. Feldnamen entsprechen den Feldern in `GET /settings` → `iot`.

**Response:** `200 true` / `200 false`

---

### `POST /setpush`

Konfiguriert Push-Benachrichtigungen (Telegram, Pushover, App). Optional: Testnachricht auslösen.

**Auth:** Ja  
**Content-Type:** `application/json`

**Request-Body:**

```json
{
  "test": false,
  "telegram": {
    "enabled": true,
    "token": "123456:ABC...",
    "chat_id": "-100123456"
  },
  "pushover": {
    "enabled": false,
    "token": "azGDORePK8gMaC0QOYAMyEEuzJnyUi",
    "user_key": "uQiRzpo4DXghDmr9QzzfQu27cmVRsG",
    "priority": 0,
    "retry": 60,
    "expire": 3600
  },
  "app": {
    "enabled": false,
    "devices": [
      {
        "name": "Mein Handy",
        "id": "abc123",
        "token": "fcm-token",
        "sound": 0,
        "android_channel_id": "wlanthermo"
      }
    ]
  }
}
```

Wenn `"test": true`, wird eine Testnachricht gesendet (Konfiguration wird nicht gespeichert).  
Statt `token` kann auch `token_sha256` übergeben werden — das Gerät sucht dann den passenden gespeicherten Token.

**Response:** `200 true` / `200 false`

---

### `POST /setbluetooth`

Aktiviert/deaktiviert BLE und weist BLE-Sonden Kanälen zu.

**Auth:** Ja  
**Content-Type:** `application/json`

**Request-Body:**

```json
{
  "enabled": true,
  "devices": [
    {
      "address": "AA:BB:CC:DD:EE:FF",
      "count": 4,
      "selected": 3
    }
  ]
}
```

`selected` ist ein Bitfield: Bit 0 = Sonde 1 aktiv, Bit 1 = Sonde 2 aktiv, etc.

**Response:** `200 true` / `200 false`

---

### `POST /setnetwork`

Setzt die primären WLAN-Zugangsdaten (ersetzt bestehende).

**Auth:** Nein  
**Content-Type:** `application/json`

**Request-Body:**

```json
{
  "ssid": "MeinNetz",
  "password": "geheimesPasswort"
}
```

**Response:** `200 true` / `200 false`

---

### `POST /setadmin`

Setzt das Web-UI-Passwort (max. 10 Zeichen). Leeres Passwort deaktiviert die Auth.

**Auth:** Ja  
**Content-Type:** `application/x-www-form-urlencoded`

**Parameter:** `wwwpw=neuesPasswort`

**Response:** `200 true` / `200 false`

---

### `POST /rotate`

Dreht die Display-Ausrichtung (90°/270°-Toggle). Löst bei Nextion-Displays ein OTA-Update der Display-Firmware aus.

**Auth:** Ja  
**Response:** `text/html` (gzip) — Neustart-Seite

---

### `POST /calibrate`

Startet die Touchscreen-Kalibrierung.

**Auth:** Ja  
**Response:** `200 true`

---

### `POST /configreset`

Setzt alle NVS-Einstellungen auf Werkseinstellungen zurück.

**Auth:** Nein  
**Response:** `200 true`

---

### `POST /clearwifi`

Löscht alle gespeicherten WLAN-Zugangsdaten und startet das Gerät neu.

**Auth:** Nein  
**Response:** `200 true` (dann Neustart)

---

### `POST /update`

Löst ein OTA-Firmware-Update aus.

**Auth:** Ja  
**Content-Type:** `application/x-www-form-urlencoded`

**Parameter:** `version=v3.5.1` (mit führendem `v`)  
Ohne Parameter: startet sofort das zuletzt angekündigte Update.

**Response:** `200 Do Update...` / `200 Version unknown!`

---

### `POST /updatestatus`

Prüft, ob ein OTA-Update läuft.

**Auth:** Nein  
**Response:** `200 true` (läuft) / `200 false`

---

### `POST /dcstatus`

Prüft, ob ein Duty-Cycle-Test läuft.

**Auth:** Nein  
**Response:** `200 true` / `200 false`

---

### `POST /setDC`

Startet einen Duty-Cycle-Test für einen Aktuator.

**Auth:** Nein  
**Content-Type:** `application/json`

**Request-Body:**

```json
{
  "aktor": 1,
  "dc": true,
  "val": 500
}
```

| Feld | Typ | Beschreibung |
|------|-----|--------------|
| `aktor` | int | Aktuator-Typ: `0`=SSR, `1`=FAN, `2`=SERVO |
| `dc` | bool | Test starten (`true`) |
| `val` | int | Zielwert × 10 (z.B. `500` = 50 %) |

**Response:** `200 true` / `200 false`

---

### `POST /setapi`

Interne Server-API für Cloud-Kommunikation (wird vom WLANThermo-Server aufgerufen).

**Auth:** Ja  
**Content-Type:** `application/json`

**Request-Body:**

```json
{
  "url": {
    "ota": { "host": "ota.wlanthermo.de", "page": "/firmware" }
  },
  "update": {
    "available": true,
    "version": "v3.5.1",
    "firmware": { "url": "https://..." }
  },
  "cloud": {
    "task": true
  }
}
```

**Response:** `200 true` / `200 false`

---

## Serielle Schnittstelle

**Baudrate:** 115200, **Format:** `befehl` oder `befehl:payload` (Zeilenende: CR `\r`)

### Befehle ohne Parameter

| Befehl | Beschreibung |
|--------|--------------|
| `data` | Gibt aktuellen JSON-Datensatz aus |
| `settings` | Gibt Einstellungs-JSON aus |
| `item` | Gibt den gespeicherten Item-String aus |
| `restart` | Startet das Gerät neu |
| `stop` | Stoppt alle Pitmaster (setzt Typ auf `off`) |
| `clearwifi` | Löscht WLAN-Zugangsdaten und startet neu |
| `configreset` | Setzt alle NVS-Einstellungen zurück |
| `resetpassword` | Löscht das Web-UI-Passwort (Auth deaktiviert) |
| `resetID` | Setzt die Geräte-ID zurück |
| `checkupdate` | Prüft auf Firmware-Updates |
| `heap` | Gibt freien Heap-Speicher in Bytes aus |
| `enabledisplay` | Aktiviert das Display (nur mini V1–V3) |
| `disabledisplay` | Deaktiviert das Display (nur mini V1–V3) |
| `calibratedisplay` | Startet Touchscreen-Kalibrierung (nur mini V1–V3) |
| `removecalibrationTFT` | Löscht Touchscreen-Kalibrierdaten aus NVS (nur mini V1–V3) |
| `resetoled` | Reset des OLED-Displays via GPIO-Pulse (nur nano V3) |

### Befehle mit Parameter (`befehl:payload`)

| Befehl | Payload | Beschreibung |
|--------|---------|--------------|
| `setnetwork` | JSON `{"ssid":"...","password":"..."}` | Setzt WLAN-Zugangsdaten |
| `addnetwork` | JSON `{"ssid":"...","password":"..."}` | Fügt WLAN-Zugangsdaten hinzu |
| `item` | String | Speichert Item-String im NVS |
| `update` | Versionsstring (z.B. `v3.5.1`) | Löst Update auf bestimmte Version aus |
| `setbattmin` | 4-stellige Zahl (mV) | Setzt Akku-Mindestspannung |
| `setbattmax` | 4-stellige Zahl (mV) | Setzt Akku-Maximalspannung |

---

## Cloud-API (ausgehend)

Das Gerät sendet regelmäßig Daten an externe Endpoints:

### WLANThermo Cloud (`POST cloud.wlanthermo.de`)

```json
{
  "device": { "device": "miniV3", "serial": "...", ... },
  "cloud": {
    "task": "save",
    "api_token": "...",
    "data": [
      {
        "system": { "time": ..., "unit": "C", ... },
        "channel": [...],
        "pitmaster": [...]
      }
    ]
  }
}
```

### Custom Cloud (`POST <CCLurl>`)

```json
{
  "version": 1,
  "interval": 60,
  "channel": [
    {
      "number": 1,
      "name": "Kerntemperatur",
      "temp": 75.3,
      "unit": "C",
      "min": 50.0,
      "max": 90.0,
      ...
    }
  ]
}
```

`temp` ist `null` wenn der Sensor nicht verbunden ist.
