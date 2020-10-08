<template>
  <div class="pure-g">
    <div class="pure-u-1-1 app-bar-wrapper">
      <div class="app-bar-actions">
        <div class="button-container" @click="backToHome">
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
        {{ $t('system_settings') }}
        <span
          @click="showHelpText"
          class="icon-question_sign icon-question"
        ></span>
      </div>
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
          <div class="form-checkbox">
            <label for="update_search" class="pure-checkbox checkbox">
              <input v-model="systemSettings.autoupd" type="checkbox" id="update_search" />
              {{$t("update_search")}}
            </label>
          </div>
          <div class="form-checkbox">
            <label for="prerelease" class="pure-checkbox checkbox">
              <input v-model="systemSettings.prerelease" :true-value="'true'" :false-value="'false'" type="checkbox" id="prerelease" />
              {{$t("prerelease")}}
            </label>
          </div>
        </form>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";

export default {
  name: "System",
  props: {},
  data: () => {
    return {
      copyOfSystem: null,
      systemSettings: {
        host: "",
        ap: "",
        language: "de",
        unit: "C",
        autoupd: null,
        prerelease: null
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
  watch: {},
  mounted: function () {
    EventBus.$emit("loading", true)
    this.axios.get("/settings").then((response) => {
      this.hardwareVersions = response.data.hardware
      this.copyOfSystem = Object.assign({}, response.data) 
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
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/System-Einstellungen',
        linkText: this.$t('help_system_link')
      })
    },
    save: function() {
      EventBus.$emit("loading", true)
      this.axios.post('/setsystem', this.systemSettings).then(() => {
        EventBus.$emit("loading", false)
        EventBus.$emit("getSettings")
        this.backToHome()
      }).catch(() => {
        EventBus.$emit("loading", false)
      })
    }
  },
  components: {},
};
</script>

<style lang="scss" scoped>
</style>
