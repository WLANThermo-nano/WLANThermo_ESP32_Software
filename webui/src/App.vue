<template>
  <div id="layout">
    <div class="headmenu">
      <span id="menuLink" class="menu-link" @click="navActive = !navActive">
        <!-- Hamburger icon -->
        <span></span>
      </span>
      <div class="title">
        {{ settings.system.host }}
      </div>
      <div class="status" v-if="system">
        <!-- to do update notice -->
        <Icon v-if="settings.system.getupdate !== 'false'" class="cursor-pointer" @click="update" iconClass="info_sign icon-yellow" />
        
        <Icon v-if="cloudIconClass !== null" class="cursor-pointer" @click="handleCloudIconClick" :iconClass="cloudIconClass" />
        <!-- charging -->
        <Icon v-if="system.charge" iconClass="power-cord" />
        <!-- battery -->
        <template v-if="system && system.soc >= 0">
          <div class="mr5" v-if="showBatteryPercentage"> {{ system.soc }}%</div>
          <Icon class="cursor-pointer" @click="switchToBatteryText" v-else :iconClass="batteryIconClass" />
        </template>
        <!-- wifi -->
        <template v-if="system && system.rssi">
          <div class="mr5" v-if="showWifiStrength"> {{ system.rssi }}dBm</div>
          <Icon class="cursor-pointer" @click="switchToWifiStrength" v-else :iconClass="wifiIconClass" />
        </template>
      </div>
    </div>
    <div id="nav" :class="{ active: navActive }">
      <img @click="toHome()" class="logo" :src="logoImg" style="width: 85%" />
      <div class="version" v-if="settings.device">
        {{ settings.device.sw_version }}
      </div>
      <div class="pure-menu">
        <ul class="pure-menu-list">
          <li class="pure-menu-item" v-for="item in menuItems" :key="item.id" :class="{ 'active':  page === item.id}">
            <a @click="toPage(item.id)" class="pure-menu-link cursor-pointer">
              <span class="menu-icon" :class="'icon-' + item.icon"></span>
              {{ $t(item.translationKey) }}
            </a>
          </li>
        </ul>
      </div>
    </div>
    <div class="nav-mask" v-if="navActive" @click="navActive = false"></div>
    <div id="main">
      <div class="page-content">
        <div class="content-body">
          <router-view :channels="channels" :pitmasterpm="pitmaster.pm" :unit="system.unit" />
        </div>
      </div>
    </div>
    
    <!-- modal -->
    <div class="dialog-mask" v-if="dialogActive" @click="dialogActive = false"></div>
    <div class="dialog" v-if="dialogActive">
      <div class="title">
        {{ dialogTitle }}
        <span @click="dialogActive = false" class="close-btn">×</span>
      </div>
      <div class="body">
        <p><span>{{dialogBodyText}}</span></p>
        <div class="link" v-if="wikiLink">
          <p>
            {{ $t('see_also') }}
            <a :href="wikiLink" target="_blank"><span style="color:#3366ff">Wiki - {{ linkText }}</span></a>
          </p>
        </div>
      </div>
    </div>
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
              <label class="control-label" for="input">{{$t("username")}}</label>
              <i class="bar"></i>
            </div>
            <div class="form-group">
              <input type="password" v-model="authPass" maxlength="30" required>
              <label class="control-label" for="input">{{$t("password")}}</label>
              <i class="bar"></i>
            </div>
            <div style="text-aligh: end;">
              <button class="pure-button pure-button-primary" @click.stop="onAuthConfirm">
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
import Icon from './components/Icon.vue'
import EventBus from './event-bus'
import IconsHelper from './helpers/icons-helper'

const menuItems = [
        { icon: 'search', translationKey: 'menuScan', id: 'scan' },
        { icon: 'home', translationKey: 'menuHome', id: '/' },
        { icon: 'Wlan100', translationKey: 'menuWlan', id: 'wlan' },
        { icon: 'bluetooth_1', translationKey: 'menuBluetooth', id: 'bluetooth' },
        { icon: 'cog', translationKey: 'menuSystem', id: 'system' },
        { icon: 'fire', translationKey: 'menuPitmaster', id: 'pitmaster' },
        { icon: 'cloud', translationKey: 'menuIOT', id: 'iot' },
        { icon: 'bell', translationKey: 'menuNotification', id: 'notification' },
        { icon: 'info_sign', translationKey: 'menuAbout', id: 'about' },
        { icon: 'info_sign', translationKey: 'menuDiagnosis', id: 'diagnosis' },
];
const DEBUG_MODE_KEY = '_WLAN_DEBUG_MODE'

export default {
  name: "App",
  data: () => {
    return {
      logoImg: require(`@/assets/logo_${process.env.VUE_APP_PRODUCT_NAME}.svg`),

      // wifi icon
      wifiIconClass: null,
      showWifiStrength: false,
      // Battery
      showBatteryPercentage: false,
      batteryIconClass: null,
      // cloud
      cloudIconClass: null,

      // dialog
      dialogActive: false,
      dialogTitle: '',
      dialogBodyText: '',
      wikiLink: '',
      linkText: '',

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
  components: {
    Icon
  },
  methods: {
    initGetDataPeriodically: function() {
      this.getData();
      this.getDataInteval = setInterval(() => {
        this.getData()
      }, 2000)
    },
    clearGetDataInteval: function() {
      if (this.getDataInteval) {
        clearInterval(this.getDataInteval)
      }
    },
    toPage: function(pageName, query) {
      this.$router.push({ path: pageName, query: query})
      this.page = pageName
      this.navActive = false
    },
    toHome: function() {
      if (process.env.VUE_APP_PRODUCT_NAME === 'mobile') {
        if(this.menuItems.filter(i => i.id === '/').length > 0 ) {
          this.toPage('/')
        } else {
          this.toPage('scan')
        }
      } else {
        this.toPage('/')
      }
    },
    getData: function() {
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
    getSettings: function() {
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
    update: function() {
      const promptText = `${this.$t('update_prompt')}\n\n${this.$t('current_verision')}: ${this.settings.system.version}\n${this.$t('new_version')}: ${this.settings.system.getupdate}`
      if (confirm(promptText) == true) {
        this.showSpinner = true
        this.axios.post('/update').then(() => {
          this.checkUpdateStatus()
          this.isUpdating = true
        })
      }
    },
    checkUpdateStatus: function() {
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
    handleCloudIconClick: function() {
      this.axios.get('/settings').then((response) => {
        const data = response.data
        this.settings = data
        var url = "https://" + data.iot.CLurl + "?api_token=" + data.iot.CLtoken

        if (process.env.VUE_APP_PRODUCT_NAME === 'mobile') {
          window.flutter_inappwebview
            .callHandler('openExternalLink', url).then(() => {})
        } else {
          window.location = url
        }
      })
    },
    onAuthConfirm: function() {
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
    prepareStatusIcons: function() {
      // wifi icon
      const dbm = this.system.rssi
      this.wifiIconClass = IconsHelper.getWifiIcon(dbm)

      // Battery
      const percent = this.system.soc
      if (percent >= '90') {
        this.batteryIconClass = 'battery-100'
      } else if (percent >= '75') {
        this.batteryIconClass = 'battery-75'
      } else if (percent >= '50') {
        this.batteryIconClass = 'battery-50'
      } else if (percent >= '15') {
        this.batteryIconClass = 'battery-25'
      } else if (percent >= '10') {
        this.batteryIconClass = 'battery-0 icon-red'
      } else {
        this.batteryIconClass = 'battery-0 icon-red icon-blinker'
      }
      // cloud
      const online = this.system.online
      if (online == 1) {
        this.cloudIconClass = 'cloud icon-red'
      } else if (online == 2) {
        this.cloudIconClass = 'cloud icon-green'
      } else {
        this.cloudIconClass = null
      }
    },
    switchToWifiStrength: function() {
      this.showWifiStrength = true;
      setTimeout(() => {
        this.showWifiStrength = false;
      }, 5000)
    },
    switchToBatteryText: function() {
      this.showBatteryPercentage = true;
      setTimeout(() => {
        this.showBatteryPercentage = false;
      }, 5000)
    },
    fetchDebugModeAndUpdateMenu: async function() {
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
  mounted: function() {
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
@import "./assets/global.scss";

#layout {
  display: flex;
  align-items: stretch;
}

#nav {
  flex: 0 0 $nav_width;
  min-height: 100vh;
  background-color: $medium;
}

#main {
  flex: 1 1 auto;
}

.headmenu {
  position: fixed;
  z-index: 5;
  display: flex;
  width: calc(100% - #{$nav_width});
  left: $nav_width;
  background-color: $medium;
  color: #ccc;
  height: 44px;
  .title {
    flex: 1 1 auto;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
  .status {
    display: flex;
    flex: 0 0 auto;
    padding-right: 0.5em;
  }
}

.version {
  color: #fff;
  text-align: right;
  font-size: 0.9em;
  padding-right: 8.3%;
}

.logo {
  margin: 15px auto 10px auto;
  display: block;
  cursor: pointer;
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
  &:hover, &:focus {
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

.nav-mask, .dialog-mask {
  position: fixed;
  width: 100vw;
  height: 100vh;
  background-color: rgba(0, 0, 0, 0.3);
  display: none;
  z-index: 499;
  &.dialog-mask {
    display: block;
  }
}

.dialog {
  position: fixed;
  left: 50%;
  top: 15vh;
  transform: translateX(-50%);
  background-color: #fff;
  z-index: 500;
  .title {
    padding: 0.3em;
    font-size: 1.5em;
    background-color: $primary;
    color: #fff;
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
  .body {
    padding: 0.7em;
  }
}

.pure-menu-item {
  &:hover {
    background-color: $medium_dark;
  }
  &.active {
    background-color: #fff;
    .pure-menu-link {
      background-color: #fff;
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
  #nav {
    flex-basis: 200px;
    z-index: 500;
    width: 12.5em;
    left: -12.5em;
    position: fixed;
    transition: left .4s ease;
    &.active {
      display: block;
      left: 0;
    }
  }
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

.menu-icon {
  display: inline-block;
  width: 21px;
  margin-right: 0.2em;
  text-align: center;
}

</style>