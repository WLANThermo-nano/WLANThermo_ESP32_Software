# Formular-System

## Beschreibung
Das Formular-Design basiert auf dem **Material Design Floating-Label-Pattern**, implementiert mit reinem CSS (keine Library). Definiert in `src/assets/controls.scss`.

## Grundlegendes Muster

```html
<div class="form-group">
  <input type="text" v-model="value" required>
  <label class="control-label" for="input">Label-Text</label>
  <i class="bar"></i>
</div>
```

Das Label "floated" nach oben wenn der Input Fokus hat oder einen Wert enthält.

## Formular-Zustände

### Normal (kein Wert, kein Fokus)
- Label: mittig im Feld positioniert, `color: #b3b3b3`, `font-size: 1rem`
- Border-Bottom: `0.0625rem solid #999`
- Input-Text: transparent (unsichtbar solange kein Fokus)

### Aktiv (hat Wert / fokussiert)
- Label: oben positioniert (`top: -0.5rem`), `font-size: 0.8rem`, `color: gray`
- Input-Text: `color: #fff` (sichtbar)
- Border-Bottom: `0.0625rem solid #999`

### Fokus
- Label-Farbe: `$input_highlight_color` (#337ab7)
- `::before` Bar animiert von Mitte nach außen: `width: 100%, left: 0`
- Bar-Farbe: `$input_highlight_color`
- Transition: `left 0.28s ease, width 0.28s ease`

### Fehler (`form-group.error`)
```scss
.form-group.error {
  .control-label { color: #f44336 !important; }
  .bar { border-bottom-color: #f44336 !important; }
  .bar::before { background: #f44336; }
}
```
- Error-Text unter Feld: `.error-prompt { color: #f44336; font-size: 0.8em }`

## Input-Typen und Verwendung

### Text-Input
```html
<div class="form-group">
  <input type="text" v-model="value" maxlength="13" required>
  <label class="control-label">Hostname</label>
  <i class="bar"></i>
</div>
```

### Number-Input (mit Validierung)
```html
<div class="form-group" :class="{ 'error': $v.field.$invalid }">
  <input type="number" v-model.lazy="field" max="999.9" min="-999.9" step="any" required>
  <label class="control-label">Max-Temperatur</label>
  <i class="bar"></i>
  <div class="error-prompt" v-if="$v.field.$invalid">
    {{ $t('v_must_between', {...}) }}
  </div>
</div>
```

### Password-Input
```html
<div class="form-group">
  <input type="password" v-model="password" maxlength="63" required>
  <label class="control-label">Passwort</label>
  <i class="bar"></i>
</div>
```

### Select-Input
```html
<div class="form-group">
  <select v-model="value">
    <option v-for="opt in options" :key="opt.value" :value="opt.value">
      {{ opt.label }}
    </option>
  </select>
  <label class="control-label">Sprache</label>
  <i class="bar"></i>
</div>
```
- `select` immer im "aktiven" State (Label oben): wegen CSS-Selektor `select ~ .control-label`
- Custom-Appearance: `-webkit-appearance: none` (kein Browser-Standard-Pfeil)
- `select option { background-color: $medium_dark }` — Dunkel-Theme für Dropdown

### Input mit Icon-Overlay
```html
<div class="form-group">
  <input class="with-icon" type="text" v-model="value" required>
  <label class="control-label">Label</label>
  <i class="bar"></i>
  <span class="icon-pencil icon-form"></span>
</div>
```
`.icon-form`: `position: absolute; top: 0.375rem; right: 0.25rem; color: #fff`
`.with-icon`: `width: calc(100% - 1.8em)` (Platz für Icon)

## Checkboxen

```html
<div class="form-checkbox">
  <label for="myCheck" class="pure-checkbox checkbox">
    <input v-model="checked" type="checkbox" id="myCheck" />
    Label-Text
  </label>
</div>
```

```scss
.form-checkbox {
  margin-top: 1.25rem;
  margin-bottom: 1rem;
  .checkbox { color: #fff; }
}
```

## Buttons

### Primär (Speichern, Aktionen)
```html
<button class="pure-button pure-button-primary" @click="save">
  {{ $t('save') }}
</button>
```
- Hintergrund: `$primary` (#00adb5), Text: weiß

### Fehler (Löschen)
```html
<button class="pure-button pure-button-error" @click="delete">
  <span class="icon-trash"></span>
</button>
```
- Hintergrund: `$error_color` (#f44336), Text: weiß

### Standard (Abbrechen)
```html
<button class="pure-button" @click="cancel">
  {{ $t('cancel') }}
</button>
```
- Pure CSS Default (hellgrau)

### Disabled-State
```html
<button class="pure-button pure-button-primary" :disabled="condition">
```

## App-Bar Navigation (Zurück/Speichern)

Kein `<button>`, sondern Click-Handler auf Div:

```html
<div class="app-bar-wrapper">
  <div class="app-bar-actions">
    <div class="button-container" @click="backToHome">
      <span class="icon-arrow_left"></span>
      <span>{{ $t('back') }}</span>
    </div>
    <div class="button-container" @click="save">
      <span>{{ $t('save') }}</span>
      <span class="icon-arrow_right"></span>
    </div>
  </div>
</div>
```

```scss
.app-bar-actions {
  background-color: $medium;
  color: $primary;
  font-size: 1.2em;
  height: 1.7em;
  position: fixed;
  z-index: 5;
}
.button-container {
  padding: 0.2em;
  color: $primary;
  cursor: pointer;
}
```

## Formular-Struktur (Seiten-Level)

```html
<div class="config-form-container pure-u-1-1">
  <div class="name">
    Seitenüberschrift
    <span @click="showHelp" class="icon-question_sign icon-question"></span>
  </div>
  <div class="config-form">
    <form>
      <!-- form fields -->
    </form>
  </div>
  <!-- optionale weitere Sektionen: -->
  <div class="form-section-name">Sektion 2</div>
  <div class="config-form">...</div>
</div>
```

`.config-form-container .name`:
- `font-size: 1.5em; font-weight: 600; color: $light`
- Hilfe-Icon rechts (`.icon-question { float: right; cursor: pointer }`)

`.config-form`:
- `background-color: $medium_dark`
- `padding: 0.25rem 0.5rem 0.75rem`
- `margin: 0.25rem`

`.form-section-name`:
- `color: #fff; font-size: 1.1em; padding: 0.3em; margin-top: 0.3em`

## Number-Input ohne Spinner-Arrows

```scss
/* Chrome, Safari, Edge, Opera */
input::-webkit-outer-spin-button,
input::-webkit-inner-spin-button {
  -webkit-appearance: none;
  margin: 0;
}
/* Firefox */
input[type=number] {
  -moz-appearance: textfield;
}
```
