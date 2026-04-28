# Icon

## Beschreibung
Atom-Komponente für Icons aus dem icomoon Custom-Font. Kapselt Größe, Klick-Event und Schriftgröße.

## Datei-Pfad
`/webui/src/components/Icon.vue`

## Props

| Prop | Typ | Default | Beschreibung |
|------|-----|---------|--------------|
| `iconClass` | `String` | — | Icon-Name ohne `icon-` Prefix (z.B. `'Wlan100'`) |
| `width` | `*` | `'33px'` | CSS-Breite des Container-Divs |
| `height` | `*` | `'33px'` | CSS-Höhe des Container-Divs |
| `fontSize` | `*` | `'1em'` | Font-Größe des Icons |

## Events

| Event | Parameter | Beschreibung |
|-------|-----------|--------------|
| `click` | — | Weitergeleitet vom Container-Div |

## Slots
Keine.

## Verwendungsbeispiel

```vue
<!-- WiFi-Icon in der Header-Bar -->
<Icon 
  class="cursor-pointer" 
  @click="switchToWifiStrength"
  :iconClass="wifiIconClass"
/>

<!-- Lock-Icon in der WiFi-Liste mit angepasster Größe -->
<Icon 
  class="ic_white" 
  width="1em" 
  height="1em" 
  iconClass="lock" 
/>

<!-- Bluetooth-Icon mit expliziter Größe -->
<Icon 
  class="ic_white" 
  width="1.5em" 
  height="1.5em" 
  fontSize="1.5em" 
  iconClass="bluetooth_1" 
/>
```

## Varianten

Über zusätzliche CSS-Klassen am `<Icon>`-Element oder `iconClass` kombinierbar:

```vue
<!-- Roter Icon -->
<Icon iconClass="battery-0 icon-red" />

<!-- Roter + blinkender Icon -->
<Icon iconClass="battery-0 icon-red icon-blinker" />

<!-- Weißer Icon -->
<Icon class="ic_white" iconClass="lock" />
```

## Style-Details
- Kein eigenes Scoped-CSS
- Container: `<div :style="{width, height, fontSize}">`
- Icon selbst: `<span :class="'icon-' + iconClass">`
- Globale Icon-Stile aus `src/assets/icons/style.scss`

## Dependencies
- icomoon Custom-Font (`src/assets/icons/fonts/`)
- Globale Icon-CSS-Klassen aus `src/assets/icons/style.scss` und `icons.scss`

## Verfügbare Icon-Namen (iconClass-Werte)

| Wert | Darstellung |
|------|-------------|
| `Wlan33` | WiFi schwach |
| `Wlan66` | WiFi mittel |
| `Wlan100` | WiFi stark |
| `arrow_left` | Pfeil links (Zurück) |
| `arrow_right` | Pfeil rechts (Weiter) |
| `temp_down` | Temperatur-Minimum |
| `temp_up` | Temperatur-Maximum |
| `fan` | Lüfter |
| `trash` | Löschen |
| `download` | Download |
| `refresh` | Aktualisieren |
| `lock` | Gesperrt |
| `pencil` | Bearbeiten |
| `question_sign` | Hilfe |
| `info_sign` | Info |
| `notification` | Benachrichtigung |
| `unlock` | Entsperrt |
| `charger` | Ladegerät |
| `battery-100/75/50/25/0` | Akkustand |
| `cloud` | Cloud |
| `bluetooth_1/2` | Bluetooth |
| `stats-dots` | Statistik |
| `power-cord` | Stromversorgung |
| `home` | Startseite |
| `bell` | Glocke |
| `cog` | Zahnrad |
| `fire` | Feuer (Pitmaster) |
| `search` | Suche |
