<template>
  <div>
    <div class="inline-flex cursor-pointer" >
      <div class="flex justify-between items-center p-1 rounded-2xl" @click="$emit('input', !value)">
        <div class="w-12 h-6 flex items-center bg-gray-300 rounded-full p-1 duration-300 ease-in-out " :class="{ 'bg-primary-400': value, 'bg-darkblue-600': !value}">
          <div class="bg-white w-4 h-4 rounded-full shadow-md transform duration-300 ease-in-out" :class="{ 'translate-x-6': value,}"></div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>

export default {
  name: "wlan-toggle-button",
  props: ['value'],
  data: () => {
    return {};
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
