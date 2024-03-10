<template>
  <div class="pure-g m-4">
    <div class="page-title-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="name">
        <span class="back-button cursor-pointer" @click="backToHome">
          <span class="icon-arrow_left"></span>
        </span>
        {{ $t('notificationTitle') }}
        <span @click="showHelpText" class="icon-question_sign icon-question">
        </span>
      </div>
    </div>

    <template v-if="isMobile">
      <div class="pure-u-1-1 tracking-normal mt-2">
        <div class="flex ml-1 my-2">
          <div class="text-blue-800 dark:text-white align-middle flex-grow inline-block">
            <span class="align-middle font-semibold">{{ $t('notificationActivate') }}</span>
          </div>
          <div class="self-end">
            <wlan-toggle-button v-model="app.enabled"></wlan-toggle-button>
          </div>
        </div>
      </div>
      <transition name="fadeHeight">
        <div v-if="app.enabled" class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
          <div class="config-form">
            <div class="form-group">
              <div v-for="(d) in app.devices" :key="d.id" class="flex w-full">
                <div class="grid gap-2 grid-cols-2 flex-grow">
                  <div class="col-span control mt-2">
                    <div class="form-group">
                      <input v-model="d.name" required />
                      <label class="control-label" for="input"> {{ $t('deviceName') }} </label>
                      <i class="bar"></i>
                    </div>
                  </div>
                  <div class="col-span control">
                    <div class="form-group">
                      <select v-model="d.sound">
                        <option v-for="r in soundOptions" :key="r.value" :value="r.value">{{ $t(r.translationKey) }}
                        </option>
                      </select>
                      <label class="control-label" for="select"> {{ $t('notificationSound') }} </label>
                      <i class="bar"></i>
                    </div>
                  </div>
                </div>
                <div class="control mx-2">
                  <button class="py-1 px-3 rounded ml-2 mt-6 bg-darkblue-600 text-white hover:text-error"
                    @click="removeDevice(d.id)">
                    <span class="icon-trash"></span>
                    {{ $t('delete') }}
                  </button>
                </div>
              </div>
            </div>
          </div>
          <div class="mb-6 -mt-5 ml-2">
            <wlan-button v-if="!currentPhoneIsConfigured" @click="configuredCurrentPhone" :label="$t('addThisDevice')">
            </wlan-button>
          </div>
        </div>
      </transition>
    </template>

    <div class="pure-u-1-1 tracking-normal mt-2">
      <div class="flex ml-1 my-2">
        <div class="text-blue-800 dark:text-white align-middle flex-grow inline-block">
          <span class="align-middle font-semibold">Telegram</span>
        </div>
        <div class="self-end">
          <wlan-toggle-button v-model="telegram.enabled"></wlan-toggle-button>
        </div>
      </div>
    </div>
    <transition name="fadeHeight">
      <div v-if="telegram.enabled" class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
        <div class="config-form">
          <form>
            <div class="form-group">
              <input type="text" maxlength="50" v-model="telegram.token" required />
              <label class="control-label" for="input">{{ $t("notificationToken") }}</label>
              <i class="bar"></i>
            </div>
            <div class="form-group">
              <input type="text" maxlength="30" v-model="telegram.chat_id" required />
              <label class="control-label" for="input">{{ $t("notificationChatId") }}</label>
              <i class="bar"></i>
            </div>
          </form>
          <div class="mb-4">
            <wlan-button @click="sendTestMessage('telegram', telegram)" :label="$t('notificationSendMessage')">
            </wlan-button>
          </div>
        </div>
      </div>
    </transition>

    <div class="pure-u-1-1 tracking-normal mt-2">
      <div class="flex ml-1 my-2">
        <div class="text-blue-800 dark:text-white align-middle flex-grow inline-block">
          <span class="align-middle font-semibold">Pushover</span>
        </div>
        <div class="self-end">
          <wlan-toggle-button v-model="pushover.enabled"></wlan-toggle-button>
        </div>
      </div>
    </div>
    <transition name="fadeHeight">
      <div v-if="pushover.enabled" class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
        <div class="config-form">
          <div class="form-group">
            <form>
              <div class="form-group">
                <input type="text" maxlength="50" v-model="pushover.token" required />
                <label class="control-label" for="input">{{ $t("notificationToken") }}</label>
                <i class="bar"></i>
              </div>
              <div class="form-group">
                <input type="text" maxlength="30" v-model="pushover.user_key" required />
                <label class="control-label" for="input">{{ $t("notificationUserKey") }}</label>
                <i class="bar"></i>
              </div>
              <div class="form-group">
                <select v-model="pushover.priority">
                  <option v-for="r in priorityOptions" :key="r.value" :value="r.value">{{ $t(r.translationKey) }}</option>
                </select>
                <label class="control-label" for="select">{{ $t("notificationPriority") }}</label>
                <i class="bar"></i>
              </div>
            </form>
            <div class="">
              <wlan-button @click="sendTestMessage('pushover', pushover)" :label="$t('notificationSendMessage')">
              </wlan-button>
            </div>
          </div>
        </div>
      </div>
    </transition>

    <div class="pure-u-1-1">
      <div class="grid mt-2">
        <div class="justify-self-end">
          <wlan-button :label="$t('save')" @click="save">
          </wlan-button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";
import WlanButton from './shared/Button.vue'
import WlanToggleButton from './shared/ToggleButton.vue'

export default {
  name: "PushNotification",
  props: {},
  components: {
    WlanButton,
    WlanToggleButton
  },
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
      currentPhoneId: null,
    };
  },
  watch: {},
  computed: {
    currentPhoneIsConfigured: function () {
      return this.app.devices.some(d => d.id === this.currentPhoneId)
    }
  },
  mounted: function () {
    EventBus.$emit("loading", true)
    this.isMobile = process.env.VUE_APP_PRODUCT_NAME === 'mobile'

    if (this.isMobile) {
      window.flutter_inappwebview.callHandler('getDeviceInfo').then(({ id }) => {
        this.currentPhoneId = id;
      })
    }

    this.axios.get("/getpush").then((response) => {
      this.copyOfPush = Object.assign({}, response.data)
      this.telegram = response.data.telegram
      this.pushover = response.data.pushover
      this.app = response.data.app
      EventBus.$emit("loading", false)
    });
  },
  methods: {
    removeDevice: function (deviceId) {
      this.app.devices = this.app.devices.filter(d => d.id !== deviceId)
    },
    configuredCurrentPhone: async function () {
      EventBus.$emit("loading", true)

      const { id, model } = await window.flutter_inappwebview.callHandler('getDeviceInfo')

      window.flutter_inappwebview
        .callHandler('requestNotificationPermission').then(() => { })

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
    save: function () {
      EventBus.$emit("loading", true)
      // Sets android_channel_id before sending to BE
      this.app.devices.forEach(device => {
        // see [soundOptions]
        if (device.sound == '0') {
          device.android_channel_id = 'wlanthermo_channel_default_id'
        } else if (device.sound == '1') {
          device.android_channel_id = 'wlanthermo_channel_bell_id'
        }
      });

      const requestObj = Object.assign({}, { telegram: this.telegram, pushover: this.pushover, app: this.app })
      this.axios.post('/setpush', requestObj).then(() => {
        EventBus.$emit("loading", false)
        this.backToHome()
      }).catch(async (error) => {
        await window.flutter_inappwebview.callHandler('debug', error)
        EventBus.$emit("loading", false)
      });
    },
    sendTestMessage: function (serviceName, serviceData) {
      let dataObj = { test: true }
      dataObj[serviceName] = serviceData
      const requestObj = Object.assign({}, dataObj)
      this.axios.post('/setpush', requestObj).then(() => { });
    }
  }
};
</script>

<style lang="scss" scoped>
.form-section-name {
  color: #fff;
  padding: 0.3em;
  margin-top: 0.3em;
  font-size: 1.1em;
}
</style>
