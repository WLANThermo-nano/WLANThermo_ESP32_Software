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
              @click="initAndScan"
            ></span>
          </div>
          <div
            class="scan-device-item"
            v-for="(device, deviceIndex) in devices"
            :key="deviceIndex"
            @click="deviceSelected(device)"
          >
            <div class="info">
              <div class="body">
                <div class="name-address">
                  <div class="name">
                    {{ device.name }}
                  </div>
                  <div class="address">
                    {{ device.ip }}
                  </div>
                </div>
              </div>
            </div>
          </div>
        </form>
      </div>
    </div>
    <div class="connection-lost-toast" v-if="showConnectionLost">
      Lost Connection...
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";
const Netmask = require("netmask").Netmask;

export default {
  name: "Scan",
  props: {},
  data: () => {
    return {
      refreshing: false,
      devices: [],
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
    document.addEventListener('deviceready', this.initAndScan.bind(this), false);
  },
  methods: {
    checkScanCompleted: function() {
      if (this.requestCompletedCount >= this.blockSize) {
        this.refreshing = false
      }
    },
    deviceSelected: function(device) {
      this.requestCancelTokenSource.cancel('')
      EventBus.$emit("loading", true)
      setTimeout(() => {
        this.axios.defaults.baseURL = `http://${device.ip}`
        EventBus.$emit("loading", false)
        EventBus.$emit('device-selected')
      })
    },
    initAndScan: function () {
      if (this.refreshing) {
        return;
      }
      this.devices = []
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
                  this.deviceName = deviceName
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
      // var zeroconf = cordova.plugins.zeroconf;
      // zeroconf.registerAddressFamily = "ipv4";
      // zeroconf.watchAddressFamily = "ipv4";

      // zeroconf.watch("_wlanthermo._tcp.", "local.", (result) => {
      //   var action = result.action;
      //   var service = result.service;
      //   if (action == "added") {
      //     console.log("service added", service);
      //   } else if (action == "resolved") {
      //     console.log("service resolved", service);
      //     console.log("device name: " + service.txtRecord.device);
      //     console.log("device ip: " + service.ipv4Addresses[0]);
      //     // ipAddress = service.ipv4Addresses[0];
      //     this.deviceName = service.txtRecord.device
      //     // window.location.href = "device.html?ip=" + ipAddress;
      //   } else {
      //     console.log("service removed", service);
      //   }
      // });
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
      .name-address {
        display: flex;
        flex-direction: column;
        flex: 1 1 auto;
        color: #fff;
        .name {
          font-size: 1em;
          .space {
            margin-right: 0.5em;
          }
        }
        .address {
          font-size: 0.8em;
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
