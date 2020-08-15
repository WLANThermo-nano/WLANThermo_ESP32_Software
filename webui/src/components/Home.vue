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
        "#FFFF00",
        "#FFC002",
        "#00FF00",
        "#FFFFFF",
        "#FF1DC4",
        "#E46C0A",
        "#C3D69B",
        "#0FE6F1",
        "#0000FF",
        "#03A923",
        "#C84B32",
        "#FF9B69",
        "#5082BE",
        "#FFB1D0",
        "#A6EF03",
        "#D42A6B",
        "#FFDA8F",
        "#00B0F0",
        "#948A54"
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
