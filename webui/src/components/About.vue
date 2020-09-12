<template>
  <div class="pure-g">
    <div class="pure-u-1-1">
      <div class="app-bar-actions app-bar-wrapper">
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
          <a href="https://forum.wlanthermo.de" target="_blank">{{$t('forumUrl')}}</a>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";

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
      ]
    };
  },
  watch: {},
  mounted: function () {
  },
  methods: {
    backToHome: function () {
      EventBus.$emit("back-to-home")
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
  &:hover {
    color: $input_highlight_color;
  }
}

</style>
