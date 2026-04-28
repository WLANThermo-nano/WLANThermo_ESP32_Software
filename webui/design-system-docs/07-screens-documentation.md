# Screen-Dokumentation

## Router-Übersicht

**Router-Mode:** `hash` (alle Varianten) — URLs wie `/#/pitmaster`
**Base URL:** variabel per `VUE_APP_BASE_URL`

```js
const routes = [
  { path: '/',             name: 'home',         component: Home },
  { path: '/wlan',         name: 'wlan',          component: Wlan },
  { path: '/system',       name: 'system',        component: System },
  { path: '/bluetooth',    name: 'bluetooth',     component: Bluetooth },
  { path: '/pitmaster',    name: 'pitmaster',     component: Pitmaster },
  { path: '/about',        name: 'about',         component: About },
  { path: '/iot',          name: 'iot',           component: IoT },
  { path: '/notification', name: 'notification',  component: PushNotification },
  { path: '/scan',         name: 'scan',          component: Scan }  // Mobile only
]
```

**Kein Lazy Loading** — alle Komponenten werden beim App-Start geladen (sinnvoll für embedded-Device).

---

## Screen 1: Home — Temperatur-Dashboard

**Route:** `/`  
**Komponente:** `Home.vue`

### Zweck
Hauptansicht: Zeigt alle aktiven Temperaturkanäle als Kacheln. Primärer Einstiegspunkt nach dem Laden.

### Layout
- Pure CSS Grid: 1 → 2 → 4 Spalten (mobile → tablet → desktop)
- Kacheln: `70px` Höhe, farbiger linker Rand (Kanalfarbe)

### Datenfluss
- **Input:** Props `channels`, `pitmasterpm`, `unit` von `App.vue` (alle 2s aktualisiert)
- **Edit:** `GET /settings` für Sensor-Typen; `POST /setchannels` zum Speichern

### Kanal-Kachel Aufbau
```
┌──────────────────────────────┐
│ Kanalname         #Nummer    │  ← title-row
│ ↑ Max             23.5°C    │  ← body-row
│ ↓ Min                       │
└──────────────────────────────┘
```
- Mit Pitmaster (auto): Zeigt Zieltemperatur + Fan-Icon + Fan-% statt Name
- Verbindungsstatus-Icons: `icon-bluetooth_1` (BLE), `icon-radio` (Remote)

### User-Flows
1. Temperatur beobachten → Kacheln (automatisch alle 2s aktualisiert)
2. Kanal konfigurieren → Kachel antippen → Edit-Formular

### Edit-Formular
Felder: Name, Max-Temp (validiert), Min-Temp (validiert), Sensor-Typ, Farbe, Push-Alarm, Buzzer-Alarm

---

## Screen 2: WLAN-Einstellungen

**Route:** `/wlan`  
**Komponente:** `Wlan.vue`

### Zweck
WiFi-Netzwerk-Verwaltung: Aktuelles Netz anzeigen, Scan, Netz auswählen, Passwort eingeben.

### Layout
- Config-Form-Container (Vollbreite)
- Aktuelles Netzwerk: Blauer Hintergrund, Icon + SSID + BSSID
- Netzwerkliste: Expandierbare Items (Passwort-Panel)

### Datenfluss
- `POST /networkscan` → scan starten
- `POST /networklist` → nach 3s Scan-Ergebnis abrufen
- `POST /setnetwork { ssid, password }` → Netz verbinden
- `POST /clearwifi` → gespeicherte Netze löschen
- `POST /stopwifi` → WiFi deaktivieren (leert Browser-Tab)

### Besonderheit
WiFi-Icon zeigt Signalstärke (dBm → `Wlan33/66/100`). Aktuelles Netzwerk erhält Refresh nach Verbindung.

---

## Screen 3: System-Einstellungen

**Route:** `/system`  
**Komponente:** `System.vue`

### Zweck
Grundlegende Gerätekonfiguration: Hostname, AP-Name, Sprache, Temperatureinheit, Hardware-Version, Auto-Update.

### Layout
- Einzelne Config-Form
- Optional: Display-Einstellungen (nur mini v1/v2)

### Datenfluss
- `GET /settings` → Einstellungen laden
- `POST /setsystem` → Speichern
- `POST /rotate` → Display drehen (nur mini v1/v2, Neustart)
- `POST /calibrate` → Touch-Kalibrierung (nur mini v1/v2)

### Feature-Flags
- Display-Einstellungen: `settings.device.device === 'mini' && hw_version in ['v1', 'v2']`

---

## Screen 4: Bluetooth

**Route:** `/bluetooth`  
**Komponente:** `Bluetooth.vue`

### Zweck
BLE-Sensor-Verwaltung: Bluetooth aktivieren/deaktivieren, erkannte Geräte und deren Kanäle konfigurieren.

### Layout
- Checkbox für Aktivierung
- Liste expandierbarer BLE-Geräte
- Im expanded State: Kanal-Checkboxen (Bit-Maske)

### Datenfluss
- `GET /getbluetooth` → Geräte-Liste laden
- `POST /setbluetooth { enabled, devices }` → Speichern

### Besonderheit
Kanal-Auswahl per Bit-Maske: `device.selected & (1 << channelIndex)`. Refresh-Button mit 200ms Delay für UX.

---

## Screen 5: Pitmaster

**Route:** `/pitmaster`  
**Komponente:** `Pitmaster.vue`

### Zweck
PID-Temperaturregler konfigurieren. Zwei Modi: Pitmaster-Liste und Profil-Editor (inline, kein eigener Route).

### Layout
- **Modus 1:** Liste aller Pitmaster (je nach Hardware 1-2), je mit Typ/Profil/Kanal/Wert-Auswahl
- **Modus 2:** Profil-Editor mit Sektionen: Name, Aktor, PID-Parameter, Erweitert (Lid-Open-Detection)

### Datenfluss
- `GET /data` + `GET /settings` (parallel) → Pitmaster + Profile + Aktoren laden
- `POST /setpitmaster` → Pitmaster-Liste speichern
- `POST /setpid` → Profile speichern

### Formular-Felder im Profil-Editor

| Aktor | Felder |
|-------|--------|
| SSR (0) | DCmmin, DCmmax |
| FAN (1) | DCmmin, DCmmax |
| SERVO (2) | SPmin, SPmax |
| DAMPER (3) | DCmmin, DCmmax + SPmin, SPmax + Link-Modus |

PID: Kp (0-150), Ki (0-5, step 0.01), Kd (0-800), jp/Jump (10-100)

---

## Screen 6: IoT / Cloud

**Route:** `/iot`  
**Komponente:** `IoT.vue`

### Zweck
Cloud-Dienste und MQTT-Integration konfigurieren.

### Layout
Drei Sektionen (config-form je Sektion):
1. WLANThermo Cloud
2. Custom Cloud (eigene URL)
3. Private MQTT

### Datenfluss
- `GET /settings` → IoT-Settings laden
- `POST /setIoT` → Speichern
- `POST /newtoken` → Neuen Cloud-Token generieren

### Besonderheit
Cloud-URL als klickbarer Link (öffnet Browser-Tab / Cordova InAppBrowser).

---

## Screen 7: Push-Benachrichtigungen

**Route:** `/notification`  
**Komponente:** `PushNotification.vue`

### Zweck
Push-Alarm-Dienste konfigurieren: WLANThermo App (Mobile), Telegram, Pushover.

### Layout
- Optional (nur Mobile): App-Sektion mit Geräteverwaltung
- Telegram-Sektion (Token, Chat-ID)
- Pushover-Sektion (Token, User-Key, Priorität)
- Je Sektion: "Test-Nachricht" Button

### Datenfluss
- `GET /getpush` → Settings laden
- `POST /setpush` → Speichern + Test senden
- Cordova Firebase Messaging API (nur Mobile): Token-Generierung

---

## Screen 8: Über

**Route:** `/about`  
**Komponente:** `About.vue`

### Zweck
Team-Credits (9 Mitglieder) und Forum-Link.

### Layout
- Team-Namen als Liste (`✰ Name`)
- Forum-Link als externe URL

---

## Screen 9: Scan (nur Mobile)

**Route:** `/scan`  
**Komponente:** `Scan.vue`

### Zweck
WLANThermo-Geräte im Netzwerk via mDNS (ZeroConf) entdecken und verbinden.

### Layout
- Geräteliste mit `vue-swipe-actions` (Swipe-to-Delete)
- Jedes Item: Gerätebild + Verbindungs-Dot + Name + IP + Info
- Demo-Eintrag immer sichtbar (nicht löschbar, kein Swipe)
- Toast: "Lost Connection..." bei Verbindungsabbruch

### Datenfluss
- ZeroConf Plugin: `_wlanthermo._tcp.` Discovery
- `GET http://{ip}/settings` → Gerät verifizieren + Typ ermitteln
- localStorage: Bekannte Geräte persistieren
- Verbindungsstatus: alle bekannten Geräte beim Start neu pingen

### Gerätebild-Mapping
| Typ | SVG-Asset |
|-----|-----------|
| nanov1/nanov2 | `nanov1.svg` |
| nanov3 | `nanov3.svg` |
| miniv1/miniv2 | `miniv2.svg` |
| miniv3 | `miniv3.svg` |
| linkv1 | `linkv1.svg` |
| bonev1 | `bonev1.svg` |
| demo | `demo.svg` |

---

## Layout-Templates

Alle Screens außer Home folgen demselben Template-Muster:

```html
<div class="pure-g">
  <!-- 1. App-Bar -->
  <div class="pure-u-1-1 app-bar-wrapper">
    <div class="app-bar-actions">
      <div class="button-container" @click="backToHome">← Zurück</div>
      <div class="button-container" @click="save">Speichern →</div>
    </div>
  </div>

  <!-- 2. Formular-Container -->
  <div class="config-form-container pure-u-1-1">
    <div class="name">Screen-Titel <span class="icon-question">?</span></div>
    <div class="config-form">
      <!-- Formular-Felder -->
    </div>
  </div>
</div>
```

Home-Screen: Kein `app-bar-wrapper` im Kachel-Modus; App-Bar erscheint nur im Edit-Modus.
