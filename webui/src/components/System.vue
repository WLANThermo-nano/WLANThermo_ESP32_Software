<template>
  <div class="pure-g">
    <div class="pure-u-1-1">
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
            <input type="text" v-model="systemSettings.host" required />
            <label class="control-label" for="input">{{$t("hostname")}}</label>
            <i class="bar"></i>
          </div>
          <div class="form-group">
            <input type="text" v-model="systemSettings.ap" required />
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
              <input v-model="systemSettings.getupdate" :true-value="'true'" :false-value="'false'" type="checkbox" id="prerelease" />
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
        getupdate: null
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
    this.axios.get("/settings").then((response) => {
      this.hardwareVersions = response.data.hardware
      this.copyOfSystem = Object.assign({}, response.data) 
      this.systemSettings = response.data.system
    });
  },
  methods: {
    backToHome: function () {
      EventBus.$emit("back-to-home");
    },
    showHelpText: function () {
      EventBus.$emit("show-help-dialog", {
        title: "System Einstellungen",
        content:
          '<p><span>Konfiguriere dein WLANThermo Nano mit Hilfe der Systemeinstellungen und mach es zu DEINEM WLANThermo.</span></p><p><span>Siehe auch: </span><a href="https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/System-Einstellungen" target="_blank"><span style="color:#3366ff">Wiki - System</span></a></p>',
      });
    },
    save: function() {
      const requestObj = Object.assign({}, this.copyOfSystem)
      requestObj.system = Object.assign({}, this.systemSettings)
      this.axios.post('/settings', requestObj).then(() => {
        this.backToHome()
      });
    }
  },
  components: {},
};
</script>

<style lang="scss" scoped>
</style>