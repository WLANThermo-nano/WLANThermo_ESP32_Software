# Expandierbare Listen-Items

## Beschreibung
Akkordeon-ähnliche Listen-Items mit Expandier-State werden in `Wlan.vue`, `Bluetooth.vue` und `Scan.vue` verwendet. Alle folgen demselben visuellen Muster.

## Pattern

```html
<div class="[wifi|bluetooth|scan]-item" :class="{'expand': index === expandingIndex}" @click="selectItem(index)">
  <!-- Immer sichtbar: Basis-Info -->
  <div class="info">
    <div class="icon">
      <Icon class="ic_white" width="1.5em" height="1.5em" fontSize="1.5em" :iconClass="..." />
    </div>
    <div class="body">
      <div class="name-address">
        <div class="name">{{ item.name }}</div>
        <div class="address">{{ item.address }}</div>
      </div>
      <!-- optionales Icon rechts -->
      <Icon v-if="item.enc" iconClass="lock" />
    </div>
  </div>
  <!-- Nur bei expand: -->
  <div class="[password-panel|details-panel]" v-if="index === expandingIndex">
    <!-- expandierter Inhalt -->
  </div>
</div>
```

## Zustände

| Zustand | CSS-Klasse | Hintergrund |
|---------|-----------|-------------|
| Normal | — | transparent |
| Hover | `:hover` | `$dark` (#393e46) |
| Expanded | `.expand` | `$dark` (#393e46) |
| Aktuell verbunden (nur WiFi) | `.current` | `$input_highlight_color` (#337ab7) |

## Variante: WiFi-Item (`Wlan.vue`)

```html
<div class="wifi-item current" v-if="currentNetwork.connect">
  <!-- aktuell verbundenes Netz (blauer Hintergrund) -->
</div>
<div class="wifi-item other" :class="{'expand': index === expandingWifi}"
     v-for="(wifi, index) in wifiList" @click="selectWifi(index)">
  <div class="info">...</div>
  <div class="password-panel" v-if="index === expandingWifi">
    <!-- Passwort-Eingabe + Buttons -->
    <div class="form-group" v-if="wifi.enc">
      <input type="password" v-model="password" />
    </div>
    <button class="pure-button">{{ $t('cancel') }}</button>
    <button class="pure-button pure-button-primary" :disabled="password.length === 0 && wifi.enc">
      {{ $t('save') }}
    </button>
  </div>
</div>
```

## Variante: Bluetooth-Item (`Bluetooth.vue`)

Gleiche Struktur. Expandierter Bereich: Kanal-Checkboxen.

```html
<div class="details-panel" v-if="deviceIndex === expandingDevice">
  <div class="channels">
    <div class="form-checkbox checkbox-row" v-for="(channel, i) in device.channels">
      <label :for="channel.id" class="pure-checkbox checkbox block-label">
        <input v-model="channel.checked" type="checkbox" @change="handleChange(...)"/>
        {{ channel.name }}
      </label>
    </div>
  </div>
</div>
```

`.block-label` — Klickbare Block-Checkbox:
```scss
.block-label {
  cursor: pointer;
  display: block;
  padding: 0.5rem;
  &:hover { background-color: $dark; }
}
```

## Variante: Scan-Device-Item (`Scan.vue`)

Nutzt `vue-swipe-actions` für Swipe-to-Delete. Kein Expand-State.

```html
<swipe-list :items="devices" item-key="name">
  <template v-slot="{ item }">
    <div class="scan-device-item" @click="deviceSelected(item)">
      <div class="body">
        <div class="image">
          <!-- Connection-Dot (rot=offline, lightgreen=online) -->
          <div class="connection-state" :class="{ connected: item.connected }"></div>
          <img :src="images[item.type]" />
        </div>
        <div class="name-address">
          <div class="name">{{ item.name }}</div>
          <div class="address">{{ item.ip }}</div>
          <div class="info">{{ item.info }}</div>
        </div>
      </div>
    </div>
  </template>
  <template v-slot:right="{ item }">
    <!-- Swipe-Action: Löschen -->
    <div class="swipeout-action red" @click="removeDevice(item)">
      <span class="icon-trash icon"></span>
    </div>
  </template>
</swipe-list>
```

`.connection-state`:
```scss
position: absolute;
height: 0.8em; width: 0.8em;
border-radius: 0.8em;
background-color: red;
&.connected { background-color: lightgreen; }
```

`.swipeout-action.red`:
```scss
background-color: $error_color;
width: 6em;
```

## Gemeinsame SCSS-Struktur

```scss
.wifi-item,
.bluetooth-item,
.scan-device-item {
  border-radius: 0.4em;
  padding: 0.6em;
  
  .info { display: flex; }
  .icon { flex: 0 0 3em; margin-top: 0.3em; }
  .body { flex: 1 1 auto; display: flex; justify-content: space-between; }
  .name-address { flex: 1 1 auto; color: #fff; flex-direction: column; }
  .name { font-size: 1.0em; }
  .address { font-size: 0.8em; margin-left: 0.3em; }
  
  &:hover, &.expand { background-color: $dark; }
}
```

## Section-Header über Listen

Verwendet in `Wlan.vue` und `Bluetooth.vue`:

```html
<div class="available-networks">  <!-- oder .select-channel-text -->
  <div class="text">{{ $t('availableNetworks') }}</div>
  <span class="ic_white icon-refresh" :class="{'icon-rotate-100': scanning}" @click="scan"></span>
</div>
```

```scss
.available-networks {
  display: flex;
  .text { flex: 1 1 auto; color: #fff; font-size: 1.2em; margin: 0.7em 0; }
  .icon-refresh { margin-top: 15px; margin-right: 18px; cursor: pointer; }
}
```
