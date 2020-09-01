<template>
  <div class="pure-g">
    <div class="pure-u-1-1">
      <div class="app-bar-actions">
        <div class="button-container" @click="backToHome">
          <span class="icon-arrow_left"></span>
          <span>{{$t('back')}}</span>
        </div>
        <div class="button-container" @click="save">
          <span>{{$t('save')}}</span>
          <span class="icon-arrow_right"></span>
        </div>
      </div>
    </div>
    <div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="name">
        {{ $t('bluetoothTitle') }}
        <span
          @click="showHelpText"
          class="icon-question_sign icon-question"
        ></span>
      </div>
      <div class="config-form">
        <form>
          <div class="form-checkbox">
            <label for="bluetoothActivate" class="pure-checkbox checkbox">
              <input v-model="bluetoothSettings.enabled" type="checkbox" id="bluetoothActivate" />
              {{$t("bluetoothActivate")}}
            </label>
          </div>
          <hr>
          <div class="select-channel-text">
            {{ $t('bluetoothChannels') }}
          </div>
          <div class="devices">
            <div v-for="(device, deviceIndex) in devices" :key="deviceIndex">
              <div class="form-checkbox checkbox-row">
                <label :for="device.id" class="pure-checkbox checkbox block-label">
                  <input v-model="device.checked" type="checkbox" :id="device.id" @change="handleDeviceCheckChange(device.checked, deviceIndex)"/>
                  <span class="device-name">
                    {{device.name}}
                  </span>
                </label>
              </div>
              <div class="channels">
                <div class="form-checkbox checkbox-row" v-for="(channel, channelIndex) in device.channels" :key="channelIndex">
                  <label :for="channel.id" class="pure-checkbox checkbox block-label">
                    <input v-model="channel.checked" type="checkbox" :id="channel.id" @change="handleChannelCheckChange(channel.checked, deviceIndex, channelIndex)" />
                    {{channel.name}}
                  </label>
                </div>
              </div>
            </div>
          </div>
        </form>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";

export default {
  name: "Bluetooth",
  props: {},
  data: () => {
    return {
      bluetoothSettings: {
        enabled: false,
        devices: []
      },
      devices: []
    };
  },
  watch: {},
  mounted: function () {
    EventBus.$emit("loading", true)
    this.axios.get("/bluetooth").then((response) => {
      const data = response.data
      this.bluetoothSettings = {
        enabled: data.enabled,
        devices: data.devices
      }
      this.devices = data.devices.map((device, index) => {
        let mappedDevice = {};
        mappedDevice.name = `${device.name} (${device.address})`
        mappedDevice.id = `d_${index}` // for label
        mappedDevice.checked = device.selected > 0
        mappedDevice.channels = []
        for (let channelIndex = 0; channelIndex < device.count ; channelIndex++) {
          mappedDevice.channels.push({
            name: `${this.$t('channel')} ${channelIndex + 1}`,
            checked: (device.selected & (1 << channelIndex)) > 0, // I don't really understand this condition, it's from the old code
            id: `c_${index}_${channelIndex}` // for label
          })
        }
        return mappedDevice
      })
      EventBus.$emit("loading", false)
    });
  },
  methods: {
    handleChannelCheckChange: function(value, deviceIndex, channelIndex) {
      if (value === true) {
        this.bluetoothSettings.devices[deviceIndex].selected |= (1 << channelIndex)
      } else {
        this.bluetoothSettings.devices[deviceIndex].selected &= ~(1 << channelIndex);
      }
    },
    handleDeviceCheckChange: function(value, deviceIndex) {
      if (value === true) {
        this.bluetoothSettings.devices[deviceIndex].selected = (1 << this.bluetoothSettings.devices[deviceIndex].count) - 1;
        this.devices[deviceIndex].channels.forEach(channel => {
          channel.checked = true
        })
      } else {
        this.bluetoothSettings.devices[deviceIndex].selected = 0
        this.devices[deviceIndex].channels.forEach(channel => {
          channel.checked = false
        })
      }
    },
    backToHome: function () {
      EventBus.$emit("back-to-home")
    },
    showHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_bluetooth_title'),
        content: this.$t('help_bluetooth')
      })
    },
    save: function() {
      const requestObj = Object.assign({}, this.bluetoothSettings)
      EventBus.$emit("loading", true)
      this.axios.post('/setbluetooth', requestObj).then(() => {
        EventBus.$emit("loading", false)
      });
    }
  },
  components: {},
};
</script>

<style lang="scss" scoped>
@import "../assets/colors.scss";

.select-channel-text {
  color: #fff;
  font-size: 1.2em;
  margin: 0.7em 0 0.7em;
}

.channels {
  padding-left: 2em;
}

.checkbox-row {
  margin-top: 0.25rem !important;
  margin-bottom: 0.25rem !important;
}

.block-label {
  cursor: pointer;
  display: block;
  padding-top: 0.5rem;
  padding-bottom: 0.5rem;
  &:hover {
    background-color: $dark;
  }
}
</style>
