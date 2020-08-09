<template>
  <div id="layout">
    <div class="headmenu">
      <span id="menuLink" class="menu-link" @click="navActive = !navActive">
        <!-- Hamburger icon -->
        <span></span>
      </span>
      <div class="title">
        NANO-84b3ec
      </div>
    </div>
    <div id="nav" :class="{ active: navActive }">
      <img class="logo" src="./assets/logo_nano.svg" style="width: 85%" />
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
    <div class="nav-mask" v-if="navActive" @click="navActive = false"></div>
    <div id="main">
      <div class="page-content">
        <div class="content-body">
          <div class="pure-g">
            <div class="pure-u-1 pure-u-md-1-2 pure-u-xl-1-4" v-for="(b, i) in boxes" :key="b">
              <div class="info-box" v-bind:style="{borderColor: colors[i]}">{{b}} some info</div>
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
    // HelloWorld
  },
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
