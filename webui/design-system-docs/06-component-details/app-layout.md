# App Layout (Root-Komponente)

## Beschreibung
`App.vue` ist die Root-Komponente und implementiert das gesamte Layout: Sidebar-Navigation, Header-Bar, Router-View-Slot, Modal-Dialoge und Spinner. Alle Screens werden als `<router-view>` eingebettet.

## Datei-Pfad
`/webui/src/App.vue`

## Haupt-Layout-Bereiche

### 1. Sidebar Navigation (`#nav`)
- Breite: `200px` (Desktop), slide-in auf Mobile
- Hintergrund: `$medium` (#222831)
- Enthält: Logo (SVG, variantenspezifisch), SW-Version, Menü-Liste
- Aktiver Menüpunkt: weißer Hintergrund

**Menü-Items (je nach Variante gefiltert):**
| Icon | Label | Route |
|------|-------|-------|
| `icon-search` | Scan | `/scan` (nur Mobile) |
| `icon-home` | Home | `/` |
| `icon-Wlan100` | WLAN | `/wlan` |
| `icon-bluetooth_1` | Bluetooth | `/bluetooth` (wenn Feature aktiv) |
| `icon-cog` | System | `/system` |
| `icon-fire` | Pitmaster | `/pitmaster` (wenn Feature aktiv) |
| `icon-cloud` | IoT | `/iot` |
| `icon-bell` | Benachrichtigungen | `/notification` |
| `icon-info_sign` | Über | `/about` |

### 2. Header Bar (`.headmenu`)
- Höhe: `44px`, `position: fixed`, `z-index: 5`
- Hintergrund: `$medium`
- Links: Hostname (`settings.system.host`)
- Rechts: Status-Icons (Update-Hinweis, Cloud, Ladegerät, Akku, WiFi)

**Status-Icon-Logik:**
```js
// WiFi-Stärke → Icon-Klasse
dbm >= -80:  'Wlan100'
dbm >= -95:  'Wlan66'
dbm >= -105: 'Wlan33'

// Akkustand → Icon-Klasse
soc >= 90: 'battery-100'
soc >= 75: 'battery-75'
soc >= 50: 'battery-50'
soc >= 15: 'battery-25'
soc >= 10: 'battery-0 icon-red'
else:       'battery-0 icon-red icon-blinker'

// Cloud-Status → Icon-Klasse
online == 1: 'cloud icon-red'   (Fehler)
online == 2: 'cloud icon-green' (verbunden)
else: null                      (kein Icon)
```

**Toggle-Verhalten:**
- Klick auf WiFi-Icon: 5 Sekunden dBm-Wert anzeigen statt Icon
- Klick auf Akku-Icon: 5 Sekunden Prozent-Wert anzeigen statt Icon

### 3. Router-View (`#main`)
- `flex: 1 1 auto`
- `padding-top: 44px` (Platz für fixed Header)

### 4. Dialoge

#### Info/Hilfe-Dialog
- Getriggert via EventBus `'show-help-dialog'`
- Enthält: Titel, Text, optionaler Wiki-Link
- Schließen: Klick auf Mask oder ×-Button

#### Auth-Dialog (HTTP Basic Auth)
- Nur im Mobile-Modus sichtbar
- Benutzername + Passwort → Base64 → Axios-Header
- Breite: `40vw` (Desktop), `80vw` (Mobile)

#### Spinner (Loading-Overlay)
- Getriggert via EventBus `'loading', true/false`
- Halbransparente Maske + rotierendes CSS-Spinner-Element
- Spinner: 60×60px, `$primary`-farbige Border-Top, 0.6s Rotation

## Data-Polling

```js
// Alle 2 Sekunden /data abrufen
setInterval(() => this.getData(), 2000)

// getData() ruft /data ab und aktualisiert:
// - system (rssi, soc, charge, unit, online, ...)
// - channels (Temperaturkanäle)
// - pitmaster (PM-Daten)
```

## EventBus-Events (empfangen)

| Event | Beschreibung |
|-------|-------------|
| `show-help-dialog` | Info-Dialog mit `{title, content, wikiLink, linkText}` öffnen |
| `back-to-home` | Zur Startseite navigieren |
| `device-selected` | (Mobile) Gerät ausgewählt: Menü zurücksetzen, neu laden |
| `show-auth-popup` | (Mobile) Auth-Dialog öffnen mit Axios-Config zum Retry |
| `api-error` | (Mobile) Verbindung verloren: zurück zum Scan-Screen |
| `loading` | Spinner-Zustand setzen |
| `getData` | /data sofort abrufen |
| `getSettings` | /settings sofort abrufen |

## Mobile-Modus (`VUE_APP_PRODUCT_NAME === 'mobile'`)
- Startet auf `/scan` statt `/`
- Menü auf Scan + About reduziert
- Hostname: `$t('mobileAppHeader')`
- Auth-Dialog aktiv
- `cordova.InAppBrowser.open()` für Cloud-URLs

## Style-Details

```scss
#layout { display: flex; align-items: stretch; }
#nav { flex: 0 0 200px; min-height: 100vh; background: $medium; }
#main { flex: 1 1 auto; }

// Mobile: Nav wird zum Drawer
@media screen and (max-width: 48em) {
  #nav {
    position: fixed;
    left: -12.5em;
    transition: left .4s ease;
    &.active { left: 0; }
  }
}

// Dialog
.dialog {
  position: fixed;
  left: 50%;
  top: 15vh;
  transform: translateX(-50%);
  background-color: #fff;
  z-index: 500;
  .title { background-color: $primary; color: #fff; }
}
```
