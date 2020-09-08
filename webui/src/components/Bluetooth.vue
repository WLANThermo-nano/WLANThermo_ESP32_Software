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
          <div class="bluetooth-item" :class="{'expand': (deviceIndex === expandingDevice)}" v-for="(device, deviceIndex) in devices" :key="deviceIndex">
            <div class="info" @click="selectDevice(deviceIndex)">
              <div class="icon">
                <Icon class="ic_white" width="1.5em" height="1.5em" fontSize="1.5em" iconClass="bluetooth_1" />
              </div>
              <div class="body">
                <div class="name-address">
                  <div class="name">
                    {{ device.name }} <span class="space"></span> ({{device.selectedChannels}}/{{device.total}})
                  </div>
                  <div class="address">
                    {{ device.address }}
                  </div>
                </div>
              </div>
            </div>
            <div class="details-panel" v-if="deviceIndex === expandingDevice">
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
import Icon from "./Icon";

export default {
  name: "Bluetooth",
  props: {},
  data: () => {
    return {
      expandingDevice: -1,
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
      EventBus.$emit("loading", false)
    });
  },
  methods: {
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
  components: {
    Icon
  },
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
  padding-left: 3em;
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

.bluetooth-item {
  display: flex;
  flex-direction: column;
  cursor: pointer;
  padding: 0.6em;
  border-radius: 0.4em;
  &:hover {
    background-color: $dark;
  }
  &.expand {
    background-color: $dark;
  }
  .info {
    display: flex;
    .icon {
      flex: 0 0 3em;
    }
    .body {
      flex: 1 1 auto;
      display: flex;
      justify-content: space-between;
      .name-address {
        display: flex;
        flex-direction: column;
        flex: 1 1 auto;
        color: #fff;
        .name {
          font-size: 1.0em;
          .space {
            margin-right: 0.5em;
          }
        }
        .address {
          font-size: 0.8em;
        }
      }
      .lock {
        flex: 0 0 1.5em;
      }
    }
  }
}
</style>
