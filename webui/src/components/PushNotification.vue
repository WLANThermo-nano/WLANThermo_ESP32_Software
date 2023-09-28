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
      <div class="form-section-name" v-if="isMobile">
        WLANThermo App
      </div>
      <div class="config-form" v-if="isMobile">
      <div class="form-checkbox">
          <label for="notificationActivate" class="pure-checkbox checkbox">
            <input v-model="app.enabled" type="checkbox" id="notificationActivate" />
            {{$t("notificationActivate")}}
          </label>
        </div>
        <button
          v-if="!currentPhoneIsConfigured"
          @click="configuredCurrentPhone"
          class="pure-button pure-button-primary test-msg-button" style="margin-top: 5px" >
          {{ $t('notificationConfigureThisPhone') }}
        </button>
        <div class="form-group">
          <div v-for="(d) in app.devices" :key="d.id">
            <div class="pure-u-2-5 control pure-grid">
              <div class="form-group">
                <input v-model="d.name" required />
                <label class="control-label" for="input"> {{ $t('deviceName') }} </label>
                <i class="bar"></i>
              </div>
            </div>
            <div class="pure-u-2-5 control pure-grid">
              <div class="form-group">
                <select v-model="d.sound">
                  <option v-for="r in soundOptions" :key="r.value" :value="r.value">{{$t(r.translationKey)}}</option>
                </select>
                <label class="control-label" for="select"> {{ $t('notificationSound') }} </label>
                <i class="bar"></i>
              </div>
            </div>
            <div class="pure-u-1-5 control pure-grid delete-button-container">
              <button class="pure-button pure-button-error renew-token-button" @click="removeDevice(d.id)">
                <span class="icon-trash"></span>
              </button>
            </div>
          </div>
        </div>
      </div>
      <div class="form-section-name">
        Telegram
      </div>
      <div class="config-form">
        <form>
          <div class="form-checkbox">
            <label for="notificationActivate" class="pure-checkbox checkbox">
              <input v-model="telegram.enabled" type="checkbox" id="notificationActivate" />
              {{$t("notificationActivate")}}
            </label>
          </div>
          <div class="form-group">
            <input type="text" maxlength="50" v-model="telegram.token" required />
            <label class="control-label" for="input">{{$t("notificationToken")}}</label>
            <i class="bar"></i>
          </div>
          <div class="form-group">
            <input type="text" maxlength="30" v-model="telegram.chat_id" required />
            <label class="control-label" for="input">{{$t("notificationChatId")}}</label>
            <i class="bar"></i>
          </div>
        </form>
        <button class="pure-button pure-button-primary test-msg-button" @click="sendTestMessage('telegram', telegram)">
          {{ $t('notificationSendMessage') }}
        </button>
      </div>
      <div class="form-section-name">
        Pushover
      </div>
      <div class="config-form">
        <form>
          <div class="form-checkbox">
            <label for="notificationActivate" class="pure-checkbox checkbox">
              <input v-model="pushover.enabled" type="checkbox" id="notificationActivate" />
              {{$t("notificationActivate")}}
            </label>
          </div>
          <div class="form-group">
            <input type="text" maxlength="50" v-model="pushover.token" required />
            <label class="control-label" for="input">{{$t("notificationToken")}}</label>
            <i class="bar"></i>
          </div>
          <div class="form-group">
            <input type="text" maxlength="30" v-model="pushover.user_key" required />
            <label class="control-label" for="input">{{$t("notificationUserKey")}}</label>
            <i class="bar"></i>
          </div>
          <div class="form-group">
            <select v-model="pushover.priority">
              <option v-for="r in priorityOptions" :key="r.value" :value="r.value">{{$t(r.translationKey)}}</option>
            </select>
            <label class="control-label" for="select">{{$t("notificationPriority")}}</label>
            <i class="bar"></i>
          </div>
        </form>
        <button class="pure-button pure-button-primary test-msg-button" @click="sendTestMessage('pushover', pushover)">
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
      copyOfPush: null,
      priorityOptions: [
        { value: '0', translationKey: 'notificationPriorityNormal' },
        { value: '1', translationKey: 'notificationPriorityHigh' },
        { value: '2', translationKey: 'notificationPriorityEmergency' },
      ],
      soundOptions: [
        { value: '0', translationKey: 'notificationSoundDefault' },
        { value: '1', translationKey: 'notificationSoundBells' },
      ],
      telegram: {
        enabled: false,
        token: '',
        chat_id: '',
        test: false,
      },
      pushover: {
        enabled: false,
        token: '',
        user_key: '',
        test: false,
      },
      app: {
        enabled: false,
        test: false,
        devices: [],
      },
      isMobile: false,
      currentPhoneUUID: null,
    };
  },
  watch: {},
  computed: {
    currentPhoneIsConfigured: function() {
      return this.app.devices.some(d => d.id === this.currentPhoneUUID)
    }
  },
  mounted: function () {
    EventBus.$emit("loading", true)
    this.isMobile = process.env.VUE_APP_PRODUCT_NAME === 'mobile'
    this.axios.get("/getpush").then((response) => {
      this.copyOfPush = Object.assign({}, response.data) 
      this.telegram = response.data.telegram
      this.pushover = response.data.pushover
      this.app = response.data.app
      EventBus.$emit("loading", false)
    });
  },
  methods: {
    removeDevice: function(deviceId) {
      this.app.devices = this.app.devices.filter(d => d.id !== deviceId)
    },
    getNewToken: function() {
      const currentPhoneIndex = this.app.devices.findIndex((d => d.id === this.currentPhoneUUID));
      if (currentPhoneIndex !== -1) {
        // eslint-disable-next-line
        const messaging = cordova.plugins.firebase.messaging
        messaging.getToken().then((token) => {
          this.app.devices[currentPhoneIndex].token_sha256 = token
        })
      }
    },
    configuredCurrentPhone: async function() {
      EventBus.$emit("loading", true)

      const {id,model} = await window.flutter_inappwebview.callHandler('getDeviceInfo')

      window.flutter_inappwebview
        .callHandler('getFCMToken')
        .then(async (tokenResponse) => {
          EventBus.$emit("loading", false)
          this.app.devices.push({
            id: id,
            name: model,
            token: tokenResponse.token,
            sound: 0
          })
        }).catch((error) => {
          console.log(`don't get got token`)
          console.log(error)
          EventBus.$emit("loading", false)
        });
    },
    backToHome: function () {
      EventBus.$emit("back-to-home");
    },
    showHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_notification_title'),
        content: this.$t('help_notification'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/wiki/Push-Notification',
        linkText: this.$t('help_notification_link')
      })
    },
    save: function() {
      EventBus.$emit("loading", true)
      const requestObj = Object.assign({}, { telegram: this.telegram, pushover: this.pushover, app: this.app })
      this.axios.post('/setpush', requestObj).then(() => {
        EventBus.$emit("loading", false)
        this.backToHome()
      }).catch(async (error) => {
        await window.flutter_inappwebview.callHandler('debug', error)
        EventBus.$emit("loading", false)
      });
    },
    sendTestMessage: function(serviceName, serviceData) {
      let dataObj = {test: true}
      dataObj[serviceName] = serviceData
      const requestObj = Object.assign({}, dataObj)
      this.axios.post('/setpush', requestObj).then(() => {});
    }
  },
  components: {},
};
</script>

<style lang="scss" scoped>
.form-section-name {
  color: #fff;
  padding: 0.3em;
  margin-top: 0.3em;
  font-size: 1.1em;
}
.test-msg-button {
  margin-bottom: 1em;
}

.renew-token-button {
  margin-top: 0.5em;
}

.delete-button-container {
  text-align: center;
}
</style>
