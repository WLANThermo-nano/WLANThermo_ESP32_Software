<template>
  <div class="pure-g">
    <div class="pure-u-1-1 app-bar-wrapper">
      <div class="app-bar-actions">
        <div class="button-container" @click="backToHome">
          <span class="icon-arrow_left"></span>
          <span>{{$t('back')}}</span>
        </div>
      </div>
    </div>
    <div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="name">
        {{ $t('diagnosisTitle') }}
      </div>
      <div class="config-form">
        <div style="max-height: 70vh; max-width: 90vw; overflow-y:scroll;color:#fff; padding: 4px">
          <div v-for="log in logs" :key="log">
            {{ log }}
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";

export default {
  name: "About",
  props: {},
  data: () => {
    return {
      logs: [],
      logsRaw: ''
    };
  },
  watch: {},
  mounted: function () {
    this.fetchLogData()
  },
  methods: {
    fetchLogData: async function() {
      const logData = await window.flutter_inappwebview
        .callHandler('getLogs')
      this.logsRaw = logData.value
      this.logs = JSON.parse(logData.value)
    },
    backToHome: function () {
      EventBus.$emit("back-to-home")
    }
  },
  components: {},
};
</script>

<style lang="scss" scoped>

</style>
