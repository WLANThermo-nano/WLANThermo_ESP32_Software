# Claude Design Import Guide

## Projekt-Überblick

**WLANThermo Web UI** — Dark Theme SPA für ein WiFi-BBQ-Thermometer, embedded auf ESP32.
- Framework: Vue 2 + Pure CSS + SCSS
- Design: Dunkles Theme, Teal-Akzent (#00adb5), Mobile-First ab 768px
- Keine externen Fonts, keine CDN-Dependencies

---

## Import-Schritte für Claude Design

### Schritt 1: Design Tokens importieren

Lade **`design-tokens.json`** hoch.

Claude Design erhält damit:
- Alle Farben (Backgrounds, Akzente, Text, Semantic, Kanal-Palette)
- Typografie (Größen, Weights, Line-Heights)
- Spacing, Border-Radius, Z-Index, Transitions
- Icon-System (icomoon, 35 Icons)
- Layout-Dimensionen (Nav-Breite, Header-Höhe, Breakpoint)

### Schritt 2: CSS importieren

Lade **`design-system.css`** hoch.

Enthält alle CSS-Variablen und Klassen:
- Layout-Struktur (Sidebar, Header, Content)
- Formular-Komponenten (Floating Labels, Bar-Animation)
- App-Bar-Navigation
- Temperaturkacheln (Info-Box)
- Dialog, Spinner, Listen-Items
- Utility-Klassen, Icon-Animationen

### Schritt 3: Komponenten-Referenz hochladen

Lade **`component-examples.html`** hoch.

Zeigt alle wichtigen Komponenten visuell:
1. App-Layout-Gerüst
2. Temperaturkanal-Kacheln (normal, zu heiß, zu kalt, OFF)
3. App-Bar (Zurück/Speichern)
4. Config-Form-Struktur
5. Formular-Zustände (leer, mit Wert, Fehler)
6. Buttons (primary, error, default, disabled)
7. Modal-Dialog
8. Loading-Spinner
9. Expandierbare Listen-Items (WiFi/BLE-Pattern)
10. Kanal-Farbwähler

### Schritt 4 (optional): Codebase verlinken

Verlinke das `/webui`-Verzeichnis als Codebase-Referenz für direkten Code-Zugriff.

---

## Verwendung in Claude Design

Nach dem Import kannst du Claude Design bitten:

### Neue Screens im bestehenden Stil

```
"Erstelle einen neuen Screen für Temperatur-Verlaufsdiagramme 
im WLANThermo Dark-Theme mit Teal-Akzent"
```

### Bestehende Komponenten erweitern

```
"Baue eine erweiterte Kanal-Kachel mit einem Mini-Verlaufsgraphen 
im bestehenden info-box Stil"
```

### Vue 3 Migration

```
"Migriere die Home.vue Komponente auf Vue 3 Composition API, 
behalte das gleiche Design-System"
```

### Neue Feature-Seiten

```
"Designe einen neuen Screen für OTA-Updates mit Progress-Bar 
im WLANThermo Design-System"
```

---

## Design-Entscheidungen (wichtig für konsistentes Design)

| Aspekt | Entscheidung | Begründung |
|--------|-------------|-----------|
| Dark-only | Kein Light-Mode | Embedded-Device, immer dunkles Umfeld |
| Offline-first | Keine CDN-Assets | ESP32 ohne Internetzugang |
| Pure CSS Grid | Kein Bootstrap/Tailwind | Bundle-Größe (SPIFFS-Limit) |
| Floating Labels | Material-Design-Stil | Platzsparendes Formular-Design |
| EventBus | Kein Vuex | Einfachheit, geringe Komplexität |
| Hash-Router | Kein History-Mode | Kein Server-Side Routing nötig |
| icomoon | Kein FA/Material Icons | Custom-Icons für Spezial-Symbole (Temp, Fan) |

---

## Schlüssel-Farben für schnellen Einstieg

```css
--color-bg-body:    #393e46   /* Seitenhintergrund */
--color-bg-nav:     #222831   /* Navigation/Header */
--color-bg-surface: #474a4e   /* Karten/Formulare */
--color-primary:    #00adb5   /* Teal-Akzent */
--color-error:      #f44336   /* Fehler/Löschen */
--color-input-hl:   #337ab7   /* Input-Fokus */
```

---

## Dateistruktur dieses Import-Packages

```
10-claude-design-import/
├── README.md                 ← Diese Datei
├── design-tokens.json        ← Alle Design-Tokens (aus 01-design-tokens.json)
├── design-system.css         ← Kompiliertes CSS aller Styles
├── component-examples.html   ← Interaktive Komponenten-Galerie
└── asset-references.md       ← Wo Icons, Logos und Bilder liegen
```

Die vollständige Dokumentation liegt in `../` (dem `design-system-docs/`-Ordner).
