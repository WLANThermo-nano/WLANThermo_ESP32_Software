<template>
  <div class="pure-g m-4">
    <div class="page-title-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="name">
        <span class="back-button cursor-pointer" @click="backToHome">
          <span class="icon-arrow_left"></span>
        </span>
        {{ $t('wlanTitle') }}
        <span
            @click="showHelpText"
            class="icon-question_sign icon-question"
          >
        </span>
      </div>
    </div>
    <div class="pure-u-1-1 mt-4 mb-1 tracking-normal">
      <div class="flex ml-1 my-4">
        <div class="text-white align-middle flex-grow inline-block">
          <span class="align-middle font-semibold">{{ $t('wlanActivate') }}</span>
        </div>
        <div class="self-end">
          <wlan-toggle-button v-model="activateWlanChecked" @input="handleWlanUncheck"></wlan-toggle-button>
        </div>
      </div>
    </div>
    <div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1">
      <div class="wifi-item" v-if="currentNetwork.connect">
        <div class="info">
          <div class="body">
            <div>
              <wlan-checkbox 
                :value="true"
                :readonly="true"
                label="">
              </wlan-checkbox>
            </div>
            <div class="name-address">
              <div class="name">
                {{ currentNetwork.SSID }}
              </div>
              <div class="address">
                {{currentNetwork.BSSID}}
              </div>
            </div>
            <div class="icons">
              <img 
                class="inline-block cursor-pointer text-center mr-3 w-5.5 h-5 mt-0.5"
                :src="currentNetwork.wifiIcon"/>
              <span class="lock-icon wlan-icons-lock"></span>
            </div>
          </div>
        </div>
      </div>
    </div>
    <div class="pure-u-1-1 mt-12 mb-1 tracking-normal">
      <div class="flex ml-1 h-6">
        <div class="text-white align-middle flex-grow">
          <span class="align-middle font-semibold">{{ $t('availableNetworks') }}</span>
        </div>
        <div class="self-end">
          <span 
            class="wlan-icons-refresh text-grey-800 text-base inline-block text-center mr-3 cursor-pointer"
            :class="{'icon-rotate-100': scanning}"
            @click="networkscan">
          </span>
        </div>
      </div>
    </div>
    <div class="config-form-container pure-u-1-1 pure-u-md-1-1 pure-u-lg-1-1 mt-3 cursor-pointer"
      v-for="(wifi, index) in wifiList" :key="index" 
      @click="selectWifi(index)">
      <div class="wifi-item" :class="{'expand': (index === expandingWifi)}">
        <div class="info">
          <div class="body">
            <div class="border-l-4 border-primary-400 h-8 mt-4 mr-1 ml-4"></div>
            <div class="name-address">
              <div class="name">
                {{ wifi.SSID }}
              </div>
              <div class="address">
                {{wifi.BSSID}}
              </div>
            </div>
            <div class="icons">
              <img 
                class="inline-block cursor-pointer text-center mr-3 w-5.5 h-5 mt-0.5"
                :src="wifi.wifiIcon"/>
              <span v-if="wifi.enc" class="lock-icon wlan-icons-lock"></span>
            </div>
          </div>
        </div>
        <transition name="fadeHeight">
          <div class="password-panel px-4 mt-3" v-if="index === expandingWifi">
            <div class="form-group mb-2" v-if="wifi.enc">
              <input type="password" maxlength="63" required v-model="password" />
              <label class="control-label" for="input">{{$t("wlanPwd")}}</label>
              <i class="bar"></i>
            </div>
            <div class="grid">
              <div class="justify-self-end">
                <wlan-button 
                  :label="$t('save')" 
                  :disabled="password.length === 0 && wifi.enc"
                  @click="save">
                </wlan-button>
              </div>
            </div>
          </div>
        </transition>
      </div>
    </div>
    <div class="pure-u-1-1">
      <div class="grid mt-2">
        <div class="justify-self-end">
          <wlan-button 
            :label="$t('wlanClear')" 
            @click="deleteStoredWifi">
          </wlan-button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";
import IconHelper from '../helpers/icons-helper'
import WlanToggleButton from './shared/ToggleButton.vue'
import WlanCheckbox from './shared/Checkbox.vue'
import WlanButton from './shared/Button.vue'

export default {
  name: "Wlan",
  data: () => {
    return {
      wifiIcons: {
        wifi10: require(`@/assets/icons/svg/wifi-10.svg`),
        wifi50: require(`@/assets/icons/svg/wifi-50.svg`),
        wifi100: require(`@/assets/icons/svg/wifi-100.svg`),
      },
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
      alert('save triggered')
      this.axios.post('/setnetwork', {
        password: this.password,
        ssid: this.selectedWifiSSID
      }).then(() => {
        this.expandingWifi = -1
        this.networkscan()
      })
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
          wifiIcon: this.wifiIcons[IconHelper.getWifiIcon(data.RSSI)]
        }
        this.wifiList = data.Scan.map(wifi => {
          return {
            BSSID: wifi.BSSID,
            enc: wifi.Enc,
            RSSI: wifi.RSSI,
            SSID: wifi.SSID,
            wifiIcon: this.wifiIcons[IconHelper.getWifiIcon(wifi.RSSI)]
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
    WlanToggleButton, WlanCheckbox, WlanButton
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
          @apply leading-6 text-xl mt-3;
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
