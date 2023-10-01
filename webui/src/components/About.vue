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
        <div v-for="member in members" :key="member.name" class="member">✰ {{member.name}}</div>
      </div>
      <div class="name">
        {{ $t('forumTitle') }}
      </div>
      <div class="config-form">
        <div class="link">
          <a @click="gotoForum">{{$t('forumUrl')}}</a>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";

const forumUrl = 'https://forum.wlanthermo.de'

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
      isMobile: false
    };
  },
  watch: {},
  mounted: function () {
    this.isMobile = process.env.VUE_APP_PRODUCT_NAME === 'mobile'
  },
  methods: {
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
    color: $input_highlight_color;
  }
}

</style>
