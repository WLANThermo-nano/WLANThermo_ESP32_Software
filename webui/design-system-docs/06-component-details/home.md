# Home (Temperaturkanal-Dashboard)

## Beschreibung
Der zentrale Dashboard-Screen. Zeigt alle aktiven Temperaturkanäle als Kacheln an. Klick auf eine Kachel öffnet das Bearbeitungs-Formular für diesen Kanal inline (kein eigener Route, State-Wechsel).

## Datei-Pfad
`/webui/src/components/Home.vue`

## Props

| Prop | Typ | Default | Beschreibung |
|------|-----|---------|--------------|
| `unit` | `String` | — | Temperatureinheit (`'C'` oder `'F'`) |
| `channels` | `Array` | — | Kanal-Daten vom `/data`-API |
| `pitmasterpm` | `Array` | — | Pitmaster-PM-Daten (für Kanal-Verknüpfung) |
| `settings` | `Object` | — | (ungenutzt im aktuellen Code) |

## Events
Keine direkten eigenen Events. Nutzt EventBus:
- `EventBus.$emit('show-help-dialog', {...})` — Hilfe-Dialog
- `EventBus.$emit('loading', true/false)` — Spinner
- `EventBus.$emit('getData')` — Daten neu laden nach Speichern

## Slots
Keine.

## Verwendungsbeispiel

```vue
<!-- In App.vue (via router-view mit Pass-Through Props) -->
<router-view 
  :channels="channels" 
  :pitmasterpm="pitmaster.pm" 
  :unit="system.unit" 
/>
```

## Varianten

### Modus 1: Kanal-Übersicht (`editing: false`)
- Grid von Kacheln (`.info-box`)
- 1 Spalte Mobile → 2 Spalten Tablet → 4 Spalten Desktop
- Gefiltert: Kanäle mit `temp === '999'` ausgeblendet (wenn mind. 1 Kanal aktiv ist)
- Kanal mit Pitmaster (auto): zeigt Zieltemperatur + Fan-Icon + Fan-Wert statt Kanalname

### Modus 2: Kanal-Bearbeitung (`editing: true`)
- Vollbreite Formular
- Felder: Name (max 10 Zeichen), Max-Temp, Min-Temp, Sensor-Typ, Farbe, Alarm-Checkboxen
- Validierung via Vuelidate (`between` für min/max)
- Farb-Picker: Inline Color-Swatches (19 Farben)

## Kanal-Datenstruktur (vom API)

```js
{
  number: 1,          // Kanal-Nummer
  name: 'Kanal 1',   // Name (max 10 Zeichen)
  temp: 23.5,         // Aktuelle Temperatur (999 = OFF)
  min: 10,            // Minimum-Alarm
  max: 90,            // Maximum-Alarm
  color: '#FF9B69',   // Kachel-Rahmenfarbe
  typ: 0,             // Sensor-Typ (Integer)
  alarm: 0,           // Alarm-Flags (0=off, 1=push, 2=buzzer, 3=beide)
  connected: true,    // BLE-/Remote-Verbindungsstatus
  fixed: false,       // Sensor-Typ gesperrt
  // pm wird zur Laufzeit hinzugefügt:
  pm: { typ: 'auto', set: 120, value: 65, channel: 1 } | undefined
}
```

## Style-Details

### Info-Box (Kachel)
```scss
.info-box {
  height: 70px;
  background-color: #474a4e;  // $medium_dark
  border-left: 10px solid;    // Farbe dynamisch (Kanalfarbe)
  color: #fff;
  margin: 5px;
  padding: 10px;
}
```

### Temperaturanzeige
- `font-size: 2.2em`
- Normal: `color: #fff`
- `too-hot` (temp > max): `color: red; font-weight: bold`
- `too-cold` (temp < min): `color: #1874cd; font-weight: bold`

### Color-Picker
```scss
.color-option {
  width: 2em;
  height: 2em;
  border-radius: 2em;   // voller Kreis
  opacity: 0.8;
  &:hover { opacity: 1; }
}
```

## Dependencies
- `vuelidate/lib/validators` (between)
- EventBus (`../event-bus`)
- Globale CSS-Klassen: `.config-form-container`, `.app-bar-actions` etc.
