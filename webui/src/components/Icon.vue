<template>
  <div :style="{width: width, height: height, fontSize: fontSize}" @click="$emit('click')">
    <span :class="'icon-' + iconClass"></span>
  </div>
</template>

<script>

export default {
  name: "Icon",
  props: {
    iconClass: {
      type: String
    },
    width: {
      default: '33px'
    },
    height: {
      default: '33px'
    },
    fontSize: {
      default: '1em'
    }
  },
  data: () => {
    return {};
  },
  components: {
  },
};
</script>

<style lang="scss">

</style>
