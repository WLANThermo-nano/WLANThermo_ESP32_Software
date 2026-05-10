# Asset-Referenzen

## Icon-System

**Typ:** Custom icomoon-Font (kein Icon-Framework wie FontAwesome)

**Font-Dateien:**
```
src/assets/icons/fonts/icomoon.ttf   ← Primär (für @font-face)
src/assets/icons/fonts/icomoon.woff  ← Web
src/assets/icons/fonts/icomoon.eot   ← IE Legacy
src/assets/icons/fonts/icomoon.svg   ← SVG-Font
```

**Font-Face Deklaration:**
```css
@font-face {
  font-family: 'icomoon';
  src: url('./fonts/icomoon.ttf') format('truetype');
  font-weight: normal;
  font-style: normal;
  font-display: block;
}
```

**Verwendung:**
```html
<span class="icon-home"></span>
<span class="icon-Wlan100"></span>
<span class="icon-battery-75"></span>
```

**Vollständige Icon-Liste:** siehe `src/assets/icons/style.scss` oder `01-design-tokens.json` → `icons.list`

**icomoon-Projektdatei:** `webui/font/WT - icomoon.io.json` (zum Hinzufügen neuer Icons auf icomoon.io)

---

## Logos (SVG, variantenspezifisch)

| Datei | Verwendung |
|-------|-----------|
| `src/assets/logo_nano.svg` | nano-Variante |
| `src/assets/logo_mini.svg` | mini-Variante |
| `src/assets/logo_link.svg` | link-Variante |
| `src/assets/logo_bone.svg` | bone-Variante |
| `src/assets/logo_cloud.svg` | cloud-Variante |
| `src/assets/logo_demo.svg` | Demo-Modus |
| `src/assets/logo_mobile.svg` | Mobile-App |

Logo-Einbindung in `App.vue`:
```js
logoImg: require(`@/assets/logo_${process.env.VUE_APP_PRODUCT_NAME}.svg`)
```

---

## Gerätebilder (SVG, für Scan-Screen)

| Datei | Gerätetyp |
|-------|----------|
| `src/assets/images/nanov1.svg` | NanoV1, NanoV2 |
| `src/assets/images/nanov3.svg` | NanoV3 |
| `src/assets/images/miniv2.svg` | MiniV1, MiniV2 |
| `src/assets/images/miniv3.svg` | MiniV3 |
| `src/assets/images/linkv1.svg` | LinkV1 |
| `src/assets/images/bonev1.svg` | BoneV1 |
| `src/assets/images/demo.svg` | Demo-Gerät |

---

## SCSS-Dateien (Quell-Assets)

| Datei | Inhalt |
|-------|--------|
| `src/assets/colors.scss` | SCSS-Variablen (6 Farben + 1 Input-Farbe) |
| `src/assets/global.scss` | Globale Stile + Imports aller SCSS |
| `src/assets/controls.scss` | Formular-Komponenten (floating labels, bar, etc.) |
| `src/assets/icons/style.scss` | @font-face + Icon-Klassen |
| `src/assets/icons/icons.scss` | Icon-Animationen + Farb-Modifier |
| `src/assets/external/pure.scss` | Pure CSS Framework (Grid, Buttons, Menu) |
| `src/assets/external/pure-responsive.scss` | Pure CSS Responsive Grid |
| `src/assets/external/normalized.scss` | CSS Reset |

---

## Favicon

`webui/public/favicon.ico` — Standard Browser-Tab Icon

---

## Build-Varianten Assets

Jede Variante bekommt ihr eigenes Logo über `VUE_APP_PRODUCT_NAME`.
Alle anderen Assets (Gerätebilder, Icons) sind variantenübergreifend gleich.

---

## Keine externen CDN-Assets

Die App ist vollständig **offline-fähig** (embedded auf ESP32). Es werden:
- **keine Google Fonts** geladen
- **keine CDN-Icons** (kein FontAwesome CDN etc.)
- **keine externen Skripte** oder Stile

Alle Assets sind im Bundle eingebettet.
