<template>
  <div id="layout">
    <div class="headmenu">
      <a href="#menu" id="menuLink" class="menu-link">
        <!-- Hamburger icon -->
        <span></span>
      </a>
      <div>
        NANO-84b3ec
      </div>
    </div>
    <div id="nav">
      <a href="#" class="nav-menu-button">Menu</a>

      <div class="nav-inner">
        <img class="logo" src="./assets/logo.png" />
        <div class="pure-menu">
          <ul class="pure-menu-list">
            <li class="pure-menu-item">
              <a href="#" class="pure-menu-link">Home</a>
            </li>
            <li class="pure-menu-item">
              <a href="#" class="pure-menu-link">WLAN</a>
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
    </div>

    <div id="main">
      <div class="page-content">
        <div class="content-body">
          <div class="pure-g">
            <div class="pure-u-1-4 pure-u-md-1-2 pure-u-xl-1-4" v-for="(b, i) in boxes" :key="b">
              <div class="info-box" v-bind:style="{borderColor: colors[i]}">
                {{b}} some info
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
// import HelloWorld from './components/HelloWorld.vue'

export default {
  name: "App",
  data: () => {
    return {
      boxes: [1, 2, 3, 4, 5, 6, 7, 8],
      colors: ['#a349a4', '#22b14c', '#ef562d', '#ffc100', '#0c4c88', '#804000', '#5587a2', '#a4ca81']
    }
  },
  components: {
    // HelloWorld
  },
};
</script>

<style lang="scss">
@import "./assets/global.scss";

.headmenu {
  position: fixed;
  width: calc(100% - #{$nav_width});
  left: $nav_width;
  background-color: $medium;
  color: #ccc;
  height: 44px;
}

.logo {
  margin: 0 auto 25px auto;
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
  background-color: $dar_medium;
  border-left: 8px solid;
}
</style>
