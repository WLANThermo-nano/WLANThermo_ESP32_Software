<template>
 <div class="pure-g m-4">
    <template v-if="!editing">
      <div class="page-title-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
        <div class="name">
          {{ $t('channel_page_title') }}
          <span
            @click="showHelpText"
            class="icon-question_sign icon-question"
          ></span>
        </div>
      </div>
      <div class="pure-u-1 pure-u-md-1-2 pure-u-xl-1-4" v-for="(c, i) in transformedChannels" :key="i">
        <div @click="editChannel(c)" 
          class="info-box m-1.5 bg-white dark:bg-darkblue-800 shadow-lg dark:shadow-none text-blue-800 dark:text-white h-36 py-3 rounded-xl cursor-pointer">
          <div class="title-row flex">
            <span class="ml-2 flex-grow font-semibold">{{c.name}}</span>
            <span class="flex-grow-0 mr-2">
              <img v-if="c.typ === 16"
                class="inline-block text-center w-5.5 h-5 mr-5"
                :src="c.connected ? bluetoothIcon : bluetoothDisabledIcon"/>
              <span class="text-sm font-normal	">#{{ c.number }}</span>
              <!-- <span v-if="c.typ === 16" :class="{'icon-disabled': !c.connected}" class="wlan-icons-bluetooth-fill text-primary"></span> -->
              <!-- <span v-if="c.typ === 17" :class="{'icon-disabled': !c.connected}" class="icon icon-radio"></span> -->
            </span>
          </div>
          <div class="body-row mt-2">
            <div class="temperature-range">
              <div class="max flex">
                <div class="bg-contain bg-no-repeat bg-center ml-3 bg-light-temp-up dark:bg-temp-up bg w-8 h-8"></div>
                <span class="text-lg mt-1 ml-2 font-semibold flex-grow">{{c.max}}°</span>
                <span class="mr-2 flex items-center">
                  <template v-if="c.pm && c.pm.typ == 'auto'">
                    <span class="text-2xl inline-block text-center wlan-icons-temperature mr-3"></span>
                    <span class="font-semibold text-lg mr-3">{{c.pm.set}}°</span>
                    <!-- fan speed value is from 0 - 100 -->
                    <!-- based on the value shows different rotate speeds -->
                    <span class="text-2xl inline-block text-center mr-3 wlan-icons-fan" :class="'icon-rotate-' + Math.ceil(c.pm.value/25)*25"></span>
                    <span class="font-semibold text-lg ">{{c.pm.value}}%</span>
                  </template>
                </span>
              </div>
              <div class="min flex mt-3">
                <div class="bg-contain bg-no-repeat bg-center ml-3 bg-light-temp-down dark:bg-temp-down bg w-8 h-8"></div>
                <span class="text-lg mt-1 ml-2 font-semibold flex-grow">{{c.min}}°</span>
                <span class="temperature font-bold text-4xl mr-2 -mt-1">
                  <template v-if="c.temp == '999'">
                    OFF
                  </template>
                  <template v-else>
                    <span :class="{
                      'text-error': c.temp > c.max,
                      'text-primary-400': c.temp < c.min,
                      }">
                      {{c.temp.toFixed(1)}}°{{unit}}
                    </span>
                  </template>
                </span>
              </div>
            </div>
          </div>
        </div>
      </div>
    </template>
    <template v-else>
      <div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
        <div class="name">
          <span class="border-l-8 pl-2" v-bind:style="{borderColor: editingChanelClone.color}">{{ editingChanelName }}</span>
          <span @click="showHelpText" class="icon-question_sign icon-question"></span>
        </div>
        <div class="config-form">
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
                <option v-for="s in filteredSensors" :key="s.type" :value="s.type">{{s.name}}</option>
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
              <div class="color-option border border-solid border-grey-500" @click="setColor(c)" v-for="c in colors" :key="c" :style="{'backgroundColor': c}"></div>
            </div>
            <wlan-checkbox 
              v-model="pushAlarmChecked"
              label="push_alarm">
            </wlan-checkbox>
            <wlan-checkbox 
              v-model="buzzerAlarmChecked"
              label="buzzer_alarm">
            </wlan-checkbox>
            <div class="flex justify-end">
              <button 
                @click="editing = false"
                class="text-blue-800 dark:text-white hover:text-grey-500 font-semibold py-1 pr-3 flex-start mr-auto">
                {{$t('back')}}
              </button>
              <button 
                @click="save"
                class="bg-primary-400 hover:bg-primary-600 text-white font-semibold py-1 px-3 rounded-full">
                {{$t('save')}}
              </button>
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
import WlanCheckbox from './shared/Checkbox.vue'

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
      bluetoothIcon: require(`@/assets/icons/svg/bluetooth-fill.svg`),
      bluetoothDisabledIcon: require(`@/assets/icons/svg/bluetooth-fill-disabled.svg`),
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
  computed: {
    filteredSensors: function () {
      if (this.editingChanelClone.fixed) {
        return this.sensors;
      }
      return this.sensors.filter(i => i.fixed === false)
    }
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
      this.transformedChannels = this.channels.map((c) => {
        c.pm = this.pitmasterpm.find(pm => pm.channel === c.number)
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
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/wiki/Temperaturmessung',
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
      }).catch(() => {
        EventBus.$emit("loading", false)
      })
    }
  },
  components: {
    WlanCheckbox
  },
};
</script>

<style lang="scss" scoped>
@import "../assets/colors.scss";

.info-box {
  display: flex;
  flex-direction: column;
  .title-row {
    display: flex;
    .name {
      flex: 1 1 auto;
    }
    .icons {
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
      .max, .min {
        flex: 1 1 auto;
        display: flex;
      }
      .icon {
        font-size: 1.1em;
        margin-right: 0.2em;
      }
    }
    .temperature {
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
  text-shadow: -1px 0 #a9adb0, 0 1px #a9adb0, 1px 0 #a9adb0, 0 -1px #a9adb0;
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

</style>
