<template>
  <div class="pure-g m-4">
    <div class="page-title-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="name">
        <span class="back-button cursor-pointer" @click="backToHome">
          <span class="icon-arrow_left"></span>
        </span>
        {{ $t('bluetoothTitle') }}
        <span
            @click="showHelpText"
            class="icon-question_sign icon-question"
          >
        </span>
      </div>
    </div>
    <div class="pure-u-1-1 mt-4 mb-1 tracking-normal">
      <div class="flex ml-1 my-4">
        <div class="text-white align-middle flex-grow">
          <span class="align-middle font-semibold">{{ $t('bluetoothActivate') }}</span>
        </div>
        <div class="self-end">
          <wlan-toggle-button v-model="bluetoothSettings.enabled"></wlan-toggle-button>
        </div>
      </div>
    </div>
    <div class="pure-u-1-1 mt-2 mb-1 tracking-normal">
      <div class="flex ml-1 h-6">
        <div class="text-white align-middle flex-grow">
          <span class="align-middle font-semibold">{{ $t('bluetoothChannels') }}</span>
        </div>
        <div class="self-end">
          <span 
            class="wlan-icons-refresh text-grey-800 text-base inline-block text-center mr-3 cursor-pointer"
            :class="{'icon-rotate-100': refreshing}"
            @click="refreshBluetooth">
          </span>
        </div>
      </div>
    </div>
    <div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1 mt-3 cursor-pointer"
      v-for="(device, deviceIndex) in devices" :key="deviceIndex">
      <div class="bluetooth-item" :class="{'expand': (deviceIndex === expandingDevice)}" >
        <div class="info" @click="selectDevice(deviceIndex)">
          <div class="body">
            <div>
              <img 
                class="inline-block text-center w-5 h-4.5 mr-5 mt-3"
                :src="bluetoothIcon"/>
            </div>
            <div class="name-address">
              <div class="name">
                {{ device.name }} 
                <span class="mr-1"></span>
                <span class="font-normal">({{device.selectedChannels}}/{{device.total}})</span>
              </div>
              <div class="address">
                {{ device.address }}
              </div>
            </div>
          </div>
        </div>
        <transition name="fadeHeight">
          <div v-if="deviceIndex === expandingDevice">
            <div v-for="(channel, channelIndex) in device.channels" :key="channelIndex">
              <wlan-checkbox 
                  v-model="channel.checked"
                  @change="handleChannelCheckChange(channel.checked, deviceIndex, channelIndex)"
                  :label="channel.name">
              </wlan-checkbox>
            </div>
          </div>
        </transition>
      </div>
    </div>
    <div class="pure-u-1-1">
      <div class="grid mt-2">
        <div class="justify-self-end">
          <wlan-button 
            :label="$t('save')" 
            @click="save">
          </wlan-button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";
import WlanToggleButton from './shared/ToggleButton.vue'
import WlanCheckbox from './shared/Checkbox.vue'
import WlanButton from './shared/Button.vue'

export default {
  name: "Bluetooth",
  props: {},
  data: () => {
    return {
      bluetoothIcon: require(`@/assets/icons/svg/bluetooth-fill.svg`),
      expandingDevice: -1,
      refreshing: false,
      bluetoothSettings: {
        enabled: false,
        devices: []
      },
      devices: []
    };
  },
  watch: {},
  mounted: function () {
    this.refreshBluetooth()
  },
  methods: {
    refreshBluetooth: function() {
      this.refreshing = true;
      this.axios.get("/getbluetooth").then((response) => {
        console.log(response)
        const data = response.data
        this.bluetoothSettings = {
          enabled: data.enabled,
          devices: data.devices
        }
        this.devices = data.devices.map((device, index) => {
          let mappedDevice = {};
          mappedDevice.name = device.name
          mappedDevice.address = device.address
          mappedDevice.id = `d_${index}` // for label
          mappedDevice.checked = device.selected > 0
          mappedDevice.channels = []
          mappedDevice.total = device.count
          for (let channelIndex = 0; channelIndex < device.count ; channelIndex++) {
            mappedDevice.channels.push({
              name: `${this.$t('channel')} ${channelIndex + 1}`,
              checked: this.channelIsChecked(device, channelIndex),
              id: `c_${index}_${channelIndex}` // for label
            })
          }

          mappedDevice.selectedChannels = mappedDevice.channels.filter(c => c.checked).length
          
          return mappedDevice
        })
        // API returns too fast, adding some delay makes UX a bit better, otherwise the button looks doesn't work at all
        setTimeout(() => {
          this.refreshing = false
        }, 200)
      });
    },
    channelIsChecked: function(device, channelIndex) {
      return (device.selected & (1 << channelIndex)) > 0 // I don't really understand this condition, it's from the old code
    },
    selectDevice: function(index) {
      if (this.expandingDevice === index) {
        this.expandingDevice = -1
      } else {
        this.expandingDevice = index
      }
    },
    handleChannelCheckChange: function(value, deviceIndex, channelIndex) {
      if (value === true) {
        this.bluetoothSettings.devices[deviceIndex].selected |= (1 << channelIndex)
      } else {
        this.bluetoothSettings.devices[deviceIndex].selected &= ~(1 << channelIndex);
      }

      this.devices[deviceIndex].selectedChannels = this.devices[deviceIndex].channels.filter(c => c.checked).length
    },
    backToHome: function () {
      EventBus.$emit("back-to-home")
    },
    showHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_bluetooth_title'),
        content: this.$t('help_bluetooth'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/wiki/Bluetooth-Schnittstelle',
        linkText: this.$t('help_bluetooth_link')
      })
    },
    save: function() {
      const requestObj = Object.assign({}, this.bluetoothSettings)
      EventBus.$emit("loading", true)
      this.axios.post('/setbluetooth', requestObj).then(() => {
        EventBus.$emit("loading", false)
        this.backToHome()
      }).catch(() => {
        EventBus.$emit("loading", false)
      })
    }
  },
  components: {
    WlanCheckbox,
    WlanToggleButton,
    WlanButton
  },
};
</script>

<style lang="scss" scoped>
@import "../assets/colors.scss";

.bluetooth-item {
  display: flex;
  flex-direction: column;
  @apply p-2 pl-4 rounded-sm;
  .info {
    display: flex;
    .icon {
      flex: 0 0 3em;
      margin-top: 0.3em;
    }
    .body {
      flex: 1 1 auto;
      display: flex;
      justify-content: space-between;
      line-height: 2.1em;
      .name-address {
        display: flex;
        flex-direction: column;
        flex: 1 1 auto;
        color: #fff;
        .name {
          @apply leading-6 text-xl;
        }
        .address {
          @apply leading-4 text-sm text-grey-800 font-semibold;
          margin-left: 0.3em;
        }
      }
      .icons {
        @apply flex items-center;
        .lock-icon {
          @apply text-grey-800 text-base inline-block text-center mr-3;
        }
      }
    }
  }
}
</style>
