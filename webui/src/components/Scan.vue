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
            <span
              class="ic_white icon-refresh"
              :class="{ 'icon-rotate-100': refreshing }"
              @click="checkConnection"
            ></span>
          </div>
          <div
            class="scan-device-item"
            v-for="(device, deviceIndex) in displayedDevices"
            :key="deviceIndex"
            @click="deviceSelected(device)"
          >
            <div class="info">
              <div class="body">
                <div class="image" style="width: 30px;">
                  <div class="connection-state"
                       v-if="device.type !== 'demo'"
                       :class="{ connected: device.connected, lower: device.type === 'linkv1' }">
                  </div>
                  <img :src="images[device.type]" alt="">
                </div>
                <div class="name-address">
                  <div class="name">
                    {{ device.name }}
                  </div>
                  <div class="address">
                    {{ device.ip }}
                  </div>
                  <div class="info">
                    {{ device.info }}
                  </div>
                </div>
              </div>
            </div>
          </div>
        </form>
        <div style="color: #fff">
          debug message:
        </div>
        <div style="max-height: 50vh; overflow-y:scroll;color:#fff;border: grey solid 1px; padding: 2px">
          <div v-for="msg in debugMessages" :key="msg">
            {{msg}}
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
import {SPECIAL_URL_FOR_DEMO_API} from '../demo/mock-apis-mobile.models';

const Netmask = require("netmask").Netmask;

const MY_DEVICES_KEY = '_WLANTHERMO_MY_DEVICES';
const DEVICE_SCHEMA_VERSION = 'v1'
const DEVICE_SCHEMA_VERSION_KEY = '_WLANTHERMO_MY_DEVICES_VERSION'
const DEVICE_TYPES = ['nanov1','nanov2','nanov3','miniv1','miniv2','miniv3','linkv1']

function toInfoText(respData) {
  return `${respData.device?.device} || ${respData.device?.sw_version}`
}


function toDeviceType(respData) {
  const type = `${respData.device?.device?.toLocaleLowerCase()}${respData.device?.hw_version.toLocaleLowerCase()}`
  if (DEVICE_TYPES.some(t => t === type)) {
    return type
  }
  return 'demo'
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
    displayedDevices: function() {
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
        demo: require(`@/assets/images/demo.svg`)
      },
      refreshing: false,
      demoDevices: [
        {
          name: 'DEMO',
          ip: '192.168.0.1',
          info: 'Konfiguration',
          type: 'demo'
        }
      ],
      devices: [
      ],
      debugMessages: [],
      requestCompletedCount: 0,
      blockSize: 0,
      requestCancelTokenSource: '',
      showConnectionLost: false,
    };
  },
  watch: {},
  mounted: function () {
    if (this.$route.query.connectionLost) {
      this.showConnectionLost = true
      setTimeout(() => {
        this.showConnectionLost = false
      }, 3000)
    }
    this.getStoredData().then(() => {
      document.addEventListener('deviceready', this.initAndScan.bind(this), false)
    })
  },
  methods: {
    checkScanCompleted: function() {
      if (this.requestCompletedCount >= this.blockSize) {
        this.refreshing = false
      }
    },
    deviceSelected: function(device) {
      if (device.type === 'demo') {
        EventBus.$emit("loading", true)
        console.log(`should be SPECIAL_URL_FOR_DEMO_API ${SPECIAL_URL_FOR_DEMO_API}`)
        this.axios.defaults.baseURL = `http://${SPECIAL_URL_FOR_DEMO_API}`
        EventBus.$emit("loading", false)
        EventBus.$emit('device-selected')
      } else if (device.connected) {
        EventBus.$emit("loading", true)
        setTimeout(() => {
          this.axios.defaults.baseURL = `http://${device.ip}`
          EventBus.$emit("loading", false)
          EventBus.$emit('device-selected')
        })
      }
    },
    scanBySubnet: function() {
      this.refreshing = true
      this.requestCompletedCount = 0
      this.blockSize = 0
      this.requestCancelTokenSource = this.axios.CancelToken.source()

      // eslint-disable-next-line
      networkinterface.getWiFiIPAddress((address) => {
        let maskNodes = address.subnet.match(/(\d+)/g);
        let bitmask = 0;
        for (var i in maskNodes) {
          bitmask += ((maskNodes[i] >>> 0).toString(2).match(/1/g) || [])
            .length;
        }
        const addressWithMask = `${address.ip}/${bitmask}`;

        const block = new Netmask(addressWithMask);

        if (block.size <= 256) {
          block.forEach((addr) => {
            if (addr !== address.ip) {
              this.blockSize++;
              this.axios.get(`http://${addr}/settings`, {
                  cancelToken: this.requestCancelTokenSource.token,
                  headers: {scan: true},
                  // timeout: 5000
                  }).then((resp) => {
                this.requestCompletedCount++;
                if (resp?.data?.system?.host) {
                  const deviceName = resp.data.system.host
                  this.devices.push({
                    ip: addr,
                    name: deviceName,
                  });
                }
                this.checkScanCompleted()
              }).catch(() => {
                this.requestCompletedCount++
                this.checkScanCompleted()
              });
            }
          });
        } else {
          console.log(`subnet is too large to scan`);
        }
      });
    },
    scanByZeroConf: function() {
      // eslint-disable-next-line
      var zeroconf = cordova.plugins.zeroconf;
      zeroconf.registerAddressFamily = "ipv4";
      zeroconf.watchAddressFamily = "ipv4";

      this.addToDebug(`start watching _wlanthermo._tcp.`)
      zeroconf.watch("_wlanthermo._tcp.", "local.", (result) => {
        var action = result.action;
        var service = result.service;
        console.log(`action ${action}`)
        console.log(service)
        this.addToDebug(`action ${action}`)
        if (action == "added") {
          console.log(`service added`)
          const ip = service.ipv4Addresses[0]
          const name = service.txtRecord.device
          this.addToDebug(`action added, ip: ${ip}, name: ${name}`)
          if (ip && name) {
            this.checkAndAddDevice(name, ip)
          }
        } else if (action == "resolved") {
          const ip = service.ipv4Addresses[0]
          const name = service.txtRecord.device
          this.addToDebug(`action resolved, ip: ${ip}, name: ${name}`)
          if (ip && name) {
            this.checkAndAddDevice(name, ip)
          }
          // window.location.href = "device.html?ip=" + ipAddress;
        } else {
          console.log(`service removed`)
          const ip = service.ipv4Addresses[0]
          const name = service.txtRecord.device
          this.devices = this.devices.filter(d => d.ip === ip && d.name === name)
        }
      });
    },
    addToDebug: function(msg) {
      var d = new Date();
      var n = d.toLocaleTimeString();
      this.debugMessages.push(`${n}: ${msg}`)
    },
    checkAndAddDevice: function(name, ip) {
      console.log(`checking device ${name} - ${ip}`)
      this.addToDebug(`calling /settings API of ip: ${ip}, name: ${name}`)
      this.axios.get(`http://${ip}/settings`).then(resp => {
          const data = resp.data
          console.log(`resp data`)
          console.log(resp)
          const sn = data.device?.serial

          const deviceInListAndHasDifferentIp = this.devices.some(d => d.sn === sn && d.ip !== ip)
          const deviceInList = this.devices.some(d => d.sn === sn)
          console.log(`deviceInListAndHasDifferentIp: ${deviceInListAndHasDifferentIp}`)
          console.log(`deviceInList: ${deviceInList}`)

          this.addToDebug(`got device settings device name: ${name}, ip: ${ip}, sn: ${sn}`)
          this.addToDebug(`device name: ${name}, ip: ${ip} already in the list? => ${deviceInList}`)

          if (!deviceInList || deviceInListAndHasDifferentIp) {
            this.addToDebug(`adding device name: ${name}, ip: ${ip} to the list`)
            if (deviceInListAndHasDifferentIp) {
              this.devices = this.devices.filter(d => d.sn !== sn)
            }
            const info = toInfoText(data)
            const type = toDeviceType(data)
            this.devices.push({
              ip: ip,
              name: name,
              sn: sn,
              info: info,
              type: type,
              connected: true
            })
            this.updateStoredData()
          } else {
            this.addToDebug(`device name: ${name}, ip: ${ip} already in the list... skipped`)
          }
        });
    },
    updateStoredData: function() {
      const storage = window.localStorage
      storage.setItem(DEVICE_SCHEMA_VERSION_KEY, DEVICE_SCHEMA_VERSION)
      storage.setItem(MY_DEVICES_KEY, JSON.stringify(this.devices))
    },
    getStoredData: function() {
      return new Promise(resolve => {
        const storage = window.localStorage
        const value = storage.getItem(MY_DEVICES_KEY)
        const version = storage.getItem(DEVICE_SCHEMA_VERSION_KEY)
        this.addToDebug(`get data from local storage`)
        if (value !== null && version === DEVICE_SCHEMA_VERSION) {
          this.devices = JSON.parse(value)
          this.devices = this.devices.map(d => {
            d.connected = false
            return d
          })
          this.checkConnection()
        }
        resolve()
      })
    },
    checkConnection: function() {
      if (this.devices.length === 0) return
      this.refreshing = true
      this.devices.forEach(d => {
        this.axios.get(`http://${d.ip}/settings`).then(resp => {
          const data = resp.data
          d.info = toInfoText(data)
          d.type = toDeviceType(data)
          d.connected = true
          this.refreshing = false
        }).catch(() => {
          d.connected = false
          this.refreshing = false
        });
      })
    },
    initAndScan: function () {
      this.addToDebug(`scan with ZeroConf`)
      this.scanByZeroConf()
    },
  },
  components: {
  },
};
</script>

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
        .address,.info {
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
</style>
