# Komponenten-Inventar

## Übersicht

| Komponente | Datei | Typ | Beschreibung |
|------------|-------|-----|-------------|
| `App` | `App.vue` | Root/Layout | Haupt-Layout: Nav, Header, Router-View, Modals |
| `Icon` | `components/Icon.vue` | UI-Atom | Wiederverwendbarer Icon-Wrapper |
| `Home` | `components/Home.vue` | Feature-Screen | Temperaturkanal-Kacheln + Kanal-Edit-Formular |
| `Wlan` | `components/Wlan.vue` | Feature-Screen | WiFi-Netzwerk-Verwaltung |
| `System` | `components/System.vue` | Feature-Screen | System-Einstellungen (Hostname, Sprache, Einheit) |
| `Bluetooth` | `components/Bluetooth.vue` | Feature-Screen | BLE-Geräte-Verwaltung |
| `Pitmaster` | `components/Pitmaster.vue` | Feature-Screen | PID-Regler-Einstellungen |
| `IoT` | `components/IoT.vue` | Feature-Screen | Cloud/MQTT-Einstellungen |
| `PushNotification` | `components/PushNotification.vue` | Feature-Screen | Push-Benachrichtigungen (Telegram, Pushover, App) |
| `About` | `components/About.vue` | Info-Screen | Team-Credits und Forum-Link |
| `Scan` | `components/Scan.vue` | Feature-Screen (Mobile) | Gerätescanner via ZeroConf/mDNS |
| `EmptyComponent` | `components/EmptyComponent.vue` | Placeholder | Leerer Platzhalter |

## Wiederverwendbare UI-Muster (keine eigenen Komponenten, aber globale CSS-Klassen)

### Globale Layout-Klassen

| Klasse | Beschreibung |
|--------|-------------|
| `.app-bar-wrapper` | Abstandshalter für die fixed App-Bar |
| `.app-bar-actions` | Fixierte Aktionsleiste (Zurück/Speichern) |
| `.config-form-container` | Äußerer Wrapper für Config-Formulare |
| `.config-form` | Innerer Form-Container mit Hintergrund |

### Formular-Pattern

| Klasse | Beschreibung |
|--------|-------------|
| `.form-group` | Floating-Label Input-Wrapper |
| `.form-group.error` | Fehler-State des Inputs |
| `.form-radio` | Radio-Button Gruppe |
| `.form-inline` | Inline-Formular |
| `.form-help` | Hilfstext unter Feld |
| `.form-checkbox` | Checkbox-Wrapper |
| `.control-label` | Floating Label |
| `.bar` | Animierter Bottom-Border |
| `.error-prompt` | Validierungs-Fehlermeldung |
| `.icon-form` | Icon-Overlay im Formularfeld |

### Interaktive Liste-Items

Dieses Pattern wird in `Wlan.vue`, `Bluetooth.vue` und `Scan.vue` verwendet:

| Klasse | Beschreibung |
|--------|-------------|
| `.wifi-item` / `.bluetooth-item` / `.scan-device-item` | Listen-Eintrag |
| `.expand` | Erweiterter State (Accordion) |
| `.info` | Flex-Container: Icon + Body |
| `.icon` | Icon-Spalte (flex: 0 0 3em) |
| `.body` | Inhalt-Spalte |
| `.name-address` | Name + Adresse Textblock |
| `.password-panel` / `.details-panel` | Ausgeklappter Inhalt |

## Globale Utility-Klassen

| Klasse | Wert |
|--------|------|
| `.cursor-pointer` | `cursor: pointer` |
| `.mr5` | `margin-right: 5px` |
| `.mt10` | `margin-top: 10px !important` |
| `.ic_white` | `color: #fff` |
| `.icon-disabled:before` | `color: #808080` |
| `.icon-red:before` | `color: #FF0000` |
| `.icon-green:before` | `color: #00FF00` |
| `.icon-yellow:before` | `color: #FFFF00` |
| `.icon-yellow` | `color: #ffff00` |
| `.icon-blinker` | Blink-Animation |
| `.icon-rotate-100/75/50/25` | Rotations-Animationen |

## Komponenten-Abhängigkeiten

```
App.vue
  └── Icon.vue

Home.vue
  (keine direkten Komponenten-Abhängigkeiten)

Wlan.vue
  └── Icon.vue

Bluetooth.vue
  └── Icon.vue

Pitmaster.vue
  (keine)

IoT.vue
  (keine)

PushNotification.vue
  (keine)

About.vue
  (keine)

Scan.vue (mobile only)
  └── SwipeList (vue-swipe-actions)
```

## Alle Screens nach Menü-Reihenfolge

| Menü-Icon | Route | Komponente | Feature-Flag |
|-----------|-------|-----------|-------------|
| `icon-home` | `/` | Home | — |
| `icon-Wlan100` | `/wlan` | Wlan | — |
| `icon-bluetooth_1` | `/bluetooth` | Bluetooth | `features.bluetooth` |
| `icon-cog` | `/system` | System | — |
| `icon-fire` | `/pitmaster` | Pitmaster | `features.pitmaster` |
| `icon-cloud` | `/iot` | IoT | — |
| `icon-bell` | `/notification` | PushNotification | — |
| `icon-info_sign` | `/about` | About | — |
| `icon-search` | `/scan` | Scan | **Nur Mobile** |
