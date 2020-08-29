<template>
  <div class="pure-g">
    <div class="pure-u-1-1">
      <div class="app-bar-actions">
        <div class="button-container" @click="backToHome">
          <span class="icon-arrow_left"></span>
          <span>{{$t('back')}}</span>
        </div>
      </div>
    </div>
    <div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="name">
        {{ $t('wlanTitle') }}
        <span
          @click="showHelpText"
          class="icon-question_sign icon-question"
        ></span>
      </div>
      <div class="config-form">
        <div class="form-checkbox">
          <label for="activateWlan" class="pure-checkbox checkbox">
            <input v-model="activateWlanChecked" type="checkbox" id="activateWlan" @change="handleWlanUncheck"/> {{$t("wlanActivate")}}
          </label>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";

export default {
  name: "Wlan",
  data: () => {
    return {
      activateWlanChecked: true
    };
  },
  methods: {
    showHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_wlan_title'),
        content: this.$t('help_wlan'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/WLAN',
        linkText: this.$t('help_wlan_link')
      })
    },
    handleWlanUncheck: function () {
      if (this.activateWlanChecked === false) {
        if (confirm(this.$t('stopWifiPrompt')) === true) {
          this.axios.post('/stopwifi').then((resp) => {
            console.log(resp)
            if (resp.data == true) {
              self.location.href = 'about:blank'
            }
          });
        }
      }
    },
    backToHome: function () {
      EventBus.$emit("back-to-home");
    },
  },
  components: {},
  mounted: function () {},
};
</script>

<style lang="scss" scoped>
</style>
