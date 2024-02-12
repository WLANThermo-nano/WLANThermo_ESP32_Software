<template>
  <div 
    class="form-checkbox" 
    :class="{ 'cursor-pointer': !readonly }"
    @click="$emit('input', !value)">
    <img
      :src="value ? checkboxBlueFill : checkboxBlue"/>
    <span class="label">{{$t(label)}}</span>
  </div>
</template>

<script>

export default {
  name: "wlan-checkbox",
  props: ['value', 'label', 'readonly'],
  data: () => {
    return {
      checkboxBlueFill: require(`@/assets/icons/svg/checkbox-blue-fill.svg`),
      checkboxBlue: require(`@/assets/icons/svg/checkbox-blue.svg`),
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
