<template>
  <div id="layout" v-if="settings !== null">
    <div class="headmenu">
      <span id="menuLink" class="menu-link" @click="navActive = !navActive">
        <!-- Hamburger icon -->
        <span></span>
      </span>
      <div class="title">
        {{ settings.system.host }}
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
          <li class="pure-menu-item">
            <a href="#" class="pure-menu-link">System</a>
          </li>
          <li class="pure-menu-item">
            <a href="#" class="pure-menu-link">Pitmaster</a>
          </li>
          <li class="pure-menu-item">
            <a href="#" class="pure-menu-link">IoT</a>
          </li>
          <li class="pure-menu-item">
            <a href="#" class="pure-menu-link">Push Notification</a>
          </li>
          <li class="pure-menu-item">
            <a href="#" class="pure-menu-link">Über</a>
          </li>
        </ul>
      </div>
    </div>
    <div class="nav-mask" v-if="navActive" @click="navActive = false"></div>
    <div id="main">
      <div class="page-content">
        <div class="content-body">
          <Home v-if="page === 'home'" />
          <Wlan v-else-if="page === 'wlan'" />
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import Home from './components/Home.vue'
import Wlan from './components/Wlan.vue'

export default {
  name: "App",
  data: () => {
    return {
      settings: {},
      system: null,
      channel: [],
      pitmaster: null,
      page: 'home',
      navActive: false,
      boxes: [1, 2, 3, 4, 5, 6, 7, 8],
      colors: [
        "#a349a4",
        "#22b14c",
        "#ef562d",
        "#ffc100",
        "#0c4c88",
        "#804000",
        "#5587a2",
        "#a4ca81",
      ],
    };
  },
  components: {
    Home, Wlan
  },
  methods: {
    getData: function() {
      setInterval(() => {
        this.axios.get('/data').then((response) => {
          const data = response.data
          this.system = data.system
          this.channel = data.channel
          this.pitmaster = data.pitmaster
        })
      }, 2000)
    },
    getSettings: function() {
      this.axios.get('/settings').then((response) => {
        const data = response.data
        this.settings = data
        this.$i18n.locale = this.settings.system.language
      })
    },
  },
  mounted: function() {
    this.getSettings();
    this.getData();
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
  width: calc(100% - #{$nav_width});
  left: $nav_width;
  background-color: $medium;
  color: #ccc;
  height: 44px;
}

.logo {
  margin: 15px auto 25px auto;
  display: block;
}

.page-content {
  padding: 44px 2px;
}

.headmenu {
  padding: 0 0 0 15px;
  letter-spacing: normal;
  line-height: 40px;
  font-size: 1.2em;
}

.info-box {
  margin: 5px;
  padding: 10px;
  height: 100px;
  color: #fff;
  background-color: $medium_dark;
  border-left: 8px solid;
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
  position: absolute;
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
    position: absolute;
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
      margin-left: 2em;
    }
  }
  .menu-link {
    display: block;
  }
}


</style>
