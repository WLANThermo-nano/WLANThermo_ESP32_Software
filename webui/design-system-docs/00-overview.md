# WLANThermo Web UI — Design System Overview

## Projekt

WLANThermo ist ein WiFi-BBQ-Thermometer-Controller. Die Web UI ist eine embedded Vue 2 SPA, die auf dem ESP32 gehostet wird und per gzip-komprimierter HTML/JS/CSS-Datei ausgeliefert wird.

## Tech Stack

| Kategorie | Technologie | Version |
|-----------|-------------|---------|
| Framework | Vue 2 | 2.6.11 |
| Routing | Vue Router | 3.x |
| i18n | Vue I18n | 8.x (DE, EN) |
| Validierung | Vuelidate | 0.7.5 |
| HTTP | Axios | 0.21.1 |
| CSS Framework | Pure CSS | embedded SCSS |
| Icons | icomoon (custom font) | custom |
| CSS Präprozessor | SCSS (sass 1.26) | — |
| State Management | **keines** (EventBus-Pattern) | — |
| Build | Vue CLI 4.4 | — |

## Design-Philosophie

- **Dark Theme**: Dunkle Farbpalette mit Teal-Akzent (`#00adb5`)
- **Mobile-First Responsiveness**: Ab `48em` (768px) klappt die Navigation ein
- **Material-inspirierte Formulare**: Floating Labels, Bottom-Border-Animation
- **Kompaktes UI**: Minimale Chrome, maximaler Content-Space (wichtig für embedded-Device)
- **Multi-Variant Build**: 7 Hardware-Varianten (nano, mini, link, bone, cloud, demo, mobile) mit demselben Code-Basis und varianten-spezifischen Assets (Logo, Scan-Component)

## Architektur

```
App.vue (Root)
├── Sidebar Navigation (fixed, 200px)
│   ├── Logo (variantenspezifisch)
│   ├── SW-Version
│   └── Menu Items (pure-menu-list)
├── Header Bar (fixed, 44px)
│   ├── Hostname
│   └── Status Icons (WiFi, Battery, Cloud, Update)
├── router-view (Haupt-Content)
│   ├── Home.vue — Temperaturkanal-Kacheln
│   ├── Wlan.vue — WiFi-Verwaltung
│   ├── System.vue — System-Einstellungen
│   ├── Bluetooth.vue — BLE-Geräte
│   ├── Pitmaster.vue — PID-Regler
│   ├── IoT.vue — Cloud/MQTT
│   ├── PushNotification.vue — Push-Benachrichtigungen
│   ├── About.vue — Credits
│   └── Scan.vue — Gerätescanner (nur Mobile)
├── Dialog (modal overlay)
├── Auth Dialog (Basic Auth)
└── Spinner (loading overlay)
```

## Kommunikations-Patterns

- **EventBus** (`src/event-bus.js`): Globaler Vue-EventBus für komponentenübergreifende Events
  - `show-help-dialog` — Hilfe-Dialog öffnen
  - `back-to-home` — Zurück zur Startseite
  - `device-selected` — Gerät ausgewählt (Mobile)
  - `loading` — Spinner ein/aus
  - `getData` / `getSettings` — Daten neu laden
- **Props**: `channels`, `pitmasterpm`, `unit` von `App.vue` an `Home.vue`
- **Axios**: Alle API-Calls via `this.axios` (Vue-Axios Plugin), Basis-URL variabel (mobile)

## Build-Varianten

| Variante | Produkt-Name | Router-Mode | Besonderheit |
|----------|-------------|-------------|--------------|
| nano | nano | hash | Standard |
| mini | mini | hash | Standard |
| link | link | hash | Standard |
| bone | bone | hash | Standard |
| cloud | cloud | hash | Standard |
| demo | demo | hash | Mock-APIs |
| mobile | mobile | hash | Cordova, Scan-Component, ZeroConf |

## Asset-Struktur

```
src/assets/
├── colors.scss          ← SCSS-Variablen (Farben)
├── global.scss          ← Globale Stile, importiert alle SCSS
├── controls.scss        ← Formular-Komponenten
├── icons/
│   ├── style.scss       ← @font-face + Icon-Klassen
│   ├── icons.scss       ← Icon-Animationen + Farb-Modifier
│   └── fonts/           ← icomoon.ttf/woff/eot/svg
├── external/
│   ├── pure.scss        ← Pure CSS framework
│   ├── pure-responsive.scss
│   └── normalized.scss
└── images/              ← Gerätebild-SVGs (nanov1-3, miniv2-3, linkv1, bonev1)
```
