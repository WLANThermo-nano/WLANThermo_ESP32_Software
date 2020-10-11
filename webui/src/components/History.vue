<template>
  <div class="pure-g">
    <div class="pure-u-1-1 app-bar-wrapper">
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
        {{ $t('historyTitle') }}
        <span
          @click="showHelpText"
          class="icon-question_sign icon-question"
        ></span>
      </div>
      <template v-if="!viewing">
        <div class="config-form">
          <div class="grill-item" :class="{'expand': (index === expandingGrill)}" v-for="(grill, index) in grillList" :key="index" @click="expandingGrill = index">
            <div class="info">
              <div class="icon">
                <Icon class="ic_white" width="1.5em" height="1.5em" fontSize="1.5em" iconClass="fire" />
              </div>
              <div class="body">
                <div class="title">
                  <div class="name">
                    {{ grill.name }}
                  </div>
                  <div class="date">
                    {{ grill.from }} - {{ grill.to }}
                  </div>
                  <div class="desc">
                    {{ grill.descriptions }}
                  </div>
                </div>
              </div>
            </div>
            <div class="extra-panel mt10" v-if="index === expandingGrill">
              <button class="pure-button mr5 pure-button-error">
                {{ $t('Delete') }}
              </button>
              <button class="pure-button mr5 pure-button-primary">
                {{ $t('View') }}
              </button>
              <button class="pure-button pure-button-primary">
                {{ $t('Edit') }}
              </button>
            </div>
          </div>
        </div>
      </template>
      <template v-else>
        <div class="details">
          <h3 class="grill-name">
            grill name 1
          </h3>
          <h4 class="grill-desc">
            10.10.2020 09:00 - 10.10.2020 14:00
          </h4>
          <h4 class="grill-desc">
            Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since
          </h4>
          <LineChart :settings="settings" :channels="channels"/>
        </div>
      </template>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";
import Icon from './Icon.vue'
import LineChart from './LineChart'

export default {
  name: "History",
  props: {
    channels: {
      type: Array
    },
    settings: {
      type: Object
    }
  },
  data: () => {
    return {
      expandingGrill: -1,
      viewing: false,
      grillList: [
        { 
          name: 'grill name 1',
          descriptions: `Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since`,
          from: '10.10.2020 09:00:05',
          to: '10.10.2020 14:00:05'
        },
        { 
          name: 'grill name 2',
          descriptions: `Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since`,
          from: '10.10.2020 09:00:05',
          to: '10.10.2020 14:00:05'
        },
        { 
          name: 'grill name 3',
          descriptions: `Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since`,
          from: '10.10.2020 09:00:05',
          to: '10.10.2020 14:00:05'
        }
      ]
    }
  },
  watch: {},
  mounted: function () {},
  methods: {
    backToHome: function () {
      EventBus.$emit("back-to-home")
    },
    showHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_bluetooth_title'),
        content: this.$t('help_bluetooth')
      })
    },
    save: function() {}
  },
  components: { Icon, LineChart },
};
</script>

<style lang="scss" scoped>
@import "../assets/colors.scss";

.grill-item {
  display: flex;
  flex-direction: column;
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
      margin-top: 1.2em;
    }
    .body {
      flex: 1 1 auto;
      display: flex;
      justify-content: space-between;
      line-height: 3.1em;
      padding-bottom: 0.3em;
      .title {
        display: flex;
        flex-direction: column;
        flex: 1 1 auto;
        overflow: hidden;
        color: #fff;
        .name {
          line-height: 1.2em;
          font-size: 1.2em;
        }
        .date {
          margin-top: 0.3em;
          margin-left: 0.3em;
          line-height: 0.8em;
          font-size: 0.8em;
        }
        .desc {
          margin-top: 0.3em;
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

.extra-panel {
  margin-left: 3.2em;
}


.details {
  .grill-name {
    margin-left: 0.3em;
    color: #fff;
  }
  .grill-desc {
    margin-left: 0.3em;
    color: #fff;
  }
}
</style>
