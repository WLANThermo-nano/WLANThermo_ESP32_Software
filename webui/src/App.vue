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
      <img @click="toPage('home')" class="logo" :src="logoImg" style="width: 85%" />
      <div class="version">
        {{ settings.device.sw_version }}
      </div>
      <div class="pure-menu">
        <ul class="pure-menu-list">
          <li class="pure-menu-item" v-for="item in menuItems" :key="item.id">
            <a @click="toPage(item.id)" href="#" class="pure-menu-link">
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
          <Home v-if="page === 'home'" :channels="channels" :pitmasterpm="pitmaster.pm" :unit="system.unit"/>
          <Wlan v-else-if="page === 'wlan'" />
          <System v-else-if="page === 'system'" />
          <PushNotification v-else-if="page === 'notification'" />
          <Bluetooth v-else-if="page === 'bluetooth'" />
          <Pitmaster v-else-if="page === 'pitmaster'" />
          <IoT v-else-if="page === 'iot'" />
          <About v-else-if="page === 'about'" />
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

    <!-- spinner -->
    <div class="dialog-mask" v-if="showSpinner"></div>
    <div class="spinner" v-if="showSpinner"></div>

  </div>
</template>

<script>
import Home from './components/Home.vue'
import Wlan from './components/Wlan.vue'
import Icon from './components/Icon.vue'
import System from './components/System.vue'
import Bluetooth from './components/Bluetooth.vue'
import Pitmaster from './components/Pitmaster'
import About from './components/About'
import IoT from './components/IoT'
import PushNotification from './components/PushNotification'
import EventBus from './event-bus'
import IconsHelper from './helpers/icons-helper'

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

      // menu
      menuItems: [
        { icon: 'home', translationKey: 'menuHome', id: 'home' },
        { icon: 'Wlan100', translationKey: 'menuWlan', id: 'wlan' },
        { icon: 'bluetooth_2', translationKey: 'menuBluetooth', id: 'bluetooth' },
        { icon: 'cog', translationKey: 'menuSystem', id: 'system' },
        { icon: 'fire', translationKey: 'menuPitmaster', id: 'pitmaster' },
        { icon: 'cloud', translationKey: 'menuIOT', id: 'iot' },
        { icon: 'bell', translationKey: 'menuNotification', id: 'notification' },
        { icon: 'info_sign', translationKey: 'menuAbout', id: 'about' },
      ],

      settings: {
        system: {
          host: 'N.C'
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
    };
  },
  components: {
    Home, Wlan, Icon, System, PushNotification, Bluetooth, Pitmaster, IoT, About
  },
  methods: {
    initGetDataPeriodically: function() {
      this.getData();
      setInterval(() => {
        this.getData()
      }, 2000)
    },
    toPage: function(pageName) {
      this.page = pageName
      this.navActive = false
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
        window.location = "https://" + data.iot.CLurl + "?api_token=" + data.iot.CLtoken;
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
    }
  },
  mounted: function() {
    this.getSettings()
    this.initGetDataPeriodically()
    EventBus.$on('show-help-dialog', (dialogData) => {
      this.dialogTitle = dialogData.title
      this.dialogBodyText = dialogData.content
      this.wikiLink = dialogData.wikiLink
      this.linkText = dialogData.linkText
      this.dialogActive = true
    })
    EventBus.$on('back-to-home', () => {
      this.page = 'home'
    })
    EventBus.$on('loading', (value) => {
      this.showSpinner = value
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
