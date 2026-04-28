# Spacing & Layout

## Layout-Grundstruktur

```
┌─────────────────────────────────────────────────────┐
│  HEADER BAR (fixed, 44px, z-index: 5)               │
│  #222831 | Hostname             WiFi Battery Cloud   │
├──────────┬──────────────────────────────────────────┤
│          │  MAIN CONTENT (flex: 1)                  │
│          │  padding-top: 44px                       │
│  NAV     │                                          │
│  (fixed) │  <router-view>                           │
│  200px   │                                          │
│  #222831 │                                          │
│          │                                          │
│  Logo    │                                          │
│  Version │                                          │
│  Menu    │                                          │
│          │                                          │
└──────────┴──────────────────────────────────────────┘
```

### Desktop (> 48em / 768px)
- Navigation: `flex: 0 0 200px`, permanent sichtbar, min-height: 100vh
- Main: `flex: 1 1 auto`, voller verbleibender Platz
- Header-Bar: `width: calc(100% - 200px)`, `left: 200px`

### Mobile (≤ 48em / 768px)
- Navigation: `position: fixed`, `left: -12.5em` (außerhalb), per Hamburger-Button einblendbar
- Navigation aktiv: `left: 0`, `z-index: 500`
- Nav-Mask: Halbtransparentes Overlay hinter Nav (z-index: 499)
- Header-Bar: `width: 100%`, `left: 0`
- Hamburger-Button eingeblendet (`.menu-link { display: block }`)

## Grid-System

Das Projekt nutzt **Pure CSS Grid** (`pure-g`, `pure-u-*`):

| Klasse | Bedeutung |
|--------|-----------|
| `pure-g` | Grid-Container |
| `pure-u-1` | 100% Breite |
| `pure-u-1-1` | 100% Breite (explizit) |
| `pure-u-1-2` | 50% Breite |
| `pure-u-1-3` | 33.33% Breite |
| `pure-u-2-5` | 40% Breite |
| `pure-u-1-5` | 20% Breite |
| `pure-u-md-1-2` | 50% ab mittlerer Breite |
| `pure-u-xl-1-4` | 25% ab extra-large |

### Typische Grid-Patterns

**Home — Kanal-Kacheln:**
```html
<div class="pure-g">
  <div class="pure-u-1 pure-u-md-1-2 pure-u-xl-1-4">
    <!-- 1 Spalte auf Mobile, 2 auf Tablet, 4 auf Desktop -->
  </div>
</div>
```

**Config-Formulare — Vollbreite:**
```html
<div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
```

**2-Spalten Formular-Felder (Pitmaster, IoT):**
```html
<div class="pure-u-1-2 control">
  <div class="form-group">...</div>
</div>
```

**3-Spalten Formular-Felder (PID-Werte Kp/Ki/Kd):**
```html
<div class="pure-u-1-3 control">
```

## Spacing-Werte

### Formular-Abstände
| Klasse / Property | Wert | Kontext |
|------------------|------|---------|
| `.form-group` margin-top | `1.25rem` | Standard Formularfeld-Abstand oben |
| `.form-group` margin-bottom | `1.55rem` | Standard Formularfeld-Abstand unten |
| `.form-group` (in Grid) margin-top | `0.75rem` | Formularfeld im 2/3-Spalten-Grid |
| `.form-group` (in Grid) margin-bottom | `0.85rem` | |
| `.form-group.select-form-group` margin-top | `0.25rem` | Verringerter Abstand bei Select |
| `.form-checkbox` margin-top | `1.25rem` | |
| `.form-checkbox` margin-bottom | `1rem` | |
| `.config-form` padding | `0.25rem 0.5rem 0.75rem` | Formular-Container |
| `.config-form` margin | `0.25rem` | |

### Karten / Listen
| Element | Wert |
|---------|------|
| `.info-box` margin | `5px` |
| `.info-box` padding | `10px` |
| `.info-box` height | `70px` |
| `.wifi-item` padding | `0.6em` |
| `.bluetooth-item` padding | `0.6em` |
| `.scan-device-item` padding | `0.6em` |

### App-Bar
| Element | Wert |
|---------|------|
| `.app-bar-actions` height | `1.7em` (≈ font-size × 1.7) |
| `.app-bar-actions` `.button-container` padding | `0.2em` |
| `.app-bar-wrapper` margin-bottom | `1.9em` |

### Sonstiges
| Klasse | Wert |
|--------|------|
| `.mr5` | `margin-right: 5px` |
| `.mt10` | `margin-top: 10px !important` |
| `.pure-menu-link` padding | Pure CSS Default |
| `.form-section-name` padding | `0.3em` |
| `.form-section-name` margin-top | `0.3em` |

## Container-Breiten

- Kein max-width auf Hauptinhalt
- Config-Formulare: `pure-u-1-1` (100% im Grid)
- Dialog: `position: fixed; left: 50%; transform: translateX(-50%)` — zentriert, keine feste Breite
- Auth-Dialog Body: `width: 40vw` (Desktop), `80vw` (Mobile ≤ 48em)
- Nav: `200px` (Desktop), `200px / 12.5em` (Mobile, slide-in)

## Flexbox-Patterns

### Layout-Ebene (`#layout`)
```scss
#layout {
  display: flex;
  align-items: stretch;
}
```

### Header-Bar (`.headmenu`)
```scss
display: flex;
.title { flex: 1 1 auto; }     // nimmt verfügbaren Platz
.status { flex: 0 0 auto; }    // nur Inhaltsbreite
```

### Info-Box (`Home.vue`)
```scss
.info-box {
  display: flex;
  flex-direction: column;
  .title-row {
    display: flex;
    .name { flex: 1 1 auto; }
    .number { flex: 0 0 auto; }
  }
  .body-row {
    display: flex;
    flex: 1 1 auto;
    .temperature-range { flex: 1 1 auto; flex-direction: column; }
    .temperature { flex: 0 0 auto; }
  }
}
```

### Listen-Items (WiFi, Bluetooth, Scan)
```scss
.wifi-item .info {
  display: flex;
  .icon { flex: 0 0 3em; }
  .body { flex: 1 1 auto; display: flex; justify-content: space-between; }
}
```

## Responsive Breakpoints

| Name | Wert | Auswirkung |
|------|------|-----------|
| Mobile | `max-width: 48em` (768px) | Nav wird zum Slide-in Drawer, Hamburger sichtbar, Header 100% Breite |
| Tablet (Pure) | `min-width: 35.5em` | `md`-Grid-Spalten aktiv |
| Large (Pure) | `min-width: 48em` | `lg`-Grid-Spalten aktiv |
| XLarge (Pure) | `min-width: 80em` | `xl`-Grid-Spalten aktiv |

## Pure CSS Buttons (genutzte Klassen)

```scss
.pure-button               // Basis-Button (Pure CSS Default)
.pure-button-primary       // Teal (#00adb5) — primäre Aktionen
.pure-button-error         // Rot (#f44336) — Löschen/Fehlerzustand
```

## Z-Index-Hierarchie

| Layer | Z-Index | Element |
|-------|---------|---------|
| App-Bar / Nav-Icon | 5 | `.app-bar-actions`, `.app-bar-wrapper` |
| Mobile Nav | 10 | `.menu-link` |
| Overlay-Masken | 499 | `.nav-mask`, `.dialog-mask` |
| Dialoge / Mobile Nav | 500 | `.dialog`, `#nav.active` |
