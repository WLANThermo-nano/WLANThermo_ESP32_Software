# Farbsystem

## Überblick

Das WLANThermo UI verwendet ein **dunkles Farbschema** mit einem Teal-Akzent. Die Farbpalette ist in `src/assets/colors.scss` definiert.

## SCSS-Variablen (Quelle: `src/assets/colors.scss`)

```scss
$medium: #222831;         // Nav-Hintergrund, Header
$light: #eeeeee;          // Helle Text-Farbe (selten)
$medium_dark: #474a4e;    // Card-/Form-Hintergrund
$dark: #393e46;           // Body-Hintergrund, Hover-States
$primary: #00adb5;        // Teal — Buttons, Akzente, Spinner
$error_color: #f44336;    // Rot — Fehler, Löschen
$input_highlight_color: #337ab7; // Blau — Input-Fokus, Aktuelles WiFi
```

## Farbpalette

### Hintergrund-Hierarchie (dunkel → heller)

| Name | Hex | SCSS-Variable | Verwendung |
|------|-----|---------------|-----------|
| Navigation | `#222831` | `$medium` | Sidebar, Header-Bar, Hamburger-Menu |
| Body | `#393e46` | `$dark` | Seiten-Hintergrund, Hover-States |
| Surface | `#474a4e` | `$medium_dark` | Cards (info-box), Config-Forms |
| Overlay | `rgba(0,0,0,0.3)` | — | Dialog-Masken |
| Dialog | `#ffffff` | — | Modal-Dialoge |

### Akzent-Farben

| Name | Hex | Verwendung |
|------|-----|-----------|
| Primary / Teal | `#00adb5` | Pure-Button Primary, Spinner, Dialog-Header, App-Bar Buttons |
| Input Highlight | `#337ab7` | Input-Focus-Border, Label-Farbe bei Fokus, Aktuelles WLAN-Netzwerk |
| Error / Rot | `#f44336` | Fehler-Buttons, Validation-Fehler, Swipe-Delete, Batterie-kritisch |
| Info / Blau | `#3366ff` | Wiki-Links in Hilfe-Dialogen |

### Text-Farben

| Farbe | Hex | Kontext |
|-------|-----|---------|
| Primärer Text | `#ffffff` | Formular-Inhalte, Kanalname, Gerätenamen |
| Sekundärer Text | `#cccccc` | Header-Bar Text |
| Muted | `#b3b3b3` | Hilfs-Text (form-help), inaktive Labels |
| Deaktiviert | `#808080` | Disabled Icons (`icon-disabled`), Label-Farbe nach Fokus |
| Label (inaktiv) | `#b3b3b3` | Floating Label Default-State |
| Label (Fokus) | `#337ab7` | Floating Label Fokus-State |

### Semantische Farben

| Zustand | Farbe | Hex | Verwendung |
|---------|-------|-----|-----------|
| Fehler | Rot | `#f44336` | Validation, Fehlerzustand |
| Warnung | Gelb | `#FFFF00` | Update-Hinweis (icon-yellow) |
| Erfolg / Verbunden | Grün | `#00FF00` / `lightgreen` | Cloud verbunden, Connection-Dot |
| Nicht verbunden | Rot | `red` | Connection-Dot offline |
| Zu heiß | Rot | `#ff0000` | Temperatur über Maximum |
| Zu kalt | Blau | `#1874cd` | Temperatur unter Minimum |

### Kanal-Farbpalette (19 Farben für Temperaturkanäle)

Diese Farben werden in `Home.vue` als auswählbare Kanalfarben angeboten:

```
#FFFF00  #FFC002  #00FF00  #FFFFFF  #FF1DC4
#E46C0A  #C3D69B  #0FE6F1  #0000FF  #03A923
#C84B32  #FF9B69  #5082BE  #FFB1D0  #A6EF03
#D42A6B  #FFDA8F  #00B0F0  #948A54
```

## Farbverwendung nach Kontext

### Navigation (`#nav`)
- Hintergrund: `$medium` (#222831)
- Links: weiß/transparent, hover: `$medium_dark`
- Aktiver Menüpunkt: weiß (`#fff`) Hintergrund
- Icon-Farben: weiß

### Header Bar (`.headmenu`)
- Hintergrund: `$medium` (#222831)
- Text: `#ccc`
- Status-Icons: `$primary` / Icon-Modifier-Klassen

### Cards / Info-Boxes (`.info-box`)
- Hintergrund: `$medium_dark` (#474a4e)
- Linker Rand: dynamisch (Kanalfarbe)
- Text: `#fff`
- Temperatur normal: `#fff`
- Temperatur zu heiß: `#ff0000` (fett)
- Temperatur zu kalt: `#1874cd` (fett)

### Config-Formulare (`.config-form`)
- Hintergrund: `$medium_dark`
- Label normal: `#b3b3b3`
- Label aktiv: `gray`
- Label Fokus: `$input_highlight_color` (#337ab7)
- Input-Text: `#fff`
- Border-Bottom: `#999`
- Border-Bottom Fokus: `$input_highlight_color`

### Dialoge (`.dialog`)
- Hintergrund: `#fff`
- Titel-Hintergrund: `$primary` (#00adb5)
- Titel-Text: `#fff`
- Body-Text: Standard (schwarz)

### Buttons (Pure CSS)
- `pure-button-primary`: Hintergrund `$primary` (#00adb5)
- `pure-button-error`: Hintergrund `$error_color` (#f44336), Text `#fff`
- Standard-Button: Pure CSS Default (grau)

## Dark-Mode Hinweis

Das UI ist ausschließlich als Dark-Mode konzipiert. Es gibt keinen Light-Mode und kein `prefers-color-scheme`-Handling. Der Body hat immer `background-color: $dark`.
