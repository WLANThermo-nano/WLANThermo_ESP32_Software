<template>
  <div class="pure-g m-4 ">
    <div class="page-title-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="name">
        <span class="back-button cursor-pointer" @click="backToHome">
          <span class="icon-arrow_left"></span>
        </span>
        {{ $t('aboutTitle') }}
      </div>
    </div>
    <div class="flex lg:mb-28 xl:mb-32">
      <div class="flex flex-col">
        <div class="config-form-container mt-2 flex lg:mr-12">
          <div class="config-form m-4">
            <div class="text-lg tracking-normal text-blue-800 dark:text-white font-semibold">{{ $t('slogan') }}</div>
            <div class="mt-2 text-blue-800 dark:text-white tracking-normal leading-6 pr-8">
              {{ $t('aboutDescription') }}
            </div>
          </div>
        </div>
        <div class="mt-4 text-center lg:text-left">
          <button 
            @click="gotoForum"
            class="border border-solid border-primary-400 text-primary-400 hover:border-primary-600 hover:text-primary-600 font-semibold py-1.5 px-4 rounded-full">
            {{ $t('forumTitle') }}
          </button>
          <span class="tracking-normal ml-2 text-blue-800 dark:text-white"> {{ $t('forumUrl') }} </span>
        </div>
      </div>
      <div class="hidden lg:block lg:min-w-180 xl:min-w-160 relative">
        <img 
          class="top-0 transform -translate-x-16 translate-y-8 skew-y-3 scale-150 absolute bg-lightblue-800 dark:bg-transparent"
          :src="ourTeamImage">
      </div>
    </div>
    <div class="w-full config-form-container p-2 tracking-normal mt-4">
      <div class="text-blue-800 dark:text-white text-lg font-semibold border-l-4 border-primary-400 h-8 ml-2">
        <span class="ml-2">
          {{ $t('team') }}
        </span>
      </div>
    </div>
    <div class="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-4 ml-12 mt-4 w-full">
      <ul class="list-disc" 
          v-for="member in members"
          :key="member.name">
        <li 
          class="member tracking-normal text-blue-800 dark:text-white"
          @click="handleNameClick(member.name)"> {{member.name}}
        </li>
      </ul>
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
  components: {
  },
  data: () => {
    return {
      ourTeamImage: require(`@/assets/images/our-team.png`),
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
  }
};
</script>

<style lang="scss" scoped>
@import "../assets/colors.scss";

.member {
  padding-top: 0.3em;
  padding-bottom: 0.3em;
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
