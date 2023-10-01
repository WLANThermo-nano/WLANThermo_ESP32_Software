<template>
  <div class="pure-g">
    <div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="name">
        {{ $t("scanTitle") }}
      </div>
      <div class="config-form">
        <form>
          <div class="select-channel-text">
            <span class="text">{{ $t("scanDevices") }}</span>
            <span class="ic_white icon-refresh" :class="{ 'icon-rotate-100': refreshing }"
              @click="checkConnection"></span>
          </div>
          <swipe-list :items="displayedDevices" :item-disabled="disableSwipe" item-key="name">
            <template v-slot="{ item }">
              <div @click="deviceSelected(item)" class="scan-device-item">
                <div class="info">
                  <div class="body">
                    <div class="image" style="width: 30px;">
                      <div class="connection-state" v-if="item.type !== 'demo'"
                        :class="{ connected: item.connected, lower: item.type === 'linkv1' }">
                      </div>
                      <img :src="images[item.type]" alt="">
                    </div>
                    <div class="name-address">
                      <div class="name">
                        {{ item.name }}
                      </div>
                      <div class="address">
                        {{ item.ip }}
                      </div>
                      <div class="info">
                        {{ item.info }}
                      </div>
                    </div>
                  </div>
                </div>
              </div>
            </template>
            <template v-slot:right="{ item }">
              <div class="swipeout-action red" @click="removeDevice(item)">
                <span class="icon-trash icon"></span>
              </div>
            </template>
          </swipe-list>
        </form>

        <div style="color: #fff">
          debug message:
        </div>
        <div style="max-height: 50vh; max-width: 90vw; overflow-y:scroll;color:#fff;border: grey solid 1px; padding: 2px">
          <div v-for="msg in debugMessages" :key="msg">
            {{ msg }}
          </div>
        </div>

      </div>
    </div>
    <div class="connection-lost-toast" v-if="showConnectionLost">
      Lost Connection...
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";
import { SPECIAL_URL_FOR_DEMO_API } from '../demo/mock-apis-mobile.models';
import { SwipeList } from 'vue-swipe-actions';


const Netmask = require("netmask").Netmask;

const MY_DEVICES_KEY = '_WLANTHERMO_MY_DEVICES';
const DEVICE_SCHEMA_VERSION = 'v1'
const DEVICE_SCHEMA_VERSION_KEY = '_WLANTHERMO_MY_DEVICES_VERSION'
const DEVICE_TYPES = ['nanov1', 'nanov2', 'nanov3', 'miniv1', 'miniv2', 'miniv3', 'linkv1', 'boneV1']

function toDeviceType(respData) {
  const type = `${respData.device?.device?.toLocaleLowerCase()}${respData.device?.hw_version.toLocaleLowerCase()}`
  if (DEVICE_TYPES.some(t => t === type)) {
    return type
  }
  return 'demo'
}

function toIncompatible(respData) {
  // versions < v1.2.0 are incompatible
  return Boolean(parseInt(respData.device.sw_version.replaceAll('v', '').replaceAll('.', '')) < 120)
}

// structure of a device: v1
// {
//   name: 'NANO-98f4ab7570d8',
//   ip: '192.168.178.134',
//   info: 'NANO V3 || v.1.0.6',
//   type: 'nanov2',
//   sn: '1591215343',
//   connected: true,
// },

export default {
  name: "Scan",
  props: {},
  computed: {
    displayedDevices: function () {
      return [...this.devices, ...this.demoDevices]
    }
  },
  data: () => {
    return {
      images: {
        nanov1: require(`@/assets/images/nanov1.svg`),
        nanov2: require(`@/assets/images/nanov1.svg`),
        nanov3: require(`@/assets/images/nanov3.svg`),
        miniv1: require(`@/assets/images/miniv2.svg`),
        miniv2: require(`@/assets/images/miniv2.svg`),
        miniv3: require(`@/assets/images/miniv3.svg`),
        linkv1: require(`@/assets/images/linkv1.svg`),
        bonev1: require(`@/assets/images/bonev1.svg`),
        demo: require(`@/assets/images/demo.svg`)
      },
      ipAddress: '',
      refreshing: false,
      demoDevices: [
        {
          name: 'DEMO',
          ip: '192.168.0.1',
          info: 'Konfiguration',
          type: 'demo'
        }
      ],
      devices: [],
      debugMessages: [],
      requestCompletedCount: 0,
      blockSize: 0,
      requestCancelTokenSource: '',
      showConnectionLost: false,
      disableSwipe: (device) => device.type === 'demo'
    };
  },
  watch: {},
  mounted: async function () {
    if (this.$route.query.connectionLost) {
      this.showConnectionLost = true
      setTimeout(() => {
        this.showConnectionLost = false
      }, 3000)
    }

    this.initZeroConfigScanListener()
    this.initAndScan();
  },
  methods: {
    initZeroConfigScanListener: function() {
      window.addEventListener("serviceResolved", (event) => {
        /**
         * Example of detail
         * {
         *  name: "LINK-98f4ab756fcc",
         *  ip: "192.168.x.x"
         * }
         */
        const detail = event.detail
        this.addToDebug(`receive service: ${JSON.stringify(detail)}`)
        this.checkAndAddDevice(detail.name, detail.ip)
      });

      this.addToDebug("registered listener serviceResolved")
    },
    checkScanCompleted: function () {
      if (this.requestCompletedCount >= this.blockSize) {
        this.refreshing = false
      }
    },
    removeDevice: function (device) {
      this.devices = this.devices.filter(d => d.sn !== device.sn)
      this.updateStoredData()
    },
    deviceSelected: function (device) {
      if (device.type === 'demo') {
        EventBus.$emit("loading", true)
        console.log(`should be SPECIAL_URL_FOR_DEMO_API ${SPECIAL_URL_FOR_DEMO_API}`)
        this.axios.defaults.baseURL = `http://${SPECIAL_URL_FOR_DEMO_API}`
        EventBus.$emit("loading", false)
        EventBus.$emit('device-selected')
      } else if (device.incompatible) {
        window.flutter_inappwebview
            .callHandler('openExternalLink', `http://${device.ip}`).then(() => {})
      } else if (device.connected) {
        EventBus.$emit("loading", true)
        setTimeout(() => {
          this.axios.defaults.baseURL = `http://${device.ip}`
          EventBus.$emit("loading", false)
          EventBus.$emit('device-selected')
        })
      }
    },
    scanBySubnet: function () {
      this.refreshing = true
      this.requestCompletedCount = 0
      this.blockSize = 0
      this.requestCancelTokenSource = this.axios.CancelToken.source()

      const address = this.ipAddress
      const addressWithMask = `${address}/${24}`;

      const block = new Netmask(addressWithMask);

      this.addToDebug(`there are ${block.size} address in the subnet`)

      if (block.size <= 256) {
        block.forEach((addr) => {
          if (addr !== address) {
            this.blockSize++;

            this.checkAndAddDevice('', addr);
          }
        });
      } else {
        this.addToDebug(`subnet is too large to scan`)
        console.log(`subnet is too large to scan`);
      }
    },
    addToDebug: function (msg) {
      var d = new Date();
      var n = d.toLocaleTimeString();
      this.debugMessages.push(`${n}: ${msg}`)
    },
    checkAndAddDevice: function (name, ip) {
      this.axios.get(`http://${ip}/settings`).then(resp => {
        const data = resp.data
        const sn = data.device?.serial

        const deviceInListAndHasDifferentIp = this.devices.some(d => d.sn === sn && d.ip !== ip)
        const deviceInList = this.devices.some(d => d.sn === sn)

        if (!deviceInList || deviceInListAndHasDifferentIp) {
          this.addToDebug(`adding device name: ${name}, ip: ${ip} to the list`)
          if (deviceInListAndHasDifferentIp) {
            this.devices = this.devices.filter(d => d.sn !== sn)
          }
          const info = this.toInfoText(data)
          const type = toDeviceType(data)
          const incompatible = toIncompatible(data)
          this.devices.push({
            ip: ip,
            name: name,
            sn: sn,
            info: info,
            type: type,
            connected: true,
            incompatible: incompatible
          })
          this.updateStoredData()
        } else {
          this.addToDebug(`device name: ${name}, ip: ${ip} already in the list... skipped`)
        }
        this.requestCompletedCount++
        this.checkScanCompleted()
      }).catch(() => {
        this.requestCompletedCount++
        this.checkScanCompleted()
      });
    },
    updateStoredData: function () {
      window.flutter_inappwebview
        .callHandler('saveData', DEVICE_SCHEMA_VERSION_KEY, DEVICE_SCHEMA_VERSION)
        .then()
      window.flutter_inappwebview
        .callHandler('saveData', MY_DEVICES_KEY, JSON.stringify(this.devices))
        .then()
    },
    getStoredData: async function () {
      const deviceListData = await window.flutter_inappwebview
        .callHandler('getData', MY_DEVICES_KEY)
      const versionData = await window.flutter_inappwebview
        .callHandler('getData', DEVICE_SCHEMA_VERSION_KEY)

      this.addToDebug(`device list from storage ${deviceListData}`)
      return new Promise((resolve) => {
        const devicesAsString = deviceListData.value ?? []
        const version = versionData.value

        this.addToDebug(`get data from local storage: ${devicesAsString}`)
        if (devicesAsString !== null && version === DEVICE_SCHEMA_VERSION) {
          this.devices = JSON.parse(devicesAsString)
          this.devices = this.devices.map(d => {
            d.connected = false
            return d
          })
          this.checkConnection()
        }
        resolve()
      })
    },
    checkConnection: function () {
      if (this.devices.length === 0) return
      this.refreshing = true
      this.devices.forEach(d => {
        this.axios.get(`http://${d.ip}/settings`).then(resp => {
          const data = resp.data
          d.info = this.toInfoText(data)
          d.type = toDeviceType(data)
          d.connected = true
          this.refreshing = false
        }).catch(() => {
          d.connected = false
          this.refreshing = false
        });
      })
    },
    initAndScan: async function () {
      // trigger scan
      setTimeout(async () => {
        await this.getStoredData()
        window.flutter_inappwebview.callHandler('scanByZeroConfig').then(()=>{})
      }, 1000)
    },
    toInfoText: function (respData) {
      var info = `${respData.device?.device} ${respData.device?.hw_version} || ${respData.device?.sw_version}`
      if (true === toIncompatible(respData)) {
        info += ` || ${this.$t("scanDeviceIncompatible")}`
      }
      return info
    },
  },
  components: {
    SwipeList,
  },
};
</script>

<style src='vue-swipe-actions/dist/vue-swipe-actions.css'>
/* global styles */
</style> 
<style lang="scss" scoped>
@import "../assets/colors.scss";

.select-channel-text {
  display: flex;

  .text {
    flex: 1 1 auto;
    color: #fff;
    font-size: 1.2em;
    margin: 0.7em 0 0.7em;
  }

  .icon-refresh {
    margin-top: 15px;
    margin-right: 18px;
    cursor: pointer;
    width: 16px;
    height: 16px;
  }
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

.scan-device-item {
  display: flex;
  flex-direction: column;
  cursor: pointer;
  padding: 0.6em;

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
      margin-top: 0.3em;
    }

    .body {
      flex: 1 1 auto;
      display: flex;
      justify-content: space-between;

      .image {
        flex: 0 0 3em;
        margin-right: 0.5em;
        position: relative;

        .connection-state {
          position: absolute;
          height: 0.8em;
          width: 0.8em;
          top: -0.1em;
          right: -0.1em;
          background-color: red;
          border-radius: 0.8em;

          &.lower {
            top: 0.25em;
          }

          &.connected {
            background-color: lightgreen;
          }
        }

        img {
          height: 100%;
          width: 100%;
        }
      }

      .name-address {
        display: flex;
        flex-direction: column;
        flex: 1 1 auto;
        color: #fff;

        .name {
          margin-top: 0;
          font-size: 1em;

          .space {
            margin-right: 0.5em;
          }
        }

        .address,
        .info {
          font-weight: 300;
          font-size: 0.75em;
          margin-left: 0.3em;
        }
      }
    }
  }
}

.connection-lost-toast {
  position: absolute;
  bottom: 10vh;
  letter-spacing: 0;
  padding: 10px;
  border-radius: 15px;
  background-color: $medium;
  color: #fff;
  margin: 0 auto;
  left: 0;
  right: 0;
  margin-left: auto;
  margin-right: auto;
  width: 200px;
  text-align: center;
}

.swipeout-action {
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  left: 0;
  width: 6em;

  &.red {
    background-color: $error_color;
  }

  .icon {
    font-size: 2em;
    color: #fff;
  }
}
</style>
