# Typografie

## Font-Families

### Systemschrift (primär)
Keine explizite `font-family` im Projekt-CSS definiert. Es wird die Browser-Standardschrift genutzt — auf modernen Geräten typischerweise:
- macOS/iOS: San Francisco
- Windows: Segoe UI
- Android: Roboto
- Fallback: `system-ui, -apple-system, sans-serif`

**Hinweis für Vue 3 Migration:** Hier könnte eine explizite Schrift (z.B. Inter, Roboto, oder eine Custom-Font) definiert werden.

### Icon Font (icomoon)
```scss
@font-face {
  font-family: 'icomoon';
  src: url('~@/assets/icons/fonts/icomoon.ttf?anupur') format('truetype');
  font-weight: normal;
  font-style: normal;
  font-display: block;
}
```
Anwendung via CSS-Klassen: `[class^="icon-"]`, `[class*=" icon-"]`

## Font-Größen

| Token | Wert | Verwendung |
|-------|------|-----------|
| xs | `0.75em` | Kleinste Info-Texte (Adresse in Scan-Liste) |
| sm | `0.8em` / `0.8rem` | Hilfstext, Labels im Fokus-State, Fehler-Prompts |
| sm-plus | `0.9em` | Versionsnummer in Nav, Temperatur-Range-Icons |
| base | `1em` / `1rem` | Standard-Fließtext, Form-Inputs, Netzwerknamen |
| md | `1.1em` | Section-Namen (form-section-name) |
| lg | `1.2em` | App-Bar Schrift, Section-Titel (available networks, bluetooth) |
| xl | `1.5em` | Config-Form-Überschrift (`.name`), Dialog-Titel |
| 2xl | `2.2em` | Temperaturanzeige (Hauptwert in Info-Box) |

## Font-Weights

| Token | Wert | Verwendung |
|-------|------|-----------|
| light | 300 | Adress-/Info-Texte in Listen (Scan) |
| regular | 400 | Standard |
| semibold | 600 | Config-Form-Überschriften (`.name`) |
| bold | 700 | Temperatur außerhalb des Ranges (`.too-hot`, `.too-cold`) |

## Line-Heights

| Kontext | Wert |
|---------|------|
| Icon-Font | `1` (explizit gesetzt) |
| Normal / Formulare | `1.6` (Pure CSS Default) |
| App-Bar | `45px` (Höhe der Bar) |
| WiFi-Item-Body | `2.1em` |
| Cloud-Link-Label | `1.9em` |

## Letter-Spacing

Nicht explizit definiert. `letter-spacing: normal` (Browser-Default) überall.

## Heading-Hierarchie

Das Projekt verwendet **keine semantischen `<h1>`–`<h6>` Elemente** im UI-Code. Stattdessen:

| Visuelle Ebene | Element/Klasse | Stil |
|----------------|---------------|------|
| Screen-Titel | `.config-form-container .name` | `1.5em`, `font-weight: 600`, `color: $light` |
| Section-Titel | `.form-section-name` | `1.1em`, `color: #fff` |
| Subsection | `.available-networks .text` | `1.2em`, `color: #fff` |
| Body | `<input>`, `<select>` | `1rem` |
| Hilfstext | `.form-help` | `0.8rem`, `color: #b3b3b3` |

## Spezielle Text-Stile

### Floating Labels (Formular-Pattern)
```scss
.form-group .control-label {
  position: absolute;
  top: 0.25rem;
  color: #b3b3b3;
  font-size: 1rem;
  font-weight: normal;
  transition: all 0.28s ease;
}

// Bei Fokus / gültigem Wert:
  font-size: 0.8rem;
  color: gray;
  top: -0.5rem;
```

### Temperaturanzeige
- Normalzustand: `font-size: 2.2em`, `color: #fff`
- Zu heiß: `color: red; font-weight: bold`
- Zu kalt: `color: #1874cd; font-weight: bold`
- OFF-Status: Text "OFF", kein Sonderstil

### Links
- Standard in Dialogen: `color: #3366ff`
- In Config-Forms (About, Cloud): `color: #fff; text-decoration: none; hover: $input_highlight_color`
- Wiki-Links: `color: #3366ff`

### Version
```scss
.version {
  color: #fff;
  text-align: right;
  font-size: 0.9em;
  padding-right: 8.3%;
}
```

## i18n

Das UI unterstützt Deutsch (`de`) und Englisch (`en`). Die Locale wird aus `settings.system.language` gesetzt. Alle UI-Texte via `$t('key')`. Keine hardcodierten deutschen/englischen Texte im Template (außer Sektionsüberschriften wie "Telegram", "Pushover", "WLANThermo").
