# Code-Patterns & Best Practices

## Vue 2 Patterns

### Options API (Standard in diesem Projekt)
```js
export default {
  name: 'ComponentName',
  props: { ... },
  data: () => ({ ... }),
  computed: { ... },
  watch: { ... },
  validations: { ... },  // Vuelidate
  mounted() { ... },
  methods: { ... },
  components: { ... },
}
```

### Props-Validierung
Minimalistische Validierung: meist nur `type` ohne `required` oder `default`:
```js
props: {
  channels: { type: Array },
  unit: { type: String },
  settings: { type: Object }
}
```

### Events emittieren
Über den globalen EventBus, nicht über `$emit` auf Parent-Komponenten:
```js
// NICHT: this.$emit('loading', true)
// STATTDESSEN:
EventBus.$emit('loading', true)
EventBus.$emit('back-to-home')
EventBus.$emit('getData')
```

### EventBus-Pattern
```js
// src/event-bus.js
import Vue from 'vue'
const EventBus = new Vue()
export default EventBus

// In Komponente abonnieren:
EventBus.$on('event-name', (data) => { ... })
// In Komponente auslösen:
EventBus.$emit('event-name', data)
```

## API-Kommunikation

### Axios via Vue-Axios Plugin
```js
// GET Request
this.axios.get('/settings').then((response) => {
  const data = response.data
  this.settings = data.system
})

// POST Request
this.axios.post('/setchannels', requestObj).then(() => {
  EventBus.$emit('loading', false)
}).catch(() => {
  EventBus.$emit('loading', false)
})

// Parallel Requests
Promise.all([
  this.axios.get('/data'),
  this.axios.get('/settings')
]).then(([dataResp, settingsResp]) => { ... })
```

### Loading-Spinner Pattern
```js
// Vor API-Call:
EventBus.$emit('loading', true)

// In .then() UND .catch():
EventBus.$emit('loading', false)
```

### Daten-Initialisierung in mounted()
```js
mounted() {
  EventBus.$emit('loading', true)
  this.axios.get('/settings').then((response) => {
    this.data = response.data
    EventBus.$emit('loading', false)
  })
}
```

## Formular-Validierung (Vuelidate)

### Validierungs-Deklaration
```js
import { between } from 'vuelidate/lib/validators'

// Statische Regeln:
validations: {
  editingProfile: {
    Kp: { between: between(0, 150) },
    Ki: { between: between(0, 5) },
  }
}

// Dynamische Regeln (abhängig von anderen Feldern):
validations() {  // als Funktion!
  return {
    editingChanelClone: {
      max: { between: between(Math.max(this.editingChanelClone.min, -999.9), 999.9) },
      min: { between: between(-999.9, Math.min(this.editingChanelClone.max, 999.9)) }
    }
  }
}
```

### Fehler-State im Template
```html
<div class="form-group" :class="{ 'error': $v.field.$invalid }">
  <input v-model.lazy="field" />
  <div class="error-prompt" v-if="$v.field.$invalid">
    {{ $t('v_must_between', { min: $v.field.$params.between.min, max: $v.field.$params.between.max }) }}
  </div>
</div>
```

### Speichern verhindern bei Fehler
```js
save() {
  if (this.$v.$invalid) return
  // ... API-Call
}
```

### Vuelidate in `v-for` (Pitmaster)
```html
<div v-for="(pm, index) in $v.pitmaster.pm.$each.$iter" :key="index">
  <input v-model="pm.value.$model" />
  <div v-if="pm.value.$invalid">...</div>
</div>
```

## Naming Conventions

### Komponenten
- PascalCase für Dateien und Registrierung: `Home.vue`, `PushNotification.vue`
- Semantische Namen nach Feature: `Wlan`, `Pitmaster`, `Bluetooth`

### CSS-Klassen
- kebab-case: `.config-form-container`, `.app-bar-actions`, `.temp-down`
- BEM-ähnlich aber ohne `__` und `--`: `.wifi-item .info .body .name-address`
- Utility-Klassen kurz: `.mr5`, `.mt10`, `.ic_white`
- Pure CSS Präfix: `.pure-*`
- Icon-Klassen: `.icon-*`

### SCSS-Variablen
```scss
$variable_name  // Underscore-Trennung (nicht kebab)
$medium, $dark, $primary, $error_color
```

### JavaScript
- camelCase für Variablen und Methoden: `backToHome`, `editingChanelClone`, `showColorPicker`
- kebab-case für EventBus-Events: `'back-to-home'`, `'show-help-dialog'`
- SCREAMING_SNAKE_CASE für Konstanten: `MY_DEVICES_KEY`, `DEVICE_SCHEMA_VERSION`

### i18n-Keys
- camelCase: `menuHome`, `wlanTitle`, `notificationActivate`
- Hierarchisch nach Screen: `help_channel_title`, `help_channel_link`

## Styles Organisation

### Scoped vs. Global
- **Global** (`App.vue`): Layout-Klassen, Basis-UI-Elemente die überall genutzt werden
- **Scoped** (Feature-Komponenten): Komponenten-spezifische Styles
- **Globals in Assets**: `colors.scss`, `controls.scss`, `global.scss`

### SCSS-Import in Komponenten
```vue
<style lang="scss" scoped>
@import "../assets/colors.scss";  // Nur Variablen, kein CSS-Output
// ...
</style>
```

### Pure CSS Integration
Pure CSS wird als SCSS-Datei importiert (in `global.scss`):
```scss
@import "./external/pure.scss";
@import "./external/pure-responsive.scss";
```
Buttons werden durch `pure-button-primary` etc. gestylt, nicht durch eigene Klassen.

## Responsive Design Pattern

### Breakpoint (einziger in diesem Projekt)
```scss
@media screen and (max-width: 48em) { ... }  // 768px
```

### Responsive Grid (Pure CSS)
```html
<div class="pure-u-1 pure-u-md-1-2 pure-u-xl-1-4">
  <!-- 100% → 50% → 25% -->
</div>
```

## State-Management

**Kein Vuex!** State wird per Props von `App.vue` nach unten gegeben oder direkt in der Komponente gehalten.

### App-Ebene State (`App.vue`)
```js
data: () => ({
  settings: { system: {...}, features: {...}, device: {...} },
  system: { unit: '', rssi: null, soc: null, ... },
  channels: [],
  pitmaster: { pm: [], type: [] },
  navActive: false,
  showSpinner: false,
})
```

### Lokaler State (Komponenten)
- Editierter Datensatz: Klon erstellen (`Object.assign({}, original)`)
- Edit-Mode-Flag: `editing: false` / `isEditingProfile: false`

## Daten-Initialisierung

### Deep Clone für Edit
```js
editChannel(channel) {
  this.editingChanelClone = Object.assign({}, channel)  // shallow clone
  // ACHTUNG: Nur flache Objekte! Bei nested Objects: JSON.parse(JSON.stringify(obj))
}
```

### Alarm-Bits dekodieren
```js
if (alarm == 0) { push = false; buzzer = false }
if (alarm == 1) { push = true;  buzzer = false }
if (alarm == 2) { push = false; buzzer = true  }
if (alarm == 3) { push = true;  buzzer = true  }
```

### Alarm-Bits kodieren
```js
let value = 0
if (pushChecked) value += 1
if (buzzerChecked) value += 2
```

## Icon-Animations Pattern

```scss
// Rotation für Loading-States (Geschwindigkeit variabel)
.icon-rotate-100 { animation: rotate 2s infinite linear; }   // schnell
.icon-rotate-75  { animation: rotate 3s infinite linear; }
.icon-rotate-50  { animation: rotate 4s infinite linear; }
.icon-rotate-25  { animation: rotate 5s infinite linear; }   // langsam

// Dynamische Geschwindigkeit basierend auf Wert (Home.vue, Pitmaster Fan):
:class="'icon-rotate-' + Math.ceil(c.pm.value / 25) * 25"
// value=0→0, value=1-25→25, value=26-50→50, value=51-75→75, value=76-100→100
```

## Mobile (Cordova) Pattern

```js
// Feature-Detection
if (process.env.VUE_APP_PRODUCT_NAME === 'mobile') {
  // Cordova-spezifischer Code
}

// Cordova-Plugin-Calls (nach deviceready Event)
document.addEventListener('deviceready', this.initMobile.bind(this), false)

// URL öffnen
cordova.InAppBrowser.open(url, '_system')

// Firebase Messaging
cordova.plugins.firebase.messaging.getToken().then((token) => { ... })
```
