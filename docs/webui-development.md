# Web Interface — Entwicklerdokumentation

Das WLANThermo Web-Interface ist eine Vue 3 Single-Page-Application (SPA), die als einzelne
gzip-komprimierte HTML-Datei in die Firmware eingebettet wird. Kein Webserver nötig — der
ESP32 liefert die Datei direkt aus dem Flash.

---

## Voraussetzungen

| Tool | Mindestversion | Hinweis |
|------|---------------|---------|
| Node.js | 18.x | Node 22 funktioniert ebenfalls |
| npm | 9.x | yarn **nicht** verwenden (TLS-Zertifikat-Probleme) |

```bash
node --version   # >= 18
npm --version    # >= 9
```

---

## Einrichtung

```bash
cd webui
npm install --legacy-peer-deps
```

> `--legacy-peer-deps` ist nötig weil `@vue/cli-service 5.x` noch `eslint 8.x`
> als Peer-Dep erwartet, während neuere Plugins schon `eslint 9.x` fordern.

---

## Dev-Server (ohne echtes Gerät)

```bash
cd webui
npm run serve
# → http://localhost:8080
```

Der Dev-Server läuft im **Demo-Modus** (`.env.development`): alle API-Aufrufe werden
durch einen axios-Adapter mit Testdaten abgefangen — kein echtes WLANThermo nötig.

Testdaten liegen in `src/demo/mocks/data.js`. Dort sind Kanäle, Pitmaster-Konfigurationen,
WLAN-Netzwerke, Bluetooth-Geräte usw. hinterlegt.

### Dev-Server gegen ein echtes Gerät

Wenn ein WLANThermo im Netzwerk erreichbar ist, kann der Proxy umgeleitet werden.
`vue.config.js` anpassen:

```js
devServer: {
    proxy: {
        '/': {
            target: 'http://192.168.x.x',  // IP des Geräts
            ws: false,
            bypass: function(req) {
                if (req.headers.accept && req.headers.accept.includes('text/html')) {
                    return '/index.html'
                }
            }
        }
    }
}
```

Außerdem in `src/main.js` die Mock-Zeile deaktivieren:

```js
// require(process.env.VUE_APP_DEMO_FILE_PATH)  // auskommentieren für echtes Gerät
```

---

## Production-Build

### Einzelne Variante

```bash
npm run build-mini    # → webui/dist/mini/index.html.gz  (~124 KB)
npm run build-nano    # → webui/dist/nano/index.html.gz  (~125 KB)
npm run build-link    # → webui/dist/link/index.html.gz  (~125 KB)
npm run build-bone    # → webui/dist/bone/index.html.gz  (~128 KB)
npm run build-cloud   # → webui/dist/cloud/index.html.gz (~125 KB)
npm run build-demo    # → webui/dist/demo/index.html.gz  (~130 KB)
```

### Alle Varianten auf einmal

```bash
npm run build
```

### Warum eine einzige .gz-Datei?

Das Build-System inlinet alle JavaScript- und CSS-Dateien in eine einzelne `index.html`
(via `@effortlessmotion/html-webpack-inline-source-plugin`). Diese wird anschließend
gzip-komprimiert. Das Ergebnis ist eine selbst-enthaltende Datei, die der ESP32 direkt
aus dem Flash ausliefert — kein Dateisystem, kein SPIFFS.

---

## Varianten und Firmware-Zuordnung

Jede Hardware-Variante bekommt ihr eigenes Web-UI-Build:

| Firmware-Env | Web-UI-Variante | Besonderheiten |
|-------------|-----------------|----------------|
| `miniV1`, `miniV2`, `miniV3`, `connectV1` | `mini` | Pitmaster + Bluetooth |
| `nanoV3` | `nano` | Pitmaster + Bluetooth |
| `linkV1` | `link` | kein Pitmaster |
| `boneV1` | `bone` | kein Pitmaster |

Die Funktionsflags (`pitmaster`, `bluetooth`) kommen **nicht** vom Build, sondern vom Gerät:
`/settings → features.pitmaster` und `features.bluetooth`. Die App filtert Menüeinträge
dynamisch basierend auf der Geräte-Antwort. Das Build enthält immer alle Seiten.

---

## Projektstruktur

```
webui/
├── public/
│   └── index.html          # HTML-Template (enthält <div id="app">)
├── src/
│   ├── main.js             # App-Bootstrap: Vue 3, Router, i18n, axios
│   ├── App.vue             # Root-Komponente: Navigation, Polling, EventBus
│   ├── event-bus.js        # mitt-basierter globaler EventBus
│   ├── router/index.js     # Vue Router 4 — alle Routen
│   ├── i18n/
│   │   ├── de.js           # Deutsche Übersetzungen
│   │   └── en.js           # Englische Übersetzungen
│   ├── assets/
│   │   ├── colors.scss     # Farb-Variablen
│   │   ├── controls.scss   # Formular-Styles
│   │   ├── global.scss     # Globale Styles
│   │   └── logo_*.svg      # Logo pro Variante
│   ├── components/
│   │   ├── Home.vue        # Kanalübersicht + Kanal-Editor
│   │   ├── Pitmaster.vue   # Pitmaster-Konfiguration
│   │   ├── Wlan.vue        # WLAN-Einstellungen
│   │   ├── System.vue      # Systemeinstellungen
│   │   ├── Bluetooth.vue   # Bluetooth-Geräte
│   │   ├── IoT.vue         # Cloud/IoT-Einstellungen
│   │   ├── PushNotification.vue
│   │   ├── Scan.vue        # Gerätesuche (nur mobile App)
│   │   ├── About.vue       # Über / Firmware-Info
│   │   └── Icon.vue        # Icon-Hilfskomponente
│   ├── demo/
│   │   ├── mock-apis.js    # axios-Adapter für Demo-Modus
│   │   ├── mock-data.js    # Mock-Response-Router
│   │   └── mocks/data.js   # Statische Testdaten
│   └── helpers/
│       └── icons-helper.js # WiFi/Batterie-Icon-Logik
├── vue.config.js           # webpack 5 Konfiguration
├── package.json
└── .env.*                  # Varianten-spezifische Umgebungsvariablen
```

---

## API-Endpunkte

Das Web-Interface kommuniziert ausschließlich per HTTP mit dem Gerät:

| Methode | Endpunkt | Beschreibung |
|---------|----------|-------------|
| GET | `/data` | Temperaturen, Pitmaster-Status (gepolt alle 2 s) |
| GET | `/settings` | Geräteeinstellungen, Features, PID-Profile |
| GET | `/networklist` | Gefundene WLAN-Netzwerke |
| GET | `/getbluetooth` | Bluetooth-Gerätekonfiguration |
| GET | `/getpush` | Push-Notification-Einstellungen |
| POST | `/setchannels` | Kanal-Einstellungen speichern |
| POST | `/setsystem` | Systemeinstellungen speichern |
| POST | `/setwlan` / `/setnetwork` | WLAN-Konfiguration |
| POST | `/setpitmaster` | Pitmaster-Einstellungen |
| POST | `/setpid` | PID-Profile speichern |
| POST | `/setbluetooth` | Bluetooth-Konfiguration |
| POST | `/setpush` | Push-Konfiguration |
| POST | `/setIoT` | IoT/Cloud-Konfiguration |
| POST | `/newtoken` | API-Token neu generieren |
| POST | `/networkscan` | WLAN-Scan starten |
| POST | `/stopwifi` / `/clearwifi` | WLAN-Verbindung trennen |
| POST | `/rotate` | Display-Rotation (Neustart) |
| POST | `/calibrate` | Touch-Kalibrierung |
| POST | `/update` | Firmware-Update starten |

---

## Technologie-Stack

| Paket | Version | Rolle |
|-------|---------|-------|
| Vue | 3.4 | UI-Framework (Options API) |
| Vue Router | 4.3 | Client-seitiges Routing |
| vue-i18n | 9.x (legacy mode) | Internationalisierung (DE/EN) |
| vue-axios | 3.5 | axios-Integration |
| axios | 1.7 | HTTP-Client |
| mitt | 3.0 | Globaler EventBus |
| @vuelidate/core | 2.x | Formularvalidierung |
| @vue/cli-service | 5.x | Build-Tool (webpack 5) |
| sass | 1.x | SCSS-Compiler |

---

## Bekannte Stolperfallen (Vue 3 Migration)

### `v$` statt `$v` für Vuelidate

Vue 3 blockiert Properties aus `setup()`, deren Name mit `$` beginnt — sie sind für
Vue-interne Verwendung reserviert. `this.$v` wäre immer `undefined`.

```js
// FALSCH (Vue 3)
setup() { return { $v: useVuelidate() } }

// RICHTIG
setup() { return { v$: useVuelidate() } }
```

Im Template entsprechend `v$.fieldName.$invalid` statt `$v.fieldName.$invalid`.

### Vue Router 4: Props an `<router-view>` übergeben

Vue Router 4 leitet Props, die direkt auf `<router-view>` gesetzt werden, nicht mehr
an die gerenderte Komponente weiter. Stattdessen `v-slot` verwenden:

```html
<!-- FALSCH (Vue Router 3) -->
<router-view :channels="channels" :unit="unit" />

<!-- RICHTIG (Vue Router 4) -->
<router-view v-slot="{ Component }">
  <component :is="Component" :channels="channels" :unit="unit" />
</router-view>
```

### axios 1.x: POST-Body ist ein JSON-String

In axios 1.x wird der POST-Body vor der Übergabe an den Adapter automatisch
JSON-serialisiert. Im Mock (und in eigenen Adaptern) muss `config.data` geparst werden:

```js
const body = typeof config.data === 'string' ? JSON.parse(config.data) : config.data
```

### Dev-Server: Inline-Source-Plugin nur für Production

Das Plugin `@effortlessmotion/html-webpack-inline-source-plugin` inlinet alle Scripts in
den `<head>`. Im Dev-Modus würde das dazu führen, dass `app.mount('#app')` läuft bevor
`<div id="app">` im DOM existiert. Deshalb ist das Plugin in `vue.config.js` auf
`NODE_ENV === 'production'` beschränkt.

---

## Firmware-Integration

Nach einem Web-UI-Build muss die Firmware neu gebaut werden, damit das neue Interface
eingebettet wird:

```bash
# 1. Web-UI bauen
cd webui && npm run build-mini

# 2. Firmware bauen (beinhaltet das neue Web-UI automatisch)
cd ..
~/.platformio/penv/bin/pio run -e miniV3

# 3. Flashen
~/.platformio/penv/bin/pio run -e miniV3 -t upload
```

Der PlatformIO-Build-Script `extra_script.py` übernimmt automatisch:
- Komprimierung der `index.html` → `index.html.gz`
- Einbettung als C-Array in den Firmware-Binary (`board_build.embed_files`)
- Der ESP32 liefert die Datei über den Endpunkt `GET /` aus

---

## Übersetzungen hinzufügen

Neue UI-Texte in beide Dateien eintragen:

```
src/i18n/de.js   ← Deutsch
src/i18n/en.js   ← Englisch
```

Im Template mit `{{ $t('schluessel') }}` verwenden.
Die Sprache wird automatisch aus `settings.system.language` des Geräts gesetzt.

---

## Icons

Die App verwendet einen Custom Icon-Font (IcoMoon). Alle verfügbaren Icons sind in
`src/icons/demo.html` sichtbar (im Browser öffnen). Verwendung im Template:

```html
<span class="icon icon-home"></span>
<span class="icon-wifi_1"></span>
```

Neue Icons über IcoMoon hinzufügen und die generierten Dateien in `src/icons/fonts/`
sowie `src/icons/style.css` aktualisieren.
