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
        {{ $t('notificationTitle') }}
        <span
          @click="showHelpText"
          class="icon-question_sign icon-question"
        ></span>
      </div>
      <div class="config-form">

      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";

export default {
  name: "Wlan",
  data: () => {
    return {};
  },
  methods: {
    showHelpText: function () {
      EventBus.$emit("show-help-dialog", {
        title: "WLAN Einstellungen",
        content:
          '<p><span>Verbinde dein WLANThermo mit deinem Heimnetzwerk und erweitere somit die Funktionsvielfalt deines WLANThermo.</span></p><p><span>Siehe auch: </span><a href="https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/WLAN" target="_blank"><span style="color:#3366ff">Wiki - WLAN</span></a></p>',
      });
    },
    backToHome: function () {
      EventBus.$emit("back-to-home");
    }
  },
  components: {},
  mounted: function () {},
};
</script>

<style lang="scss" scoped>
</style>
