<template>
 <div class="pure-g">
    <template v-if="!editing">
      <div class="pure-u-1 pure-u-md-1-2 pure-u-xl-1-4" v-for="(c, i) in transformedChannels" :key="i">
        <div @click="editChannel(c)" class="info-box cursor-pointer" v-bind:style="{borderColor: c.color}">
          <div class="title-row">
            <div class="name">
              <template v-if="c.pm && c.pm.typ == 'auto'">
                <span class="icon icon-uniF2C7"></span>
                {{c.pm.set}}° /
                <!-- fan speed value is from 0 - 100 -->
                <!-- based on the value shows different rotate speeds -->
                <span class="icon icon-fan" :class="'icon-rotate-' + Math.ceil(c.pm.value/25)*25"></span>
                {{c.pm.value}}%
              </template>
              <template v-else>
                {{c.name}}
              </template>
            </div>
            <div class="number">
              <span v-if="c.typ === 16" :class="{'icon-disabled': !c.connected}" class="icon icon-bluetooth_1"></span>
              <span v-if="c.typ === 17" :class="{'icon-disabled': !c.connected}" class="icon icon-radio"></span>
              #{{c.number}}
            </div>
          </div>
          <div class="body-row">
            <div class="temperature-range">
              <div class="max">
                <span class="icon icon-temp_up"></span>{{c.max}}°
              </div>
              <div class="min">
                <span class="icon icon-temp_down"></span>{{c.min}}°
              </div>
            </div>
            <div class="temperature">
              <template v-if="c.temp == '999'">
                OFF
              </template>
              <template v-else>
                <span :class="{
                  'too-hot': c.temp > c.max,
                  'too-cold': c.temp < c.min,
                  }">
                  {{c.temp.toFixed(1)}}°{{unit}}
                </span>
              </template>
            </div>
          </div>
        </div>
      </div>
    </template>
    <template v-else>
      <div class="pure-u-1-1 app-bar-wrapper">
        <div class="app-bar-actions">
          <div class="button-container" @click="editing = false">
            <span class="icon-arrow_left"></span>
            <span>{{$t('back')}}</span>
          </div>
          <div class="button-container" @click="save">
            <span>{{$t('save')}}</span>
            <span class="icon-arrow_right"></span>
          </div>
        </div>
      </div>
      <div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
        <div class="name">
          {{ editingChanelName }}
          <span @click="showHelpText" class="icon-question_sign icon-question"></span>
        </div>
        <div class="config-form" v-bind:style="{borderColor: editingChanelClone.color}">
          <form>
            <div class="form-group">
              <input type="text" v-model="editingChanelClone.name" maxlength="10" required>
              <label class="control-label" for="input">{{$t("channel_name")}}</label>
              <i class="bar"></i>
            </div>
            <div class="form-group" :class="{ 'error': $v.editingChanelClone.max.$invalid}">
              <input type="number" max="999.9" min="-999.9" step="any" v-model.lazy="editingChanelClone.max" required>
              <label class="control-label" for="input">{{$t("temp_max")}}</label>
              <i class="bar"></i>
              <div class="error-prompt" v-if="$v.editingChanelClone.max.$invalid">
                {{$t('v_must_between', {min: $v.editingChanelClone.max.$params.between.min, max: $v.editingChanelClone.max.$params.between.max})}}
              </div>
            </div>
            <div class="form-group" :class="{ 'error': $v.editingChanelClone.min.$invalid}">
              <input type="number" max="999.9" min="-999.9" step="any" v-model.lazy="editingChanelClone.min" required>
              <label class="control-label" for="input">{{$t("temp_min")}}</label>
              <i class="bar"></i>
              <div class="error-prompt" v-if="$v.editingChanelClone.min.$invalid">
                {{$t('v_must_between', {min: $v.editingChanelClone.min.$params.between.min, max: $v.editingChanelClone.min.$params.between.max})}}
              </div>
            </div>
            <div class="form-group">
              <select v-model="editingChanelClone.typ" :disabled="editingChanelClone.fixed">
                <option v-for="s in sensors" :key="s.type" :value="s.type">{{s.name}}</option>
              </select>
              <label class="control-label" for="select">{{$t("temp_sensor")}}</label>
              <i class="bar"></i>
            </div>
            <div class="form-group">
              <input class="color-input" readonly="readonly" value="⬤" @click="showColorPicker = true" :style="{'color': editingChanelClone.color}" required>
              <label class="control-label" for="select">{{$t("color")}}</label>
              <i class="bar"></i>
            </div>
            <div class="colors-container" v-if="showColorPicker">
              <div class="color-option" @click="setColor(c)" v-for="c in colors" :key="c" :style="{'backgroundColor': c}"></div>
            </div>
            <div class="form-checkbox">
              <label for="alarm" class="pure-checkbox checkbox">
                <input v-model="pushAlarmChecked" type="checkbox" id="alarm" /> {{$t("push_alarm")}}
              </label>
            </div>
            <div class="form-checkbox">
              <label for="bz-alarm" class="pure-checkbox checkbox">
                <input v-model="buzzerAlarmChecked" type="checkbox" id="bz-alarm" /> {{$t("buzzer_alarm")}}
              </label>
            </div>
          </form>
        </div>
      </div>
    </template>
  </div>
</template>

<script>
import EventBus from '../event-bus'
import { between } from 'vuelidate/lib/validators'

export default {
  name: "Home",
  props: {
    unit: {
      type: String
    },
    channels: {
      type: Array
    },
    pitmasterpm: {
      type: Array
    },
    settings: {
      type: Object
    }
  },
  data: () => {
    return {
      navActive: false,
      editing: false,
      editingChanelName: '',
      editingChanelClone: {
        color: '',
        fixed: false,
        max: null,
        min: null,
      },
      pushAlarmChecked: false,
      buzzerAlarmChecked: false,
      showColorPicker: false,
      transformedChannels: [],
      sensors: [],
      colors: [
        "#FFFF00",
        "#FFC002",
        "#00FF00",
        "#FFFFFF",
        "#FF1DC4",
        "#E46C0A",
        "#C3D69B",
        "#0FE6F1",
        "#0000FF",
        "#03A923",
        "#C84B32",
        "#FF9B69",
        "#5082BE",
        "#FFB1D0",
        "#A6EF03",
        "#D42A6B",
        "#FFDA8F",
        "#00B0F0",
        "#948A54"
      ],
    };
  },
  validations() {
    return {
      editingChanelClone: {
        max: {
          between: between(Math.max(this.editingChanelClone.min, -999.9), 999.9)
        },
        min: {
          between: between(-999.9, Math.min(this.editingChanelClone.max, 999.9))
        }
      }
    }
  },
  watch: {
    channels: function() {
      this.transformChannels()
    }
  },
  mounted: function() {
    this.transformChannels()
  },
  methods: {
    transformChannels: function() {
      this.transformedChannels = this.channels.map((c, i) => {
        c.pm = this.pitmasterpm[i]
        return c
      });
      if (this.channels.some(c => c.temp != '999')) {
        this.transformedChannels = this.transformedChannels.filter(c => c.temp != '999')
      }
    },
    editChannel: function(channel) {
      this.editingChanelClone = Object.assign({}, channel)
      this.editingChanelName = this.editingChanelClone.name
      if (this.editingChanelClone.alarm == 0) {
        this.pushAlarmChecked = false
        this.buzzerAlarmChecked = false
      } else if (this.editingChanelClone.alarm == 1) {
        this.pushAlarmChecked = true
        this.buzzerAlarmChecked = false
      } else if (this.editingChanelClone.alarm == 2) {
        this.pushAlarmChecked = false
        this.buzzerAlarmChecked = true
      } else if (this.editingChanelClone.alarm == 3) {
        this.pushAlarmChecked = true
        this.buzzerAlarmChecked = true
      }
      this.editing = true
      this.axios.get('/settings').then((response) => {
        const data = response.data
        this.sensors = data.sensors
      })
    },
    setColor: function(color) {
      this.editingChanelClone.color = color
      this.showColorPicker = false
    },
    getAlarmValue() {
      let value = 0
      if (this.pushAlarmChecked) {
        value += 1
      }
      if (this.buzzerAlarmChecked) {
        value += 2
      }
      return value
    },
    showHelpText: function() {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_channel_title'),
        content: this.$t('help_channel'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/Kanal-Einstellungen',
        linkText: this.$t('help_channel_link')
      })
    },
    save: function() {
      if (this.$v.$invalid) {
        return;
      }
      EventBus.$emit("loading", true)
      const requestObj = {
        alarm: this.getAlarmValue(),
        color: this.editingChanelClone.color,
        connected: this.editingChanelClone.connected,
        fixed: this.editingChanelClone.fixed,
        max: this.editingChanelClone.max.toString(),
        min: this.editingChanelClone.min.toString(),
        name: this.editingChanelClone.name,
        number: this.editingChanelClone.number,
        temp: this.editingChanelClone.temp,
        typ: this.editingChanelClone.typ.toString(),
      }
      this.axios.post('/setchannels', requestObj).then(() => {
        this.editing = false;
        EventBus.$emit("getData")
        EventBus.$emit("loading", false)
      });
    }
  },
  components: {
  },
};
</script>

<style lang="scss" scoped>
@import "../assets/colors.scss";

.info-box {
  display: flex;
  flex-direction: column;
  margin: 5px;
  padding: 10px;
  height: 70px;
  color: #fff;
  background-color: $medium_dark;
  border-left: 10px solid;
  .title-row {
    display: flex;
    padding-bottom: 0.5em;
    .name {
      flex: 1 1 auto;
    }
    .number {
      flex: 0 0 auto;
    }
  }
  .body-row {
    display: flex;
    flex: 1 1 auto;
    .temperature-range {
      flex: 1 1 auto;
      display: flex;
      flex-direction: column;
      font-size: 0.9em;
      .max {
        flex: 1 1 auto;
      }
      .min {
        flex: 1 1 auto;
      }
      .icon {
        font-size: 1.1em;
      }
    }
    .temperature {
      flex: 0 0 auto;
      font-size: 2.2em;
      margin-top: 0.05em;
      .too-hot, .too-cold {
        font-weight: bold;
      }
      .too-hot {
        color: red;
      }
      .too-cold {
        color: #1874cd;
      }
    }
  }
}

.color-input {
  // font-size: 1.5em;
}

.colors-container {
  .color-option {
    margin: 2px;
    display: inline-block;
    width: 2em;
    height: 2em;
    border-radius: 2em;
    opacity: 0.8;
    cursor: pointer;
    &:hover {
      opacity: 1;
    }
  }
}

.config-form {
  border-left: 10px solid;
}
</style>
