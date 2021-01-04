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
      <div class="form-section-name">
        Telegram
      </div>
      <div class="config-form">
        <form>
          <div class="form-checkbox">
            <label for="notificationActivate" class="pure-checkbox checkbox">
              <input v-model="telegram.enabled" :true-value="1" :false-value="0" type="checkbox" id="notificationActivate" />
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
              <input v-model="pushover.enabled" :true-value="1" :false-value="0" type="checkbox" id="notificationActivate" />
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
      ],
      telegram: {
        enabled: false,
        token: '',
        chat_id: 0,
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
    };
  },
  watch: {},
  mounted: function () {
    EventBus.$emit("loading", true)
    this.axios.get("/getpush").then((response) => {
      this.copyOfPush = Object.assign({}, response.data) 
      this.telegram = response.data.telegram
      this.pushover = response.data.pushover
      this.app = response.data.app
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
      });
    },
    sendTestMessage: function(serviceName, serviceData) {
      let dataObj = {}
      dataObj[serviceName] = JSON.parse(JSON.stringify(serviceData))
      dataObj[serviceName].test = true
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
</style>
