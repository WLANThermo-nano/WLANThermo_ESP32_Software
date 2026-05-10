# OLED Display — Design System Dokumentation

Extrahiert aus `src/display/DisplayOled.cpp` + `DisplayOledLink.cpp` — SH1106 OLED auf NanoV3 und LinkV1.

---

## Stack-Überblick

| Aspekt | NanoV3 | LinkV1 |
|--------|--------|--------|
| Display | SH1106Wire OLED | SH1106Wire OLED |
| I2C-Adresse | `0x3C` | `0x3C` |
| Auflösung | **128 × 64 px** | **128 × 64 px** |
| Farben | Monochrom (WHITE/BLACK) | Monochrom (WHITE/BLACK) |
| Bibliothek | esp8266-oled-ssd1306 (OLEDDisplayUi) | esp8266-oled-ssd1306 (OLEDDisplayUi) |
| Buttons | 2 (L: GPIO 14, R: GPIO 12) | 1 (L: GPIO 32) |
| Boot-Logo | `xbmnano` (113×53 px) | `xbmlink` (113×53 px) |
| Frames | 4 (Temp, TempSettings, Pitmaster, System) | 1 (nur Temp) |

---

## Farben

Monochrom — kein Farbsystem. Nur zwei Zustände:

| Wert | Bedeutung |
|------|-----------|
| `WHITE` | Sichtbarer Pixel (leuchtet) |
| `BLACK` | Unsichtbarer Pixel (aus) |

Alarm-Blinken: kein Farbwechsel, sondern **Flash** — Temperaturwert wird bei `flashIndicator == true` ausgeblendet (500 ms Intervall).

---

## Typografie

Aus der `esp8266-oled-ssd1306`-Bibliothek, kein Custom-Font:

| Font | Höhe | Verwendung |
|------|------|-----------|
| `ArialMT_Plain_10` | ~10 px | Standard-Labels, Kanalnummer, Name, Status-Bar |
| `ArialMT_Plain_16` | ~16 px | Temperaturwert, Menüpunkte, Popup-Header, Update-Text |

Kein weiterer Font — alle Texte nur in diesen zwei Größen.

---

## Layout (128 × 64 px)

```
┌────────────────────────────────────┐  y=0
│  Status-Bar / Overlay (y 0–12)     │
│  [Batterie] [Pitmaster-Status] [AP]│
├────────────────────────────────────┤  y=13
│                                    │
│  Content-Area  (y 13–63)           │
│  51 px nutzbare Höhe               │
│                                    │
└────────────────────────────────────┘  y=63
```

### Status-Bar (Overlay, immer sichtbar)

| Element | Position | Größe | Beschreibung |
|---------|----------|-------|-------------|
| Batterie Outline | (0, 3) | 17×8 px | Rahmen |
| Batterie Button | (18, 5) | 2×4 px | Pol |
| Batterie Füllung | (2, 5) | max 13×4 px | Ladestand proportional |
| Lade-Pfeil XBM | (4, 2) | 8×10 px | `xbmcharge` |
| WiFi Boden-Linien | (116,10), (120,10), (124,10) | je 2×1 px | Balken-Basis |
| WiFi Balken 1 | (116, 7) | 2×3 px | > −105 dBm |
| WiFi Balken 2 | (120, 5) | 2×5 px | > −95 dBm |
| WiFi Balken 3 | (124, 3) | 2×7 px | > −80 dBm |
| "AP" Text | rechts (128, 1) | — | Wenn Access-Point-Modus |
| IP-Adresse | (29, 1) | — | 5 s nach WiFi-Connect |
| Pitmaster Status | (33, 1) | — | `P 120.0 / 45%` oder `M 50%` |
| Batterie % | (24, 1) | — | Nur wenn pm_off > 10 s |

---

## XBM Icons

Alle Icons als C-Array in `DisplayOledIcons.c`, deklariert in `DisplayOledIcons.h`:

| Symbol | Array | Breite | Höhe | Verwendung |
|--------|-------|--------|------|-----------|
| Thermometer | `xbmtemp` | 20 px | 36 px | Temp-Frame + Settings-Frame links |
| Pitmaster | `xbmpit` | 18 px | 32 px | Pitmaster-Frame links |
| System | `xbmsys` | 32 px | 28 px | System-Frame (auskommentiert) |
| Pfeil gleichauf | `xbmarrow` | 7 px | 6 px | Pitmaster: Temp == Ziel |
| Pfeil hoch | `xbmarrow1` | 7 px | 6 px | Pitmaster: Temp < Ziel |
| Pfeil runter | `xbmarrow2` | 7 px | 6 px | Pitmaster: Temp > Ziel |
| Lade-Pfeil | `xbmcharge` | 8 px | 10 px | Batterie lädt |
| Zurück | `xbmback` | 32 px | 28 px | Back-Frame |
| Nano-Logo | `xbmnano` | 113 px | 53 px | Boot-Screen NanoV3 |
| Link-Logo | `xbmlink` | 113 px | 53 px | Boot-Screen LinkV1 |

---

## Frames (Screens)

### Frame 0 — Temperatur-Anzeige (`drawTemp`)

Haupt-Anzeige. Ein Kanal auf einmal, links/rechts navigierbar.

```
┌────────────────────────────────────┐
│  [Status-Bar]                  [W] │ y=0..12
├────────────────────────────────────┤
│  #1         Fleisch                │ y=20  ArialMT_Plain_10
│                                    │
│  [Thermo]         72.3 °           │ y=36  ArialMT_Plain_16
│  [Level]                           │
│  [P→]                              │ y=31  Pitmaster-Indikator
└────────────────────────────────────┘
```

| Element | Position | Font / Größe |
|---------|----------|-------------|
| Thermometer XBM | (x+19, 19+y) | 20×36 px |
| Füllbalken (Levelanzeige) | (x+27, y+43−match), 4 px breit | proportional zu Min/Max |
| Kanalnummer | rechts (19+x, 20+y) | ArialMT_Plain_10 |
| Kanalname | rechts (114+x, 20+y) | ArialMT_Plain_10 |
| Temperaturwert | rechts (114+x, 36+y) | ArialMT_Plain_16 |
| Grad-Zeichen | Kreis r=2 bei (99,41) °C / (100,41) °F | drawCircle |
| Pitmaster P/A | (44+x, 31+y) | ArialMT_Plain_10 |
| Pitmaster-Pfeil | (x+37, 24+y) | XBM 7×6 px |

Alarm-Verhalten: Temperaturwert + Grad-Zeichen blinken bei `flashIndicator == false` aus.

### Frame 1 — Temperatur-Einstellungen (`drawTempSettings`)

4 Sub-Items (via Buttons navigierbar):

| MenuItem | Anzeige | Position |
|----------|---------|----------|
| `TempSettingsUpper` | Linie bei y=25, Wert rechts (104+x, 19+y) | ArialMT_Plain_10 |
| `TempSettingsLower` | Linie bei y=39, Wert rechts (104+x, 34+y) | ArialMT_Plain_10 |
| `TempSettingsType` | Label "TYP" + Typname rechts (114+x, 36+y) | ArialMT_Plain_10 |
| `TempSettingsAlarm` | Label "ALARM" + Wert `off/push/summer/all` | ArialMT_Plain_10 |

Thermometer XBM blinkt im Edit-Modus aus.  
Edit-Grenzen: 35.0–200.0°C (95.0–392.0°F), Schritt 0.1, Long-Press Schritt 1.0.

### Frame 2 — Pitmaster-Einstellungen (`drawPitmasterSettings`)

4 Sub-Items:

| MenuItem | Label | Wert |
|----------|-------|------|
| `PitmasterSettingsProfile` | "PITMASTER:" | Profilname |
| `PitmasterSettingsChannel` | "CHANNEL:" | Kanalnummer |
| `PitmasterSettingsTemperature` | "SET:" | Zieltemperatur |
| `PitmasterSettingsType` | "ACTIVE:" | AUTO / MANUAL / OFF |

Pitmaster-XBM: pos (x+15, 20+y), 18×32 px. Blinkt im Edit-Modus.  
Alle Labels: ArialMT_Plain_10, rechts bei (116, 20) + (116+x, 36+y).

### Frame 3 — System-Einstellungen (`drawSystemSettings`)

5 Sub-Items:

| MenuItem | Label | Wert |
|----------|-------|------|
| `SystemSettingsSSID` | "SSID:" | Netzwerkname oder AP-Name |
| `SystemSettingsIP` | "IP:" | IP-Adresse |
| `SystemSettingsHost` | "HOSTNAME:" | Hostname |
| `SystemSettingsUnit` | "UNIT:" | °C / °F (editierbar) |
| `SystemSettingsFirmwareVersion` | "FIRMWARE:" | Versionsnummer |

Alle Werte: ArialMT_Plain_10, rechts bei (120, 20) und (114+x, 36+y).

---

## Statische Screens (kein UI-Frame)

### Menü (`drawMenu`)

```
┌────────────────────────────────────┐
│  ≡  MENU                           │  Hamburger + "MENU" bei (50,2)
│                                    │
│     [↑]  Temperatur / Pitmaster... │  ArialMT_Plain_16 bei (30,27)
│     [↓]                            │
└────────────────────────────────────┘
```

- Hamburger: 3 Linien bei y=3, y=7, y=11 (x: 3→13)
- Pfeil-XBM: (17,27) = hoch, (17,41) = runter
- "MENU": ArialMT_Plain_10 bei (50, 2)
- Menüpunkt: ArialMT_Plain_16 bei (30, 27)

### Popup-Screens

| Typ | Header | Inhalt |
|-----|--------|--------|
| `IpAddress` | — | IP-Adresse (teilweise implementiert) |
| `Update` | — | (teilweise implementiert) |
| `Alarm` | "ALARM!" ArialMT_Plain_16 bei (5,5) | "Temperatur außerhalb" + "der Grenzwerte" bei (5,25)+(5,40), "OK" rechts bei (107,51) |

### Boot-Screen (`drawConnect`)

Nano-Logo XBM bei (7, 4), 113×53 px. Kein Text. 2s Anzeigedauer.

### Lade-Screen (`drawCharging`)

- Batterie-Outline + Button gezeichnet
- "CHARGING..." oder "READY!" — ArialMT_Plain_10, zentriert bei (64, 30)
- Ladepfeil via `xbmcharge` + Füllung

### Update-Screen (`drawUpdate`)

- "Update: " + txt — ArialMT_Plain_10 bei (3, 3)
- "Bitte warten!" — ArialMT_Plain_16, zentriert bei (64, 28)

---

## Interaktions-Modell

### NanoV3 — 2 Buttons

| Aktion | Links (GPIO 14) | Rechts (GPIO 12) |
|--------|----------------|-----------------|
| Click | Kanal/Menü-Eintrag zurück | Kanal/Menü-Eintrag vor |
| Long-Press Start | Zurück / In Settings | Öffnet Menü (aus TempShow) |
| Double-Click | — | Wechsel Show → Edit → Set |

### LinkV1 — 1 Button

Nur 1 Button (GPIO 32), nur 1 Frame (Temperaturanzeige).  
Kein Settings-Menü, kein Edit-Modus.

### Navigations-Fluss (NanoV3)

```
Boot
 └─ TempShow  ←────────────────────────────────┐
     ├─ [L long] TempSettings                  │
     │    ├─ Upper / Lower / Type / Alarm        │
     │    └─ [R double: Edit → Set]             │
     └─ [R long] Menu                           │
          ├─ Temperatur ──── [L long] ──────────┘
          ├─ Pitmaster ──── [R long] ──────────►PitmasterSettings
          │                                       ├─ Profile / Channel / Temp / Type
          └─ System ──── [R long] ──────────────►SystemSettings
                                                  └─ SSID / IP / Host / Unit / Firmware
```

---

## NanoV3 vs. LinkV1 — Unterschiede

| Aspekt | NanoV3 | LinkV1 |
|--------|--------|--------|
| Buttons | 2 (L+R) | 1 (L) |
| Frames | 4 (Temp + 3× Settings) | 1 (nur Temp) |
| Settings-Menü | Ja | Nein |
| Edit-Modus | Ja | Nein |
| Boot-Logo | `xbmnano` | `xbmlink` |
| Boot-Zeit | 200 × 10 ms = 2 s | 100 × 10 ms = 1 s |
| Task-Wakeup | ISR-gesteuert (interrupt) | Polling |

---

## Vergleich mit TFT Design

| Aspekt | OLED (NanoV3) | TFT (MiniV3) |
|--------|---------------|-------------|
| Auflösung | 128×64 px | 320×240 px |
| Farben | Monochrom (1 bit) | RGB565 (16 bit) |
| Kanäle gleichzeitig | 1 | bis zu 8 (Grid) |
| Bedienung | 2 physische Buttons | Touch |
| Fonts | ArialMT 10/16 (Library) | Roboto / Gothic A1 / Nano (embedded) |
| Icons | XBM Bitmaps | Custom Symbol-Font |
| Alarm | Blinken (Flash) | Farbwechsel (Rot/Blau) |
| Settings | Button-basiert (Edit-Modus) | Tabview + Spinbox |
| Navigation | Frame-Wechsel + Menü | Screen-Wechsel (lvScreen) |
