<template>
  <button
    @click.stop="$emit('click')"
    :disabled="disabled"
    class="bg-primary-400 text-white font-semibold py-1 px-3 rounded-full"
    :class="{
        'hover:bg-primary-600': !disabled,
        'opacity-50': disabled
      }">
    {{label}}
  </button>
</template>

<script>

export default {
  name: "wlan-button",
  props: ['disabled', 'label'],
  data: () => {
    return {
    };
  },
  watch: {},
  mounted: function () {
  },
  methods: {
  },
  components: {},
};
</script>

<style lang="scss" scoped>
</style>
