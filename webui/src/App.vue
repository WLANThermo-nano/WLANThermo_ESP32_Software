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
        <Icon v-if="settings.system.getupdate !== 'false'" class="cursor-pointer" @click="handleCloudIconClick" iconClass="notification icon-yellow" />
        
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
      <img class="logo" src="./assets/logo_nano.svg" style="width: 85%" />
      <div class="pure-menu">
        <ul class="pure-menu-list">
          <li class="pure-menu-item">
            <a @click="page = 'home'" href="#" class="pure-menu-link">{{ $t("menuHome") }}</a>
          </li>
          <li @click="page = 'wlan'" class="pure-menu-item">
            <a href="#" class="pure-menu-link">{{ $t("menuWlan") }}</a>
          </li>
          <li @click="page = 'wlan'" class="pure-menu-item">
            <a href="#" class="pure-menu-link">{{ $t("menuBluetooth") }}</a>
          </li>
          <li @click="page = 'system'" class="pure-menu-item">
            <a href="#" class="pure-menu-link">{{ $t("menuSystem") }}</a>
          </li>
          <li @click="page = 'pitmaster'" class="pure-menu-item">
            <a href="#" class="pure-menu-link">{{ $t("menuPitmaster") }}</a>
          </li>
          <li @click="page = 'iot'" class="pure-menu-item">
            <a href="#" class="pure-menu-link">{{ $t("menuIOT") }}</a>
          </li>
          <li @click="page = 'notification'" class="pure-menu-item">
            <a href="#" class="pure-menu-link">{{ $t("menuNotification") }}</a>
          </li>
          <li @click="page = 'about'" class="pure-menu-item">
            <a href="#" class="pure-menu-link">{{ $t("menuAbout") }}</a>
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
          <div v-else>
            {{ page }} comming soon
          </div>
        </div>
      </div>
    </div>
    
<!-- modal -->
<a href="#modal-one" class="btn btn-big">Modal!</a>
<div class="modal" id="modal-one" aria-hidden="true">
  <div class="modal-dialog">
    <div class="modal-header">
      <h2>Modal in CSS?</h2>
      <a href="#" class="btn-close" aria-hidden="true">×</a>
    </div>
    <div class="modal-body">
      <p>One modal example here! :D</p>
    </div>
    <div class="modal-footer">
      <a href="#" class="btn">Nice!</a>
    </div>
  </div>
</div>
<!-- modal end -->

  </div>
</template>

<script>
import Home from './components/Home.vue'
import Wlan from './components/Wlan.vue'
import Icon from './components/Icon.vue'
import EventBus from './event-bus';

export default {
  name: "App",
  data: () => {
    return {
      // wifi icon
      wifiIconClass: null,
      showWifiStrength: false,
      // Battery
      showBatteryPercentage: false,
      batteryIconClass: null,
      // cloud
      cloudIconClass: null,

      settings: {
        system: {
          host: 'N.C'
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
      navActive: false
    };
  },
  components: {
    Home, Wlan, Icon
  },
  methods: {
    initGetDataPeriodically: function() {
      this.getData();
      setInterval(() => {
        this.getData();
      }, 2000)
    },
    getData: function() {
      this.axios.get('/data').then((response) => {
        const data = response.data
        this.system = data.system
        this.channels = data.channel
        this.pitmaster = data.pitmaster
        this.prepareStatusIcons()
      })
    },
    getSettings: function() {
      this.axios.get('/settings').then((response) => {
        const data = response.data
        this.settings = data
        this.$i18n.locale = this.settings.system.language
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
      if (dbm >= '-80') {
        this.wifiIconClass = 'Wlan100'
      } else if (dbm >= '-95') {
        this.wifiIconClass = 'Wlan66'
      } else if (dbm >= '-105') {
        this.wifiIconClass = 'Wlan33'
      }

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
    this.getSettings();
    this.initGetDataPeriodically();
    EventBus.$on('show-help-dialog', (dialogData) => {
      // dialogData.title
      // dialogData.content
      console.log(dialogData)
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
  }
  .status {
    display: flex;
    flex: 0 0 auto;
    padding-right: 0.5em;
  }
}

.logo {
  margin: 15px auto 25px auto;
  display: block;
}

.page-content {
  padding: 44px 0px;
}

.headmenu {
  padding: 0 0 0 15px;
  letter-spacing: normal;
  line-height: 40px;
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
  padding: 2.1em 1.6em;
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

.nav-mask {
  position: fixed;
  width: 100vw;
  height: 100vh;
  background-color: rgba(0, 0, 0, 0.3);
  display: none;
  z-index: 499;
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
      margin-left: 3em;
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

</style>
