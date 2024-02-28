<template>
  <div class="pure-g m-4">
    <div class="page-title-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="name">
        <span class="back-button cursor-pointer" @click="backToHome">
          <span class="icon-arrow_left"></span>
        </span>
        {{ $t('system_settings') }}
        <span
            @click="showHelpText"
            class="icon-question_sign icon-question"
          >
        </span>
      </div>
    </div>
    <div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1 mt-2">
      <div class="config-form">
        <form>
          <div class="form-group">
            <input type="text" maxlength="13" v-model="systemSettings.host" required />
            <label class="control-label" for="input">{{$t("hostname")}}</label>
            <i class="bar"></i>
          </div>
          <div class="form-group">
            <input type="text" maxlength="13" v-model="systemSettings.ap" required />
            <label class="control-label" for="input">{{$t("ap_name")}}</label>
            <i class="bar"></i>
          </div>
          <div class="form-group">
            <select v-model="systemSettings.language">
              <option
                v-for="l in languages"
                :key="l.value"
                :value="l.value"
              >{{$t(l.translationKey)}}</option>
            </select>
            <label class="control-label" for="select">{{$t("language")}}</label>
            <i class="bar"></i>
          </div>
          <div class="form-group">
            <select v-model="systemSettings.unit">
              <option
                v-for="tu in temperatureUnits"
                :key="tu.value"
                :value="tu.value"
              >{{$t(tu.translationKey)}}</option>
            </select>
            <label class="control-label" for="select">{{$t("unit")}}</label>
            <i class="bar"></i>
          </div>
          <div class="form-group">
            <select v-model="systemSettings.hwversion">
              <option v-for="hv in hardwareVersions" :key="hv" :value="hv">{{hv}}</option>
            </select>
            <label class="control-label" for="select">{{$t("hw_version")}}</label>
            <i class="bar"></i>
          </div>
          <wlan-checkbox 
            v-model="systemSettings.autoupd"
            :label="$t('update_search')">
          </wlan-checkbox>
          <wlan-checkbox 
            v-model="systemSettings.prerelease"
            :label="$t('prerelease')">
          </wlan-checkbox>
          <wlan-checkbox 
            v-if="systemSettings.hasOwnProperty('crashreport')"
            v-model="systemSettings.crashreport"
            :label="$t('crashreport')">
          </wlan-checkbox>
        </form>
        <div class="pure-u-1-1">
          <div class="grid mt-2">
            <div class="justify-self-end">
              <wlan-button 
                :label="$t('save')" 
                @click="save">
              </wlan-button>
            </div>
          </div>
        </div>
      </div>
      <template v-if="showDisplaySettings">
        <div class="name">
          {{ $t('display_settings') }}
        </div>
        <div class="config-form">
          <div class="mt10">
            <wlan-button
              class="mr-2"
              :label="$t('rotate_display')" 
              @click="rotateDisplay">
            </wlan-button>
            <wlan-button 
              :label="$t('calibrate_touch')" 
              @click="calibrateTouch">
            </wlan-button>
          </div>
        </div>
      </template>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";
import WlanButton from './shared/Button.vue'
import WlanCheckbox from './shared/Checkbox.vue'

function stringTrueFalseToBool(value) {
  if (value === 'true') return true
  if (value === 'false') return false
  return value
}

export default {
  name: "System",
  components: {
    WlanButton,
    WlanCheckbox
  },
  props: {
    settings: {
      type: Object
    }
  },
  data: () => {
    return {
      systemSettings: {
        host: "",
        ap: "",
        language: "de",
        unit: "C",
        autoupd: null,
        prerelease: null,
        crashreport: null,
      },
      languages: [
        { value: "en", translationKey: "english" },
        { value: "de", translationKey: "german" },
      ],
      temperatureUnits: [
        { value: "C", translationKey: "celsius" },
        { value: "F", translationKey: "fahrenheit" },
      ],
      hardwareVersions: [],
    };
  },
  computed: {
    showDisplaySettings: function () {
      return this.settings && 
             this.settings.device.device === 'mini' && 
             (this.settings.device.hw_version === 'v1' || this.settings.device.hw_version === 'v2')
    }
  },
  watch: {},
  mounted: function () {
    EventBus.$emit("loading", true)
    this.axios.get("/settings").then((response) => {
      this.hardwareVersions = response.data.hardware    
      // the BE is sending boolean value in string.
      response.data.system.autoupd = stringTrueFalseToBool(response.data.system.autoupd)
      response.data.system.prerelease = stringTrueFalseToBool(response.data.system.prerelease)
      if (Object.prototype.hasOwnProperty.call(response.data.system, 'crashreport')) {
        response.data.system.crashreport = stringTrueFalseToBool(response.data.system.crashreport)
      }

      this.systemSettings = response.data.system
      EventBus.$emit("loading", false)
    });
  },
  methods: {
    backToHome: function () {
      EventBus.$emit("back-to-home")
    },
    showHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_system_title'),
        content: this.$t('help_system'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/wiki/System-Einstellungen',
        linkText: this.$t('help_system_link')
      })
    },
    save: function() {
      EventBus.$emit("loading", true)
      
      const request = Object.assign({}, this.systemSettings)
      request.autoupd = this.systemSettings.autoupd.toString()
      request.prerelease = this.systemSettings.prerelease.toString()
      request.crashreport = this.systemSettings.crashreport.toString()

      this.axios.post('/setsystem', request).then(() => {
        EventBus.$emit("loading", false)
        EventBus.$emit("getSettings")
        this.backToHome()
      }).catch(() => {
        EventBus.$emit("loading", false)
      })
    },
    rotateDisplay: function() {
      if (confirm(this.$t('rotate_restart_prompt')) === true) {
        EventBus.$emit("loading", true)
        this.axios.post('/rotate', this.systemSettings).then(() => {
          EventBus.$emit("loading", false)
          location.reload()
        }).catch(() => {
          EventBus.$emit("loading", false)
        })
      }
    },
    calibrateTouch: function() {
      EventBus.$emit("loading", true)
      this.axios.post('/calibrate', this.systemSettings).then(() => {
        EventBus.$emit("loading", false)
      }).catch(() => {
        EventBus.$emit("loading", false)
      })
    }
  }
};
</script>

<style lang="scss" scoped>
</style>
