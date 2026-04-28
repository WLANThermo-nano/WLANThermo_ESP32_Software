# TFT Display — Design System Dokumentation

Extrahiert aus `src/display/tft/` — LVGL 7 Implementation auf ILI9341 TFT (320×240 px).

---

## Stack-Überblick

| Aspekt | Wert |
|--------|------|
| Framework | LVGL 7.11 |
| Display | ILI9341 TFT, 320×240 px |
| Renderer | TFT_eSPI |
| Touch | XPT2046 (kalibriert via NVS "TFT") |
| Theme | Custom `lvTheme.cpp` (DARK + NO_FOCUS) |
| Brightness | I2C `0x0D` + PCA9533 (alt); 0–100% → 0–255 |
| Update-Rate | TASK_CYCLE_TIME_DISPLAY_FAST_TASK (~5 ms) |

---

## Farb-System (Dark Mode)

Die Initialisierung in `DisplayTft.cpp:121`:
```cpp
lvTheme_Init(lv_color_hex(0x0aa5c4), lv_theme_get_color_secondary(),
             LVTHEME_FLAG_DARK | LVTHEME_FLAG_NO_FOCUS, ...)
```

### Theme-Farben (Dark Mode)

| Token | Hex | Verwendung |
|-------|-----|-----------|
| `COLOR_SCR` | `#181d23` | Screen-Hintergrund, Page-BG |
| `COLOR_SCR_TEXT` | `#e7e9ec` | Screen-Text |
| `COLOR_BTN` | `#0aa5c4` | Button-Hintergrund (Primary) |
| `COLOR_BG` | `#29313a` | Container, Tiles, Header |
| `COLOR_BG_PRESSED` | `#494f57` | Button gedrückt |
| `COLOR_BG_SEC` | `#45494d` | Sekundärer Container |
| `COLOR_BG_TEXT` | `#ffffff` | Text auf Container |
| `COLOR_BG_SEC_TEXT` | `#a5a8ad` | Text auf sekundärem Container |
| `COLOR_BG_BORDER` | `#808a97` | Container-Rahmen |
| `LV_COLOR_WHITE` | `#ffffff` | Label-Text (WiFi, Tiles) |
| `LV_COLOR_RED` | `#ff0000` | Stop-Button (Pitmaster) |

### Home Screen spezifisch

| Token | Hex | Verwendung |
|-------|-----|-----------|
| `LVHOME_COLOR_BG` | `#181d23` | Page-Hintergrund (= COLOR_SCR) |
| `LVHOME_COLOR_TILE` | `#29313a` | Sensor-Kachel-BG (= COLOR_BG) |
| Alarm: kein Alarm | `LV_COLOR_WHITE` | Temperaturanzeige normal |
| Alarm: im Bereich | `#00BFFF` (LightBlue) | Temperatur innerhalb Min/Max |
| Alarm: Alarm | `LV_COLOR_RED` | Temperatur außerhalb Grenzwerte |

### Vergleich Web UI ↔ TFT

| Aspekt | Web UI | TFT |
|--------|--------|-----|
| Primary/Akzent | `#00adb5` | `#0aa5c4` |
| Body-BG | `#393e46` | `#181d23` |
| Surface/Tiles | `#474a4e` | `#29313a` |
| Nav-BG | `#222831` | `#29313a` (Header) |
| Text primär | `#ffffff` | `#e7e9ec` |
| Alarm heiß | `#ff0000` | `#ff0000` |
| Alarm kalt | `#1874cd` | `#00bfff` |

---

## Typografie

### Embedded Font-Dateien

| Datei | Familie | Größe | Verwendung |
|-------|---------|-------|-----------|
| `Font_Roboto_Medium_h22.c` | Roboto Medium | 22 px | Temperaturwert auf Home-Kacheln |
| `Font_Roboto_Medium_h28.c` | Roboto Medium | 28 px | Roller-Auswahl (Sensor-Typ, Kanal) |
| `Font_Roboto_Medium_h80.c` | Roboto Medium | 80 px | Spinbox-Eingabe (Min/Max/Target) |
| `Font_Roboto_Regular_h14.c` | Roboto Regular | 14 px | Min/Max-Werte auf Kacheln |
| `Font_Roboto_Regular_h16.c` | Roboto Regular | 16 px | Kanalname + Kanalnummer auf Kacheln |
| `Font_Gothic_A1_Medium_h16.c` | Gothic A1 Medium | 16 px | Menu-Buttons, WiFi-Labels |
| `Font_Gothic_A1_Medium_h21.c` | Gothic A1 Medium | 21 px | (reserviert) |
| `Font_Nano_h24.c` | Nano (Symbol) | 24 px | Header-Buttons, Tab-Icons, Switch-Labels |
| `Font_Nano_h40.c` | Nano (Symbol) | 40 px | (reserviert, größere Symbole) |
| `Font_Nano_Temp_Limit_h14.c` | Nano Temp Limit | 14 px | Min/Max-Pfeile auf Kacheln |

### Nano Symbol-Font Zeichen-Mapping

| Zeichen | Symbol | Verwendung |
|---------|--------|-----------|
| `"f"` | Menü-Hamburger | Header-Button links |
| `"S"` | Pfeil links | Home-Navigation |
| `"Q"` | Pfeil rechts | Home-Navigation |
| `"J"` / `"}"` / `"{"` / `"~"` | Fan-Animation | Pitmaster aktiv (4 Frames) |
| `"o"` | Alarm-Glocke | Alarm-Button |
| `"h"` | Cloud | Cloud-Status |
| `"l"` | AP-WiFi | Access-Point-Modus |
| `"I"` / `"H"` / `"G"` | WiFi 1/2/3 Balken | WiFi-Signalstärke |
| `"v"` – `"\|"` | Batterie 0–100% | Entladung (mehrere Zeichen) |
| `"s"` | Batterie voll | Eingesteckt |
| `"u"` | Lade-Symbol | Lädt |
| `"p"` | Push-Symbol | Benachrichtigung-Switch |
| `"q"` | Buzzer-Symbol | Buzzer-Switch |
| `"r"` | Display-Symbol | Display-Tab |
| `"t"` | Helligkeit | Brightness-Slider-Label |
| `"X"` | Download/Update | OTA Progress |
| `"0"` | Stop | Pitmaster Stop Tab |

---

## Layout & Dimensionen

### Display

- **Auflösung:** 320 × 240 px (Landscape)
- **Rotation:** `tft.setRotation(1)` — 90° rotiert

### Home Screen Layout

```
┌─────────────────────────────────────┐ ← 320 px
│  Header Container (320 × 40 px)     │ y=0
│  [☰ menu] [←] [pitmaster] [→] [⚙]  │
│  bg: #29313a                         │
├─────────────────────────────────────┤ y=40
│  Grid (LV_LAYOUT_GRID)              │
│  pad_inner=4, pad_left/right=2      │
│  ┌───────────┐ ┌───────────┐        │
│  │ Kachel 1  │ │ Kachel 2  │        │
│  │ 156 × 63  │ │ 156 × 63  │        │
│  └───────────┘ └───────────┘        │
│  ┌───────────┐ ┌───────────┐        │
│  │ Kachel 3  │ │ Kachel 4  │        │
│  └───────────┘ └───────────┘        │
└─────────────────────────────────────┘
```

### Sensor-Kachel (156 × 63 px)

```
┌──┬────────────────────────────────┐
│  │ Name (pos 15,1)  #Nr (112,1)   │  ← Font_Roboto_Regular_h16
│  │                                │
│ C│ [↑ Max]  [↓ Min]   72.3°      │  ← Temp: Font_Roboto_Medium_h22
│  │                                │     Min/Max: Font_Roboto_Regular_h14
└──┴────────────────────────────────┘
 ↑
 Farb-Balken: 10 × 63 px (Kanalfarbe)
```

| Element | Position | Größe | Font |
|---------|----------|-------|------|
| Name-Label | (15, 1) | 109 × 21 | Roboto Regular h16 |
| Nummer-Label | (112, 1) | 40 × 21 | Roboto Regular h16 |
| Temperatur-Label | (70, 33) | 82 × 42 | Roboto Medium h22 |
| Max-Wert | auto grid | 37 × 21 | Roboto Regular h14 |
| Min-Wert | auto grid | 37 × 21 | Roboto Regular h14 |
| Max-Symbol (↑) | auto | 20 × 21 | Nano Temp Limit h14 |
| Min-Symbol (↓) | auto | 20 × 21 | Nano Temp Limit h14 |
| Farb-Balken | links | 10 × 63 | — |

### Header-Bar (320 × 40 px)

| Element | Größe | Font |
|---------|-------|------|
| Menü-Button (links) | 40 × 40 | Nano h24 |
| Pitmaster-Button | 40 × 40 | Nano h24 |
| WiFi-/Battery-Status | — | Nano h24 |

### Close-Button (alle Detail-Screens)

| Eigenschaft | Wert |
|-------------|------|
| Position | `LV_DPX(335), LV_DPX(12)` (rechts oben) |
| Größe | `LV_DPX(50) × LV_DPX(35)` |
| Icon | `LV_SYMBOL_CLOSE` (LVGL built-in) |

---

## Screens

### Übersicht

| Screen | Typ | Navigation |
|--------|-----|-----------|
| Home | Haupt-Dashboard | Startscreen |
| Menu | Overlay-Menü | Menü-Button im Header |
| Temperature | Sensor-Einstellungen | Kachel antippen |
| Pitmaster | Regler-Einstellungen | Pitmaster-Button im Header |
| WiFi | QR-Code-Anzeige | WiFi-Status im Header |
| Display | Helligkeit | Menu → Display |
| Update | OTA-Fortschritt | Automatisch bei OTA |

### Home Screen

- 2×N Grid mit Sensor-Kacheln (156×63 px)
- Header 40px mit Schnellaktionen
- Long-Press auf Pitmaster → lvPitmaster_Create
- Short-Press auf Kachel → lvTemperature_Create (mit Kanal als userData)

### Temperature Screen (5 Tabs)

| Tab | Icon-Char | Inhalt |
|-----|-----------|--------|
| Min | `"E"` | Spinbox Min-Temp, Font_Roboto_Medium_h80, 270px wide |
| Max | `"F"` | Spinbox Max-Temp, Font_Roboto_Medium_h80, 270px wide |
| Type | `"n"` | Roller Sensor-Typ, Font_Roboto_Medium_h28, 3 Zeilen sichtbar |
| Color | `"m"` | 18 Farb-Buttons (50×28 px) + 10px Preview-Balken |
| Notif | `"o"` | 2× Switches (Push "p", Buzzer "q") |

Spinbox +/- Buttons: je 100×50 px  
Tab-Bereich rechts: `LV_HOR_RES / 3` = 107px frei (für Close-Button)

### Pitmaster Screen (3 Tabs)

| Tab | Icon-Char | Inhalt |
|-----|-----------|--------|
| Target | `"F"` | Spinbox Zieltemperatur, Font_Roboto_Medium_h80 |
| Channel | `"n"` | Roller Kanal-Auswahl, Font_Roboto_Medium_h28 |
| Stop | `"0"` | Stop-Button (LV_COLOR_RED, 100×50 px) |

### WiFi Screen

- QR-Code: 132px quadratisch, schwarz/weiß
- 2 Labels: je 320×30 px, Font_Gothic_A1_Medium_h16, weiß, zentriert
- AP-Modus: zeigt SSID + Passwort (12345678)
- Client-Modus: zeigt SSID + `http://[IP]`

### Display Screen

- 1 Slider (Helligkeit): Breite 210px, Range 1–10
- Icon `"t"` (links vom Slider): Font_Nano_h24
- Tab-Icon `"r"`: Font_Nano_h24

### Update Screen

- Progress-Bar: 200×25 px, Value = OTA-Fortschritt %
- Icon `"X"` (Download): Font_Nano_h24

### Menu Screen

- Screen-BG: `#333333`
- 2 Buttons: je 200×40 px, pos (40,40) und (40,100)
- Font: Gothic A1 Medium h16, weiß, zentriert
- Radius: 0, Border: 1px, kein Clip

---

## Farb-Palette (Kanal-Farben)

Identisch mit Web UI (18 Farben, 1 fehlt gegenüber Web-UI's 19):

```
0xFFFF00  0xFFC002  0x00FF00  0xFFFFFF
0xE46C0A  0xC3D69B  0x0FE6F1  0x0000FF
0x03A923  0xC84B32  0xFF9B69  0x5082BE
0xFFB1D0  0xA6EF03  0xD42A6B  0xFFDA8F
0x00B0F0  0x948A54
```

Web UI hat zusätzlich `#FF1DC4` (Index 4 in der Web-Palette). TFT hat 18 Farben.

---

## Tabview-Muster

Alle Detail-Screens verwenden denselben Tabview-Aufbau:

```cpp
lv_obj_set_style_local_pad_top(tabview, LV_TABVIEW_PART_TAB_BG,  LV_STATE_DEFAULT, 5);
lv_obj_set_style_local_pad_bottom(tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 5);
lv_obj_set_style_local_pad_right(tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, LV_HOR_RES / 3);
lv_tabview_set_anim_time(tabview, 0);  // kein Slide-Effekt
lv_obj_set_style_local_text_font(tabview, LV_CONT_PART_MAIN, ..., &Font_Nano_h24);
```

Tab-Icons sind immer einzelne Zeichen aus `Font_Nano_h24` (kein Text).  
Close-Button liegt über dem Tabview auf dem Screen-Objekt (festes Offset rechts oben).

---

## Start-Screen & Splash

```cpp
tft.fillScreen(0x31a6);  // RGB565: #316C (dunkles Teal-Grau)
tft.pushImage(33, 70, 254, 100, DisplayTftStartScreenImg);  // Logo 254×100 bei (33,70)
```

Charging-Screen (Batterie):
- `DisplayTftCharged` / `DisplayTftCharging`: 160×127 px bei (89,56)
- BG: `TFT_BLACK`

---

## Wichtige Design-Unterschiede zu Web UI

| Aspekt | Web UI | TFT |
|--------|--------|-----|
| Primary-Farbe | `#00adb5` | `#0aa5c4` (etwas dunkler/kälter) |
| Hintergrund | `#393e46` | `#181d23` (deutlich dunkler) |
| Kachel-BG | `#474a4e` | `#29313a` |
| Schrift-Stack | System-UI, Roboto | Embedded Roboto + Gothic A1 |
| Icons | icomoon (35 Custom + Web-Symbole) | Nano-Font (Custom-Symbol-Charset) |
| Alarm "kalt" | `#1874cd` (Blau) | `#00bfff` (LightBlue) |
| Kanal-Palette | 19 Farben inkl. Pink `#FF1DC4` | 18 Farben (kein Pink) |
| Formular-Pattern | Floating Labels | Spinbox / Roller (Touch-optimiert) |
| Navigation | Sidebar + Hash-Router | Direkter Screen-Switch via lvScreen |
| Animationen | CSS-Transitions 0.28s | Keine Transitions (NO_TRANSITION Flag) |

---

## LVGL 9 Migration — Relevante Hinweise

- `lvTheme.cpp` nutzt interne LVGL 7 APIs → **Komplett-Neuschreibung** erforderlich
- `lv_style_set_*` Einzeleigenschaften → LVGL 9: `lv_obj_set_style_*` direkt
- `lv_color_hex()` → in LVGL 9 unverändert
- Font-Dateien (`.c` Arrays) → kompatibel, LVGL Font Generator neu erzeugen empfohlen
- `LV_SYMBOL_*` → Namen in LVGL 9 teilweise geändert
- Spinbox, Tabview, Roller → alle in LVGL 9 vorhanden (API geändert)
- → [Issue #205](https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/issues/205)
