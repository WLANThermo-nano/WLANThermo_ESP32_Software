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
        {{ $t('aboutTitle') }}
      </div>
      <div class="config-form">
        <div 
          v-for="member in members"
          :key="member.name"
          class="member"
          @click="handleNameClick(member.name)">✰ {{member.name}}</div>
      </div>
      <div class="name">
        {{ $t('forumTitle') }}
      </div>
      <div class="config-form">
        <div class="link">
          <a @click="gotoForum">{{$t('forumUrl')}}</a>
        </div>
      </div>
      <div class="you-are-dev-toast" v-if="showToast">
        You are now a developer
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";

const forumUrl = 'https://forum.wlanthermo.de'
const DEBUG_MODE_KEY = '_WLAN_DEBUG_MODE'

export default {
  name: "About",
  props: {},
  data: () => {
    return {
      members: [
        {name: 'Armin Thinnes'},
        {name: 'Alexander Schäfer'},
        {name: 'Björn Schrader'},
        {name: 'Florian Riedl'},
        {name: 'Lukas Jedinger'},
        {name: 'Maik Kaste'},
        {name: 'Martin Körner'},
        {name: 'Steffen Ochs'},
        {name: 'Yu-Hsuan Tsai'}
      ],
      showToast: false,
      isMobile: false,
      debugEnabled: false,
      nameClickCounter: 0,
    };
  },
  watch: {},
  mounted: function () {
    this.isMobile = process.env.VUE_APP_PRODUCT_NAME === 'mobile'
    if (this.isMobile) {
      this.fetchDebugMode()
    }
  },
  methods: {
    fetchDebugMode: async function() {
      const debugModeData = await window.flutter_inappwebview
        .callHandler('getData', DEBUG_MODE_KEY)
      
      this.debugEnabled = debugModeData.value === 'true'
    },
    handleNameClick(name) {
      if (this.debugEnabled || !this.isMobile) return;

      if (name === 'Florian Riedl') this.nameClickCounter += 1
      else this.nameClickCounter = 0

      if (this.nameClickCounter === 7) {
        window.flutter_inappwebview
          .callHandler('saveData', DEBUG_MODE_KEY, JSON.stringify(true))
          .then(() => {
            EventBus.$emit('debug-enabled')
          })

        this.debugEnabled = true;
        this.showToast = true
        setTimeout(() => {
          this.showToast = false
        }, 3000)
      }
    },
    backToHome: function () {
      EventBus.$emit("back-to-home")
    },
    gotoForum: function() {
      if (this.isMobile) {
        window.flutter_inappwebview
        .callHandler('openExternalLink', forumUrl).then(() => {})
      } else {
        window.open(forumUrl, '_blank');
      }
    }
  },
  components: {},
};
</script>

<style lang="scss" scoped>
@import "../assets/colors.scss";

.member {
  color: #fff;
  padding-top: 0.3em;
  padding-bottom: 0.3em;
}

.link a {
  color: #fff;
  text-decoration-line: none;
  cursor: pointer;
  &:hover {
    @apply text-primary-400;
  }
}

.you-are-dev-toast {
  position: absolute;
  bottom: 10vh;
  letter-spacing: 0;
  padding: 10px;
  border-radius: 15px;
  background-color: $medium;
  color: #fff;
  margin: 0 auto;
  left: 0;
  right: 0;
  margin-left: auto;
  margin-right: auto;
  width: 200px;
  text-align: center;
}

</style>
