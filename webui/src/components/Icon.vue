<template>
  <div :style="styleObject" @click="$emit('click')">
    <span :class="'icon-' + iconClass"></span>
  </div>
</template>

<script>

export default {
  name: "Icon",
  props: {
    iconClass: {
      type: String
    }
  },
  data: () => {
    return {
      styleObject: {
        width: '45px',
        height: '45px'
      }
    };
  },
  components: {
  },
};
</script>

<style lang="scss">

</style>
