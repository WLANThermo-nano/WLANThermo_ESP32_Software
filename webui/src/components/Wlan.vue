<template>
  <div class="pure-g">
    <div class="pure-u-1-1 app-bar-wrapper">
      <div class="app-bar-actions">
        <div class="button-container" @click="backToHome">
          <span class="icon-arrow_left"></span>
          <span>{{$t('back')}}</span>
        </div>
      </div>
    </div>
    <div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="name">
        {{ $t('wlanTitle') }}
        <span
          @click="showHelpText"
          class="icon-question_sign icon-question"
        ></span>
      </div>
      <div class="config-form">
        <div class="form-checkbox">
          <label for="activateWlan" class="pure-checkbox checkbox">
            <input v-model="activateWlanChecked" type="checkbox" id="activateWlan" @input="handleWlanUncheck"/> {{$t("wlanActivate")}}
          </label>
          <div class="wifi-item current mt10" v-if="currentNetwork.connect">
            <div class="info">
              <div class="icon">
                <Icon class="ic_white" width="1.5em" height="1.5em" fontSize="1.5em" :iconClass="currentNetwork.wifiIcon" />
              </div>
              <div class="body">
                <div class="name-address">
                  <div class="name">
                    {{ currentNetwork.SSID }}
                  </div>
                  <div class="address">
                    {{currentNetwork.BSSID}}
                  </div>
                </div>
                <Icon v-if="currentNetwork.enc" class="lock ic_white" width="1em" height="1em" iconClass="lock" />
              </div>
            </div>
          </div>
          <div class="available-networks">
            <div class="text">
              {{ $t('availableNetworks') }}
            </div>
            <span class="ic_white icon-refresh" :class="{'icon-rotate-100': scanning}" @click="networkscan"></span>
          </div>
          <div class="wifi-item other" :class="{'expand': (index === expandingWifi)}" v-for="(wifi, index) in wifiList" :key="index" @click="selectWifi(index)">
            <div class="info">
              <div class="icon">
                <Icon class="ic_white" width="1.5em" height="1.5em" fontSize="1.5em" :iconClass="wifi.wifiIcon" />
              </div>
              <div class="body">
                <div class="name-address">
                  <div class="name">
                    {{ wifi.SSID }}
                  </div>
                  <div class="address">
                    {{wifi.BSSID}}
                  </div>
                </div>
                <Icon v-if="wifi.enc" class="lock ic_white" width="1em" height="1em" iconClass="lock" />
              </div>
            </div>
            <div class="password-panel" v-if="index === expandingWifi">
              <div class="form-group" v-if="wifi.enc">
                <input type="password" maxlength="63" required v-model="password" />
                <label class="control-label" for="input">{{$t("wlanPwd")}}</label>
                <i class="bar"></i>
              </div>
              <button class="pure-button mr5" @click.stop="cancel">
                {{ $t('cancel') }}
              </button>
              <button class="pure-button pure-button-primary" :disabled="password.length === 0 && wifi.enc" @click.stop="save">
                {{ $t('save') }}
              </button>
            </div>
          </div>
          <hr>
          <button class="pure-button pure-button-primary" @click="deleteStoredWifi">
            {{ $t('wlanClear') }}
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";
import Icon from './Icon.vue'
import IconHelper from '../helpers/icons-helper'

export default {
  name: "Wlan",
  data: () => {
    return {
      activateWlanChecked: true,
      currentNetwork: {
        connect: false,
        SSID: '',
        BSSID: '',
        IP: '',
        mask: '',
        gate: '',
        wifiIcon: '',
        enc: false,
        RSSI: null,
      },
      wifiList: [],
      scanning: false,
      expandingWifi: -1,
      selectedWifiSSID: '',
      password: '',
    };
  },
  methods: {
    showHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_wlan_title'),
        content: this.$t('help_wlan'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_ESP32_Software/wiki/WLAN',
        linkText: this.$t('help_wlan_link')
      })
    },
    handleWlanUncheck: function () {
      if (confirm(this.$t('stopWifiPrompt')) === true) {
        this.axios.post('/stopwifi').then((resp) => {
          if (resp.data == true) {
            self.location.href = 'about:blank'
          }
        });
      } else {
        setTimeout(() => {
          this.activateWlanChecked = true
        })
      }
    },
    selectWifi: function(index) {
      this.expandingWifi = index
      this.selectedWifiSSID = this.wifiList[index].SSID
      this.password = ''
    },
    networkscan: function() {
      this.scanning = true
      this.axios.post('/networkscan').then(() => {
        setTimeout(() => {
          this.getNetworklist()
        }, 3000)
      })
    },
    save: function() {
      this.axios.post('/setnetwork', {
        password: this.password,
        ssid: this.selectedWifiSSID
      }).then(() => {
        this.expandingWifi = -1
        this.networkscan()
      })
    },
    cancel: function() {
      this.expandingWifi = -1
    },
    deleteStoredWifi: function() {
      if (confirm(this.$t('clearWifiPrompt')) === true) {
        this.axios.post('/clearwifi').then((resp) => {
          if (resp.data == true) {
            self.location.href = 'about:blank'
          }
        });
      }
    },
    getNetworklist: function() {
      this.scanning = false
      this.axios.post('/networklist').then((resp) => {
        const data = resp.data
        this.currentNetwork = {
          connect: data.Connect,
          SSID: data.SSID,
          BSSID: data.BSSID,
          IP: data.IP,
          mask: data.Mask,
          gate: data.Gate,
          RSSI: data.RSSI,
          enc: data.Enc,
          wifiIcon: IconHelper.getWifiIcon(data.RSSI)
        }
        this.wifiList = data.Scan.map(wifi => {
          return {
            BSSID: wifi.BSSID,
            enc: wifi.Enc,
            RSSI: wifi.RSSI,
            SSID: wifi.SSID,
            wifiIcon: IconHelper.getWifiIcon(wifi.RSSI)
          }
        }).sort((a, b) => {
          return b.RSSI - a.RSSI
        }).filter(wifi => wifi.SSID !== this.currentNetwork.SSID)
      });
    },
    backToHome: function () {
      EventBus.$emit("back-to-home")
    },
  },
  components: {
    Icon
  },
  mounted: function () {
    this.getNetworklist()
    this.networkscan()
  },
};
</script>

<style lang="scss" scoped>
@import "../assets/colors.scss";

.wifi-item {
  display: flex;
  flex-direction: column;
  padding: 0.6em;
  border-radius: 0.4em;
  &.current {
    @apply bg-primary-400;
  }
  &.other {
    cursor: pointer;
    &:hover {
      background-color: $dark;
    }
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
      line-height: 2.1em;
      .name-address {
        display: flex;
        flex-direction: column;
        flex: 1 1 auto;
        color: #fff;
        .name {
          line-height: 1.2em;
          font-size: 1.0em;
        }
        .address {
          margin-left: 0.3em;
          line-height: 0.8em;
          font-size: 0.8em;
        }
      }
      .lock {
        flex: 0 0 1.5em;
      }
    }
  }
}

.available-networks {
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
</style>
