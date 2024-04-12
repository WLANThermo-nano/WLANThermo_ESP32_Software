<template>
  <div id="layout relative overflow-hidden">
    <div class="flex w-screen">
      <div id="side-bar" 
        class="bg-lightblue-800 dark:bg-darkblue-800 flex flex-col" :class="{'expanded': expanded}">
        <img @click="toHome()" v-if="expanded" class="block cursor-pointer text-center mx-auto mt-11 mb-1 w-10/12" 
          :src="logoImg" />
        <img @click="toHome()" v-if="!expanded" class="block cursor-pointer text-center mx-auto mt-2 mb-1 w-10/12" 
          :src="smallLogoImg" />  
        <div class="text-white text-right text-sm mb-11 mr-2" v-if="settings.device">
          {{ settings.device.sw_version }}
        </div>
        <div class="pure-menu flex-grow">
          <ul class="pure-menu-list">
            <li class="pl-4 my-5 text-white dark:text-grey-500 hover:text-primary-400 dark:hover:text-primary-400" v-for="item in menuItems" :key="item.id"
              :class="{ 'active': page === item.id }">
              <a @click="toPage(item.id)" class="cursor-pointer flex items-center space-x-3">
                <span :class="'wlan-icons-' + item.icon" class="text-2xl inline-block w-8 text-center">
                </span>
                <span 
                  :class="{'hidden': !expanded}"
                  class="self-center text-base font-semibold whitespace-nowrap">{{ $t(item.translationKey) }}</span>
              </a>
            </li>
          </ul>
        </div>
        <!-- dark/light mode -->
        <div class="text-center mb-4">
          <span 
            class="text-2xl text-white dark:text-grey-500 hover:text-primary-400 dark:hover:bg-darkblue-600 p-3 rounded-full cursor-pointer"
            :class="{'icon-dark_mode': darkMode, 'icon-light_mode': !darkMode}"
            @click="changeTheme">
          </span>
        </div>
      </div>
      <div id="content" class="flex flex-col flex-grow">
        <div id="navbar" class="bg-lightblue-700 dark:bg-darkblue-600 h-12 flex flex-col flex-grow-0 shadow-lg">
          <div class="flex w-full">
            <div class="flex align-middle flex-grow">
              <span v-if="cloudIconClass !== null" 
                class="icon-reorder text-blue-800 dark:text-white hover:text-primary-400 text-2xl inline-block align-middle text-center cursor-pointer mt-3 ml-3"
                @click="expanded = !expanded">
              </span>
            </div>
            <div id="status" class="flex mt-3 align-middle self-end" v-if="system">
              <!-- to do update notice -->
              <span v-if="settings.system.getupdate !== 'false'"
                class="icon-info_sign cursor-pointer text-yellow-600 mt-1.5 mr-3"
                @click="update">
              </span>
              
              <span v-if="cloudIconClass !== null" 
                class="wlan-icons-remote-cloud text-xl inline-block text-center font-bold cursor-pointer mr-3"
                :class="cloudIconClass"
                @click="handleCloudIconClick">
              </span>
              <!-- battery -->
              <template v-if="system && system.soc >= 0">
                <div class="mr-1.5 mt-1 text-blue-800 dark:text-white font-semibold" v-if="showBatteryPercentage"> {{ system.soc }}%</div>
                <div 
                  class="w-6 h-6 mt-0.5 bg-contain bg-no-repeat bg-center mr-2 cursor-pointer"
                  :class="batteryIcon"
                  @click="switchToBatteryText"></div>
              </template>
              <!-- wifi -->
              <template v-if="system && system.rssi">
                <div class="mr-1 mt-1 text-blue-800 dark:text-white font-semibold" v-if="showWifiStrength"> {{ system.rssi }}dBm</div>
                <div 
                  class="w-6 h-6 mt-0.5 bg-contain bg-no-repeat bg-center mr-2 cursor-pointer"
                  :class="wifiIcon"
                  @click="switchToWifiStrength"></div>
              </template>
            </div>
          </div>
        </div>
        <div id="page-content" class="bg-lightblue-600 dark:bg-darkblue-900 flex-grow">
        <div id="content-body" class="m-2">
          <router-view :channels="channels" :pitmasterpm="pitmaster.pm" :unit="system.unit" />
        </div>
      </div>
      </div>
    </div>

    <!-- modal -->
    <div class="dialog-mask w-screen h-screen fixed top-0 left-0" v-if="dialogActive" @click="dialogActive = false"></div>
    <transition name="slideRight">
      <div class="dialog shadow-md fixed flex flex-col w-96 right-0 top-0" v-if="dialogActive">
        <div class="title text-blue-800 dark:text-white bg-white dark:bg-darkblue-600 border-b border-grey-500 pl-4 p-2 font-semibold text-lg">
          {{ dialogTitle }}
          <span @click="dialogActive = false" class="close-btn">×</span>
        </div>
        <div class="body text-blue-800 dark:text-white bg-white dark:bg-darkblue-600 pl-4 p-2 text-base flex-grow">
          <p><span>{{ dialogBodyText }}</span></p>
          <div class="link mt-2" v-if="wikiLink">
            <p>
              {{ $t('see_also') }}
              <a :href="wikiLink" target="_blank"><span class="text-primary-400">Wiki - {{ linkText }}</span></a>
            </p>
          </div>
        </div>
      </div>
    </transition>
    <!-- modal end -->

    <!-- auth dialog -->
    <div class="dialog-mask" v-if="authDialogActive"></div>
    <div class="dialog" v-if="authDialogActive">
      <div class="title">
        {{ $t("authentication") }}
        <span @click="authDialogActive = false" class="close-btn">×</span>
      </div>
      <div class="auth-dialog-body">
        <form>
          <div class="form-group">
            <input type="text" v-model="authUsername" maxlength="30" required>
            <label class="control-label" for="input">{{ $t("username") }}</label>
            <i class="bar"></i>
          </div>
          <div class="form-group">
            <input type="password" v-model="authPass" maxlength="30" required>
            <label class="control-label" for="input">{{ $t("password") }}</label>
            <i class="bar"></i>
          </div>
          <div style="text-aligh: end;">
            <button class="pure-button pure-button-primary-400" @click.stop="onAuthConfirm">
              {{ $t('save') }}
            </button>
          </div>
        </form>
      </div>
    </div>

    <!-- spinner -->
    <div class="dialog-mask" v-if="showSpinner"></div>
    <div class="spinner" v-if="showSpinner"></div>

  </div>
</template>

<script>
import EventBus from './event-bus'
import IconsHelper from './helpers/icons-helper'

const menuItems = [
  { icon: 'search', translationKey: 'menuScan', id: 'scan' },
  { icon: 'home', translationKey: 'menuHome', id: '/' },
  { icon: 'wifi', translationKey: 'menuWlan', id: 'wlan' },
  { icon: 'bluetooth', translationKey: 'menuBluetooth', id: 'bluetooth' },
  { icon: 'settings', translationKey: 'menuSystem', id: 'system' },
  { icon: 'pitmaster', translationKey: 'menuPitmaster', id: 'pitmaster' },
  { icon: 'cloud', translationKey: 'menuIOT', id: 'iot' },
  { icon: 'bell', translationKey: 'menuNotification', id: 'notification' },
  { icon: 'info', translationKey: 'menuAbout', id: 'about' },
  { icon: 'info', translationKey: 'menuDiagnosis', id: 'diagnosis' },
];
const DEBUG_MODE_KEY = '_WLAN_DEBUG_MODE'

export default {
  name: "App",
  data: () => {
    return {
      logoImg: require(`@/assets/logo_${process.env.VUE_APP_PRODUCT_NAME}.svg`),
      smallLogoImg: require(`@/assets/images/small-logo.png`),

      // wifi icon
      wifiIcon: null,
      showWifiStrength: false,
      // Battery
      showBatteryPercentage: false,
      batteryIcon: null,
      // cloud
      cloudIconClass: null,

      // dialog
      dialogActive: false,
      dialogTitle: '',
      dialogBodyText: '',
      wikiLink: '',
      linkText: '',

      // menu
      expanded: false,

      // dark light
      darkMode: true,

      // auth
      authDialogActive: false,
      authUsername: '',
      authPass: '',
      requestToRetry: null,

      // menu
      menuItems: menuItems,

      settings: {
        system: {
          host: 'N.C',
          getupdate: 'false'
        },
        features: {
          bluetooth: true,
          pitmaster: true
        },
        device: {
          sw_version: '',
        }
      },
      system: {
        unit: ''
      },
      channels: [],
      pitmaster: {
        pm: [],
        type: []
      },
      page: 'home',
      navActive: false,
      showSpinner: false,
      isUpdating: false,
      getDataInteval: null,
      appReady: false, // for mobile app
      debugEnabled: false,
    };
  },
  components: {},
  methods: {
    initGetDataPeriodically: function () {
      this.getData();
      this.getDataInteval = setInterval(() => {
        this.getData()
      }, 2000)
    },
    clearGetDataInteval: function () {
      if (this.getDataInteval) {
        clearInterval(this.getDataInteval)
      }
    },
    toPage: function (pageName, query) {
      this.$router.push({ path: pageName, query: query })
      this.page = pageName
      this.navActive = false
    },
    toHome: function () {
      if (process.env.VUE_APP_PRODUCT_NAME === 'mobile') {
        if (this.menuItems.filter(i => i.id === '/').length > 0) {
          this.toPage('/')
        } else {
          this.toPage('scan')
        }
      } else {
        this.toPage('/')
      }
    },
    changeTheme: function() {
      this.darkMode = !this.darkMode

      if (this.darkMode) {
        localStorage.theme = 'dark'
        document.documentElement.classList.add('dark')
      } else {
        localStorage.theme = 'light'
        document.documentElement.classList.remove('dark')
      }
    },
    getData: function () {
      if (this.isUpdating) {
        return;
      }
      this.axios.get('/data').then((response) => {
        const data = response.data
        this.system = data.system
        this.channels = [...data.channel]
        this.pitmaster = data.pitmaster
        this.prepareStatusIcons()
      })
    },
    getSettings: function () {
      this.axios.get('/settings').then((response) => {
        const data = response.data
        this.settings = data
        this.$i18n.locale = this.settings.system.language
        if (!this.settings.features.bluetooth) {
          this.menuItems = this.menuItems.filter(i => i.id !== 'bluetooth')
        }
        if (!this.settings.features.pitmaster) {
          this.menuItems = this.menuItems.filter(i => i.id !== 'pitmaster')
        }
      })
    },
    update: function () {
      const promptText = `${this.$t('update_prompt')}\n\n${this.$t('current_verision')}: ${this.settings.system.version}\n${this.$t('new_version')}: ${this.settings.system.getupdate}`
      if (confirm(promptText) == true) {
        this.showSpinner = true
        this.axios.post('/update').then(() => {
          this.checkUpdateStatus()
          this.isUpdating = true
        })
      }
    },
    checkUpdateStatus: function () {
      this.axios.post('/updatestatus').then((response) => {
        if (response.data == true) {
          setTimeout(() => {
            this.checkUpdateStatus()
          }, 2000)
        } else {
          location.reload()
        }
      })
    },
    handleCloudIconClick: function () {
      this.axios.get('/settings').then((response) => {
        const data = response.data
        this.settings = data
        var url = "https://" + data.iot.CLurl + "?api_token=" + data.iot.CLtoken

        if (process.env.VUE_APP_PRODUCT_NAME === 'mobile') {
          window.flutter_inappwebview
            .callHandler('openExternalLink', url).then(() => { })
        } else {
          window.location = url
        }
      })
    },
    onAuthConfirm: function () {
      const base64Secret = btoa(`${this.authUsername}:${this.authPass}`)
      this.axios.defaults.headers.common['Authorization'] = `Basic ${base64Secret}`;
      this.authDialogActive = false
      this.showSpinner = true
      this.axios(this.requestToRetry).then(() => {
        this.showSpinner = false
      }).catch(() => {
        this.showSpinner = false
      })
    },
    prepareStatusIcons: function () {
      // wifi icon
      const dbm = this.system.rssi
      this.wifiIcon = IconsHelper.getWifiIcon(dbm)

      // Battery
      const level = this.system.soc
      this.batteryIcon = IconsHelper.getBatteryIcon(level, this.system.charge)

      // cloud
      const online = this.system.online
      if (online == 1) {
        this.cloudIconClass = 'text-red'
      } else if (online == 2) {
        this.cloudIconClass = 'text-primary-400'
      } else {
        this.cloudIconClass = null
      }
    },
    switchToWifiStrength: function () {
      this.showWifiStrength = true;
      setTimeout(() => {
        this.showWifiStrength = false;
      }, 5000)
    },
    switchToBatteryText: function () {
      this.showBatteryPercentage = true;
      setTimeout(() => {
        this.showBatteryPercentage = false;
      }, 5000)
    },
    fetchDebugModeAndUpdateMenu: async function () {
      const debugModeData = await window.flutter_inappwebview
        .callHandler('getData', DEBUG_MODE_KEY)

      this.debugEnabled = debugModeData.value === 'true'

      // Adds diagnosis to menu
      if (this.debugEnabled && !this.menuItems.some(it => it.id === 'diagnosis')) {
        this.menuItems.push(
          menuItems.find(it => it.id === 'diagnosis')
        )
      }
    },
  },
  mounted: function () {
    // get theme
    if (localStorage.theme === 'dark' || (!('theme' in localStorage) && window.matchMedia('(prefers-color-scheme: dark)').matches)) {
      document.documentElement.classList.add('dark')
      this.darkMode = true
    } else {
      document.documentElement.classList.remove('dark')
      this.darkMode = false
    }

    if (process.env.VUE_APP_PRODUCT_NAME === 'mobile') {
      EventBus.$emit('log', 'is mobile')
      this.$router.push('/scan')
      this.settings.system.host = this.$t('mobileAppHeader')
      this.menuItems = menuItems.filter(i => (i.id === 'scan') || (i.id === 'about'))

      setTimeout(() => {
        this.fetchDebugModeAndUpdateMenu()
      }, 1000)
    } else {
      this.menuItems = menuItems.filter(i => i.id !== 'scan' && i.id !== 'diagnosis')
      this.getSettings()
      this.initGetDataPeriodically()
    }
    EventBus.$on('show-help-dialog', (dialogData) => {
      this.dialogTitle = dialogData.title
      this.dialogBodyText = dialogData.content
      this.wikiLink = dialogData.wikiLink
      this.linkText = dialogData.linkText
      this.dialogActive = true
    })
    EventBus.$on('back-to-home', () => {
      this.toHome();
    })
    EventBus.$on('debug-enabled', () => {
      this.fetchDebugModeAndUpdateMenu()
    })
    EventBus.$on('device-selected', () => {
      this.clearGetDataInteval()
      this.toPage('/')
      this.menuItems = menuItems
      this.getSettings()
      this.initGetDataPeriodically()
    })
    EventBus.$on('show-auth-popup', (axiosError) => {
      if (process.env.VUE_APP_PRODUCT_NAME === 'mobile') {
        this.requestToRetry = axiosError.config
        this.authDialogActive = true
      }
    })
    EventBus.$on('api-error', () => {
      this.showSpinner = false
      if (process.env.VUE_APP_PRODUCT_NAME === 'mobile') {
        this.clearGetDataInteval()
        this.toPage('scan', { connectionLost: true })
      }
    })
    EventBus.$on('loading', (value) => {
      this.showSpinner = value
    })
    EventBus.$on('log', (logMessage) => {
      window.flutter_inappwebview.callHandler('log', logMessage)
    })
    EventBus.$on('getData', () => {
      this.getData()
    })
    EventBus.$on('getSettings', () => {
      this.getSettings()
    })
  }
};
</script>

<style lang="scss">
@import "./assets/index.css";
@import "./assets/global.scss";

#layout {
  display: flex;
  align-items: stretch;
}

#main {
  flex: 1 1 auto;
}

#side-bar {
  height: 100dvh;
  transition: all 0.2s ease-out, background-color 0ms;
  width: 4rem;
  min-width: 4rem;
  max-width: 4rem;
  &.expanded {
    width: 14rem;
    min-width: 14rem;
    max-width: 14rem;
  }
}

.page-content {
  padding: 44px 0px;
}

.headmenu {
  padding: 0 0 0 5px;
  letter-spacing: normal;
  line-height: 45px;
  font-size: 1.2em;
}

// hamburger
.menu-link {
  position: absolute;
  display: none;
  cursor: pointer;
  top: 0;
  background: $medium;
  font-size: 10px;
  z-index: 10;
  width: 2em;
  height: auto;
  padding: 2.1em 1.6em 2.1em 0.5em;
  float: left;

  &:hover,
  &:focus {
    // background: $medium_dark;
  }

  span {
    position: relative;
    display: block;
  }

  span,
  span:before,
  span:after {
    background-color: #fff;
    width: 100%;
    height: 0.2em;
  }

  span:before,
  span:after {
    position: absolute;
    margin-top: -0.6em;
    content: " ";
  }

  span:after {
    margin-top: 0.6em;
  }
}

.nav-mask,
.dialog-mask {
  position: fixed;
  width: 100dvw;
  height: 100dvh;
  background-color: rgba(0, 0, 0, 0.3);
  display: none;
  z-index: 400;

  &.dialog-mask {
    display: block;
  }
}

.dialog {
  height: 100dvh;
  z-index: 500;

  .title {
    .close-btn {
      font-size: 1.5em;
      line-height: 0.9em;
      cursor: pointer;
      float: right;

      &:hover {
        color: $medium;
      }
    }
  }
}

.auth-dialog-body {
  padding: 1em;
  width: 40vw;
  text-align: end;

  input {
    color: $dark !important;
  }
}

@media screen and (max-width: 48em) {

  .nav-mask {
    display: block;
  }

  .headmenu {
    width: 100%;
    left: 0;

    .title {
      margin-left: 2.2em;
    }
  }

  .menu-link {
    display: block;
  }

  .auth-dialog-body {
    width: 80vw;
  }
}

.wifi,
.notification,
.cloud {
  text-align: center;
  padding: 14px 2px;
  overflow: hidden;
  float: right;
  border-left: 2px solid #333;
}

.battery {
  text-align: center;
  padding: 14px 5px;
  float: right;
}
</style>