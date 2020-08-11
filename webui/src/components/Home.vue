<template>
 <div class="pure-g">
    <div class="pure-u-1 pure-u-md-1-2 pure-u-xl-1-4" v-for="(b, i) in boxes" :key="b">
      <div class="info-box" v-bind:style="{borderColor: colors[i]}">{{b}} some info</div>
    </div>
  </div>
</template>

<script>
// import HelloWorld from './components/HelloWorld.vue'

export default {
  name: "Home",
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
@import "../assets/colors.scss";

.info-box {
  margin: 5px;
  padding: 10px;
  height: 100px;
  color: #fff;
  background-color: $medium_dark;
  border-left: 8px solid;
}


</style>
