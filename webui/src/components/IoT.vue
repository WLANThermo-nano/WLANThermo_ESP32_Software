<template>
  <div class="pure-g m-4">
    <div class="page-title-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="name">
        <span class="back-button cursor-pointer" @click="backToHome">
          <span class="icon-arrow_left"></span>
        </span>
        {{ $t('cloudTitle') }}
        <span @click="showHelpText" class="icon-question_sign icon-question">
        </span>
      </div>
    </div>
    <div class="pure-u-1-1 tracking-normal mt-2">
      <div class="flex ml-1 my-2">
        <div class="text-white align-middle flex-grow inline-block">
          <span class="align-middle font-semibold">{{ $t('cloudActivate') }}</span>
        </div>
        <div class="self-end">
          <wlan-toggle-button v-model="iot.CLon"></wlan-toggle-button>
        </div>
      </div>
    </div>
    <transition name="fadeHeight">
      <div v-if="iot.CLon" class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
        <div class="config-form">
          <form>
            <div class="form-group">
              <select v-model="iot.CLint">
                <option v-for="option in intervalOptions" :key="option.value" :value="option.value">
                  {{ $t(option.translationKey) }}</option>
              </select>
              <label class="control-label" for="select">{{ $t("sendInterval") }}</label>
              <i class="bar"></i>
            </div>
            <div class="form-group cloud-link">
              <label class="control-label">{{ $t("cloudUrl") }}</label>
              <label class="link" for="input">
                <a @click="openCloudUrl">{{ cloudUrl }}</a>
              </label>
              <i class="bar"></i>
            </div>
            <wlan-button 
              :label="$t('cloudBtnToken')" 
              @click="generateToken">
            </wlan-button>
          </form>
        </div>
  
      </div>
    </transition>

    <!-- custom clould -->
    <div class="pure-u-1-1 tracking-normal mt-2">
      <div class="flex ml-1 my-2">
        <div class="text-white align-middle flex-grow">
          <span class="align-middle font-semibold">{{ $t('customCloudActivate') }}</span>
        </div>
        <div class="self-end">
          <wlan-toggle-button v-model="iot.CCLon"></wlan-toggle-button>
        </div>
      </div>
    </div>
    <transition name="fadeHeight">
      <div v-if="iot.CCLon" class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
        <div class="config-form">
          <form>
            <div class="form-group">
              <select v-model="iot.CCLint">
                <option v-for="option in intervalOptions" :key="option.value" :value="option.value">
                  {{ $t(option.translationKey) }}</option>
              </select>
              <label class="control-label" for="select">{{ $t("sendInterval") }}</label>
              <i class="bar"></i>
            </div>
            <div class="form-group cloud-link">
              <input type="text" v-model="iot.CCLurl" required />
              <label class="control-label">{{ $t("customCloudUrl") }}</label>
              <i class="bar"></i>
            </div>
          </form>
        </div>
      </div>
    </transition>

    <!-- MQTT -->
    <div class="pure-u-1-1 tracking-normal mt-2">
      <div class="flex ml-1 my-2">
        <div class="text-white align-middle flex-grow">
          <span class="align-middle font-semibold flex">
            {{ $t('mqttActivate') }}
            <span class="ml-2 text-xl">
              <span @click="showMQTTHelpText" class="icon-question_sign icon-question text-xl"></span>
            </span>
          </span>
        </div>
        <div class="self-end justify-self-end">
          <wlan-toggle-button v-model="iot.PMQon"></wlan-toggle-button>
        </div>
      </div>
    </div>
    <transition name="fadeHeight">
      <div v-if="iot.PMQon" class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
        <div class="config-form mt-2">
          <form>
            <div class="pure-u-1-2">
              <div class="form-group">
                <input type="text" v-model="iot.PMQhost" required />
                <label class="control-label" for="input">{{ $t("mqttHost") }}</label>
                <i class="bar"></i>
              </div>
            </div>
            <div class="pure-u-1-2">
              <div class="form-group">
                <input type="text" v-model="iot.PMQport" required />
                <label class="control-label" for="input">{{ $t("mqttPort") }}</label>
                <i class="bar"></i>
              </div>
            </div>
            <div class="pure-u-1-2">
              <div class="form-group">
                <input type="text" v-model="iot.PMQuser" required />
                <label class="control-label" for="input">{{ $t("mqttUser") }}</label>
                <i class="bar"></i>
              </div>
            </div>
            <div class="pure-u-1-2">
              <div class="form-group">
                <input type="password" v-model="iot.PMQpass" required />
                <label class="control-label" for="input">{{ $t("mqttPwd") }}</label>
                <i class="bar"></i>
              </div>
            </div>
            <div class="pure-u-1-2">
              <div class="form-group">
                <select v-model="iot.PMQint">
                  <option v-for="option in mtqqIntervalOptions" :key="option.value" :value="option.value">
                    {{ $t(option.translationKey) }}</option>
                </select>
                <label class="control-label" for="select">{{ $t("sendInterval") }}</label>
                <i class="bar"></i>
              </div>
            </div>
            <div class="pure-u-1-2">
              <div class="form-group">
                <select v-model="iot.PMQqos">
                  <option value="0">0</option>
                  <option value="1">1</option>
                  <option value="2">2</option>
                </select>
                <label class="control-label" for="input">{{ $t("mqttQos") }}</label>
                <i class="bar"></i>
              </div>
            </div>
          </form>
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
import WlanToggleButton from './shared/ToggleButton.vue'
import WlanButton from './shared/Button.vue'

export default {
  name: "IoT",
  props: {},
  components: {
    WlanToggleButton,
    WlanButton
  },
  computed: {
    cloudUrl: function () {
      return `https://${this.iot.CLurl}?api_token=${this.iot.CLtoken}`
    }
  },
  data: () => {
    return {
      iot: {
        CLint: 15,
        CLon: false,
        CLtoken: '',
        CLurl: '',
        PMQhost: '',
        PMQint: 15,
        PMQon: false,
        PMQpass: '',
        PMQport: null,
        PMQqos: null,
        PMQuser: '',
      },
      mtqqIntervalOptions: [
        { translationKey: 'tenSeconds', value: 10 },
        { translationKey: 'fifteenSeconds', value: 15 },
        { translationKey: 'thirtySeconds', value: 30 },
        { translationKey: 'oneMinutes', value: 60 },
        { translationKey: 'twoMinutes', value: 120 },
        { translationKey: 'fiveMinutes', value: 300 },
      ],
      intervalOptions: [
        { translationKey: 'fifteenSeconds', value: 15 },
        { translationKey: 'thirtySeconds', value: 30 },
        { translationKey: 'oneMinutes', value: 60 },
        { translationKey: 'twoMinutes', value: 120 },
        { translationKey: 'fiveMinutes', value: 300 },
      ]
    };
  },
  watch: {},
  mounted: function () {
    EventBus.$emit("loading", true)
    this.axios.get("/settings").then((response) => {
      this.iot = response.data.iot
      EventBus.$emit("loading", false)
    });
  },
  methods: {
    backToHome: function () {
      EventBus.$emit("back-to-home")
    },
    openCloudUrl: function () {
      if (process.env.VUE_APP_PRODUCT_NAME === 'mobile') {
        window.flutter_inappwebview
          .callHandler('openExternalLink', this.cloudUrl).then(() => { })
      } else {
        window.open(this.cloudUrl, '_blank')
      }
    },
    showHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_iot_title'),
        content: this.$t('help_iot'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/wiki/CloudService',
        linkText: this.$t('help_iot_link')
      })
    },
    showMQTTHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_mqtt_title'),
        content: this.$t('help_mqtt'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/wiki/Private-MQTT',
        linkText: this.$t('help_mqtt_link')
      })
    },
    generateToken: function () {
      EventBus.$emit("loading", true)
      this.axios.post('/newtoken').then((resp) => {
        this.iot.CLtoken = resp.data;
        EventBus.$emit("loading", false)
      }).catch(() => {
        EventBus.$emit("loading", false)
      })
    },
    save: function () {
      EventBus.$emit("loading", true)
      this.axios.post('/setIoT', this.iot).then(() => {
        EventBus.$emit("loading", false)
        this.backToHome()
      }).catch(() => {
        EventBus.$emit("loading", false)
      })
    }
  }
};
</script>

<style lang="scss" scoped>
@import "../assets/colors.scss";

.form-section-name {
  color: #fff;
  padding: 0.3em;
  margin-top: 0.3em;
  font-size: 1.1em;
}

.cloud-link {
  .control-label {
    font-size: 0.8rem;
    color: gray;
    top: -.5rem;
    left: 0;
  }

  .link {
    line-height: 1.9em;

    a {
      margin-left: 0.2em;
      color: #fff;

      &:hover {
        @apply text-primary-400;
      }
    }
  }
}
</style>
