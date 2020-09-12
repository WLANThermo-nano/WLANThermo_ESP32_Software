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
        {{ $t('notificationTitle') }}
        <span
          @click="showHelpText"
          class="icon-question_sign icon-question"
        ></span>
      </div>
      <div class="config-form">
        <form>
          <div class="form-checkbox">
            <label for="notificationActivate" class="pure-checkbox checkbox">
              <input v-model="notes.ext.on" :true-value="1" :false-value="0" type="checkbox" id="notificationActivate" />
              {{$t("notificationActivate")}}
            </label>
          </div>
          <div class="form-group">
            <input type="text" v-model="notes.ext.token" required />
            <label class="control-label" for="input">{{$t("notificationToken")}}</label>
            <i class="bar"></i>
          </div>
          <div class="form-group">
            <input type="text" v-model="notes.ext.id" required />
            <label class="control-label" for="input">{{$t("notificationKey")}}</label>
            <i class="bar"></i>
          </div>
          <div class="service-group">
            <div class="form-group">
              <select v-model="notes.ext.service">
                <option v-for="s in servicesOptions" :key="s.value" :value="s.value">{{s.label}}</option>
              </select>
              <label class="control-label" for="select">{{$t("notificationService")}}</label>
              <i class="bar"></i>
            </div>
            <div class="form-group">
              <select v-model="notes.ext.repeat">
                <option v-for="r in repeatOptions" :key="r.value" :value="r.value">{{$t(r.translationKey)}}</option>
              </select>
              <label class="control-label" for="select">{{$t("notificationRepead")}}</label>
              <i class="bar"></i>
            </div>
          </div>
        </form>
        <button class="pure-button pure-button-primary test-msg-button" @click="sendTestMessage">
          {{ $t('notificationSendMessage') }}
        </button>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";

export default {
  name: "PushNotification",
  props: {},
  data: () => {
    return {
      copyOfSystem: null,
      servicesOptions: [],
      repeatOptions: [
        { value: '1', translationKey: 'repeadOnce' },
        { value: '3', translationKey: 'repeadThreeTimes' },
        { value: '5', translationKey: 'repeadFiveTimes' },
        { value: '10', translationKey: 'repeadTenTimes' },
      ],
      notes: {
        ext: {
          id: '',
          on: 0,
          repeat: 1,
          service: 0,
          services: []
        }
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
      this.copyOfSystem = Object.assign({}, response.data) 
      this.notes = response.data.notes
      this.servicesOptions = response.data.notes.ext.services.map((serviceName, index) => {
        return { label: serviceName, value: index.toString() }
      })
      EventBus.$emit("loading", false)
    });
  },
  methods: {
    backToHome: function () {
      EventBus.$emit("back-to-home");
    },
    showHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_notification_title'),
        content: this.$t('help_notification'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/Push-Notification',
        linkText: this.$t('help_notification_link')
      })
    },
    save: function() {
      EventBus.$emit("loading", true)
      const requestObj = Object.assign({}, this.notes.ext)
      this.axios.post('/setPush', requestObj).then(() => {
        EventBus.$emit("loading", false)
        this.backToHome()
      });
    },
    sendTestMessage: function() {
      const requestObj = Object.assign({}, this.notes.ext)
      // 2 = send test message?
      requestObj.on = 2
      this.axios.post('/setPush', requestObj).then(() => {});
    }
  },
  components: {},
};
</script>

<style lang="scss" scoped>

.service-group {
  width: 100%;
  display: flex;
  .form-group {
    flex: 1 1 auto;
    &:last-child {
      margin-left: 0.5em;
    }
  }
}

.test-msg-button {
  margin-bottom: 1em;
}
</style>
