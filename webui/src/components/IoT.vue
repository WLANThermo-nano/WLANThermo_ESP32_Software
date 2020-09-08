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
        {{ $t('cloudTitle') }}
        <span
          @click="showHelpText"
          class="icon-question_sign icon-question"
        ></span>
      </div>
      <div class="config-form">
        <form>
          <div class="form-checkbox">
            <label for="cloudActivate" class="pure-checkbox checkbox">
              <input v-model="iot.CLon" :true-value="'true'" :false-value="'false'" type="checkbox" id="cloudActivate" />
              {{$t("cloudActivate")}}
            </label>
          </div>
          <div class="form-group">
            <select v-model="iot.CLint">
              <option
                v-for="option in intervalOptions"
                :key="option.value"
                :value="option.value"
              >{{$t(option.translationKey)}}</option>
            </select>
            <label class="control-label" for="select">{{$t("sendInterval")}}</label>
            <i class="bar"></i>
          </div>
          <div class="form-group cloud-link">
            <label for="input">
              <a :href="cloudUrl">{{cloudUrl}}</a>
            </label>
            <i class="bar"></i>
          </div>
          <button class="pure-button pure-button-primary" @click.stop="generateToken">
            {{ $t('cloudBtnToken') }}
          </button>
        </form>
      </div>
      <div class="name">
        {{ $t('mqttTitle') }}
        <span
          @click="showMQTTHelpText"
          class="icon-question_sign icon-question"
        ></span>
      </div>
      <div class="config-form">
        <form>
          <div class="form-checkbox">
            <label for="mqttActivate" class="pure-checkbox checkbox">
              <input v-model="iot.PMQon" :true-value="'true'" :false-value="'false'" type="checkbox" id="mqttActivate" />
              {{$t("mqttActivate")}}
            </label>
          </div>
          <div class="pure-u-1-2">
            <div class="form-group mr5">
              <input type="text" v-model="iot.PMQhost" required />
              <label class="control-label" for="input">{{$t("mqttHost")}}</label>
              <i class="bar"></i>
            </div>
          </div>
          <div class="pure-u-1-2">
            <div class="form-group mr5">
              <input type="text" v-model="iot.PMQport" required />
              <label class="control-label" for="input">{{$t("mqttPort")}}</label>
              <i class="bar"></i>
            </div>
          </div>
          <div class="pure-u-1-2">
            <div class="form-group mr5">
              <input type="text" v-model="iot.PMQuser" required />
              <label class="control-label" for="input">{{$t("mqttUser")}}</label>
              <i class="bar"></i>
            </div>
          </div>
          <div class="pure-u-1-2">
            <div class="form-group mr5">
              <input type="text" v-model="iot.PMQpass" required />
              <label class="control-label" for="input">{{$t("mqttPwd")}}</label>
              <i class="bar"></i>
            </div>
          </div>
          <div class="pure-u-1-2">
            <div class="form-group mr5">
              <select v-model="iot.PMQint" class="select-in-row">
                <option
                  v-for="option in mtqqIntervalOptions"
                  :key="option.value"
                  :value="option.value"
                >{{$t(option.translationKey)}}</option>
              </select>
              <label class="control-label" for="select">{{$t("sendInterval")}}</label>
              <i class="bar"></i>
            </div>
          </div>
          <div class="pure-u-1-2">
            <div class="form-group mr5">
              <input type="text" v-model="iot.PMQqos" required />
              <label class="control-label" for="input">{{$t("mqttQos")}}</label>
              <i class="bar"></i>
            </div>
          </div>
        </form>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";

export default {
  name: "IoT",
  props: {},
  computed: {
    cloudUrl: function() {
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
        {translationKey: 'tenSeconds', value: 10},
        {translationKey: 'fifteenSeconds', value: 15},
        {translationKey: 'thirtySeconds', value: 30},
        {translationKey: 'oneMinutes', value: 60},
        {translationKey: 'twoMinutes', value: 120},
        {translationKey: 'fiveMinutes', value: 300},
      ],
      intervalOptions: [
        {translationKey: 'fifteenSeconds', value: 15},
        {translationKey: 'thirtySeconds', value: 30},
        {translationKey: 'oneMinutes', value: 60},
        {translationKey: 'twoMinutes', value: 120},
        {translationKey: 'fiveMinutes', value: 300},
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
    showHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_iot_title'),
        content: this.$t('help_iot'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/Nano-Cloud-Service',
        linkText: this.$t('help_iot_link')
      })
    },
    showMQTTHelpText: function() {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_mqtt_title'),
        content: this.$t('help_mqtt'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/Private-MQTT',
        linkText: this.$t('help_mqtt_link')
      })
    },
    generateToken: function() {
      EventBus.$emit("loading", true)
      this.axios.post('/newtoken').then((resp) => {
        this.iot.CLtoken = resp.data;
        EventBus.$emit("loading", false)
      }).catch(() => {
        EventBus.$emit("loading", false)
      })
    },
    save: function() {
      EventBus.$emit("loading", true)
      this.axios.post('/setIoT', this.iot).then(() => {
        EventBus.$emit("loading", false)
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
@import "../assets/colors.scss";

.cloud-link {
  a {
    color: #fff;
    &:hover {
      color: $input_highlight_color;
    }
  }
}

.select-in-row {
  margin-top: 0.75em;
}

</style>
