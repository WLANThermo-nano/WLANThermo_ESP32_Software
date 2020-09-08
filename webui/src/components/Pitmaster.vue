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
        {{ $t('pitTitle') }}
        <span
          @click="showHelpText"
          class="icon-question_sign icon-question"
        ></span>
      </div>
      <div  v-if="!isEditingProfile">
        <div v-for="(pm, index) in pitmaster.pm" :key="index">
          <div class="form-section-name">
            {{ 'Pitmaster ' + (index + 1) }}
          </div>
          <div class="config-form" >
            <form>
              <div class="form-group">
                <select v-model="pm.typ">
                  <option
                    v-for="type in pitmaster.type"
                    :key="type"
                    :value="type">
                    {{type}}
                  </option>
                </select>
                <label class="control-label" for="select">{{$t("pitPitmaster")}}</label>
                <i class="bar"></i>
              </div>
              <!-- profiles -->
              <div class="form-group" v-if="pm.typ !== 'off'">
                <select v-model="pm.pid">
                  <option
                    v-for="(profile, index) in profiles"
                    :key="index"
                    :value="profile.id">
                    {{ profile.name }}
                  </option>
                </select>
                <label class="control-label" for="select">{{$t("pitProfile")}}</label>
                <i class="bar"></i>
                <span @click="editProfile(index)" class="icon-pencil icon-form"></span>
              </div>
              <!-- channels -->
              <div class="form-group" v-if="pm.typ === 'auto'">
                <select v-model="pm.channel">
                  <option
                    v-for="(channel, index) in channels"
                    :key="index"
                    :value="index + 1">
                    #{{index + 1}} - {{ $t('channel') }} {{ index + 1 }}
                  </option>
                </select>
                <label class="control-label" for="select">{{$t("pitChannel")}}</label>
                <i class="bar"></i>
              </div>
              <!-- pitmaster value -->
              <div class="form-group" v-if="pm.typ === 'manual'">
                <input type="number" v-model="pm.value" max="100" min="0" required />
                <label class="control-label" for="input">{{$t("pitValue")}}</label>
                <i class="bar"></i>
              </div>
              <!-- set value -->
              <div class="form-group" v-if="pm.typ === 'auto'">
                <input type="number" v-model="pm.set" max="100" min="0" required />
                <label class="control-label" for="input">{{$t("pitTemp")}}</label>
                <i class="bar"></i>
              </div>
            </form>
          </div>
        </div>
      </div>
      <!-- profile form -->
      <div v-else>
        <!-- general -->
        <div class="form-section-name">
          {{$t("pitNameTitle")}}
        </div>
        <div class="config-form" >
          <form>
              <div class="form-group">
                <input type="text" v-model="editingProfile.name" required />
                <label class="control-label" for="input">{{$t("pitProfileName")}}</label>
                <i class="bar"></i>
              </div>
          </form>
        </div>
        <div class="form-section-name">
          {{$t("pitAktorTitle")}}
        </div>
        <div class="config-form" >
          <form>
            <div class="form-group">
              <select v-model="editingProfile.aktor">
                <option
                  v-for="(act, index) in actuators"
                  :key="index"
                  :value="index">
                  {{ act }}
                </option>
              </select>
              <label class="control-label" for="select">{{$t("pitAktor")}}</label>
              <i class="bar"></i>
            </div>
            <div class="pure-u-1-2">
              <div class="form-group">
                <input type="number" v-model="editingProfile.DCmmin" max="100" min="0" required />
                <label class="control-label" for="input">{{$t("pitDCmin")}}</label>
                <i class="bar"></i>
              </div>
            </div>
            <div class="pure-u-1-2">
              <div class="form-group">
                <input type="number" v-model="editingProfile.DCmmax" max="100" min="0" required />
                <label class="control-label" for="input">{{$t("pitDCmax")}}</label>
                <i class="bar"></i>
              </div>
            </div>
          </form>
        </div>
        <div class="form-section-name">
          {{$t("pidTitle")}}
        </div>
        <div class="config-form" >
          <form>
            <div class="form-group">
              <div class="pure-u-1-3">
                <div class="form-group">
                  <input type="number" v-model="editingProfile.Kp" required />
                  <label class="control-label" for="input">{{$t("pidKp")}}</label>
                  <i class="bar"></i>
                </div>
              </div>
              <div class="pure-u-1-3">
                <div class="form-group">
                  <input type="number" step="0.1" v-model="editingProfile.Ki" required />
                  <label class="control-label" for="input">{{$t("pidKi")}}</label>
                  <i class="bar"></i>
                </div>
              </div>
              <div class="pure-u-1-3">
                <div class="form-group">
                  <input type="number" v-model="editingProfile.Kd" required />
                  <label class="control-label" for="input">{{$t("pidKd")}}</label>
                  <i class="bar"></i>
                </div>
              </div>
              <div class="form-group">
                <input type="number" v-model="editingProfile.jp" min="10" max="100" required />
                <label class="control-label" for="input">{{$t("pidJump")}}</label>
                <i class="bar"></i>
              </div>
            </div>
          </form>
        </div>
        <div class="form-section-name">
          {{$t("pitAdvancesTitle")}}
        </div>
        <div class="config-form" >
          <form>
            <div class="form-checkbox">
              <label for="lid" class="pure-checkbox checkbox">
                <input v-model="editingProfile.opl" :true-value="1" :false-value="0" type="checkbox" id="lid" />
                {{$t("pitLid")}}
              </label>
            </div>
          </form>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";

export default {
  name: "Pitmaster",
  props: {},
  data: () => {
    return {
      pitmaster: {
        pm: [],
        type: []
      },
      channels: [],
      profiles: [],
      actuators: [],
      isEditingProfile: false,
      editingProfileIndex: -1,
      editingProfile: {
        DCmmax: null,
        DCmmin: null,
        Kd: null,
        Ki: null,
        Kp: null,
        SPmax: null,
        SPmin: null,
        aktor: null,
        id: null,
        jp: null,
        link: null,
        name: '',
        opl: null,
        tune: null,
      },
    };
  },
  watch: {},
  mounted: function () {
    EventBus.$emit("loading", true)
    Promise.all([
      this.axios.get("/data"),
      this.axios.get("/settings")
    ]).then(([dataResponse, settingsResponse])=> {
        const data = dataResponse.data
        this.pitmaster.pm = data.pitmaster.pm
        this.pitmaster.type = data.pitmaster.type
        this.channels = data.channel.filter(c => c.typ != 16 && c.typ != 17)

        const settings = settingsResponse.data
        this.profiles = settings.pid
        this.actuators = settings.aktor

        EventBus.$emit("loading", false)
      })
  },
  methods: {
    editProfile: function(index) {
      this.isEditingProfile = true
      this.editingProfile = this.profiles[index]
      this.editingProfileIndex = index
    },
    backToHome: function () {
      if (this.isEditingProfile) {
        this.isEditingProfile = false
      } else {
        EventBus.$emit("back-to-home")
      }
    },
    showHelpText: function () {
      EventBus.$emit('show-help-dialog', {
        title: this.$t('help_pitmaster_title'),
        content: this.$t('help_pitmaster'),
        wikiLink: 'https://github.com/WLANThermo-nano/WLANThermo_nano_Software/wiki/Pitmaster',
        linkText: this.$t('help_pitmaster_link')
      })
    },
    save: function() {
      if (this.isEditingProfile) {
        this.profiles[this.editingProfileIndex] = this.editingProfile;
        EventBus.$emit("loading", true)
        this.axios.post('/setpid', this.profiles).then(() => {
          EventBus.$emit("loading", false)
          this.isEditingProfile = false
        }).catch(() => {
          EventBus.$emit("loading", false)
        })
      } else {
        EventBus.$emit("loading", true)
        this.axios.post('/setpitmaster', this.pitmaster.pm).then(() => {
          EventBus.$emit("loading", false)
          EventBus.$emit("back-to-home")
        }).catch(() => {
          EventBus.$emit("loading", false)
        })
      }
    }
  },
  components: {},
};
</script>

<style lang="scss" scoped>
.form-section-name {
  color: #fff;
  padding: 0.3em;
  font-size: 1.1em;
}
</style>
