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
        {{ $t('pitTitle') }}
        <span
          @click="showHelpText"
          class="icon-question_sign icon-question"
        ></span>
      </div>
      <div  v-if="!isEditingProfile">
        <div v-for="(pm, index) in $v.pitmaster.pm.$each.$iter" :key="index">
          <div class="form-section-name">
            {{ 'Pitmaster ' + (parseInt(index) + 1) }}
          </div>
          <div class="config-form" >
            <form>
              <div class="form-group">
                <select v-model="pm.typ.$model">
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
              <div class="form-group" v-if="pm.typ.$model !== 'off'">
                <select v-model="pm.pid.$model">
                  <option
                    v-for="(profile, index) in profiles"
                    :key="index"
                    :value="profile.id">
                    {{ profile.name }}
                  </option>
                </select>
                <label class="control-label" for="select">{{$t("pitProfile")}}</label>
                <i class="bar"></i>
                <span @click="editProfile(pm.pid.$model, pm.typ.$model)" class="icon-pencil icon-form"></span>
              </div>
              <!-- channels -->
              <div class="form-group" v-if="pm.typ.$model === 'auto'">
                <select v-model="pm.channel.$model">
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
              <div class="form-group" v-if="pm.typ.$model === 'manual'" :class="{ 'error': pm.value.$invalid}">
                <input type="number" v-model="pm.value.$model" max="100" min="0" required />
                <label class="control-label" for="input">{{$t("pitValue")}}</label>
                <i class="bar"></i>
                <div class="error-prompt" v-if="pm.value.$invalid">
                  {{$t('v_must_between', {min: pm.value.$params.between.min, max: pm.value.$params.between.max})}}
                </div>
              </div>
              <!-- set value -->
              <div class="form-group" v-if="pm.typ.$model === 'auto'" :class="{ 'error': pm.set.$invalid}">
                <input type="number" v-model="pm.set.$model" max="999.9" min="-999.9" step="any" required />
                <label class="control-label" for="input">{{$t("pitTemp")}}</label>
                <i class="bar"></i>
                <div class="error-prompt" v-if="pm.set.$invalid">
                  {{$t('v_must_between', {min: pm.set.$params.between.min, max: pm.set.$params.between.max})}}
                </div>
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
            <!-- 0 => SSR, 1 => FAN  -->
            <template v-if="editingProfile.aktor == 0 || editingProfile.aktor === 1 || editingProfile.aktor == 3">
              <div class="pure-u-1-2 control">
                <div class="form-group" :class="{ 'error': $v.editingProfile.DCmmin.$invalid}">
                  <input type="number" v-model="editingProfile.DCmmin" max="100" min="0" step="any" required />
                  <label class="control-label" for="input">{{$t("pitDCmin")}}</label>
                  <i class="bar"></i>
                  <div class="error-prompt" v-if="$v.editingProfile.DCmmin.$invalid">
                    {{$t('v_must_between', {min: $v.editingProfile.DCmmin.$params.between.min, max: $v.editingProfile.DCmmin.$params.between.max})}}
                  </div>
                </div>
              </div>
              <div class="pure-u-1-2 control">
                <div class="form-group" :class="{ 'error': $v.editingProfile.DCmmax.$invalid}">
                  <input type="number" v-model="editingProfile.DCmmax" max="100" min="0" step="any" required />
                  <label class="control-label" for="input">{{$t("pitDCmax")}}</label>
                  <i class="bar"></i>
                  <div class="error-prompt" v-if="$v.editingProfile.DCmmax.$invalid">
                    {{$t('v_must_between', {min: $v.editingProfile.DCmmax.$params.between.min, max: $v.editingProfile.DCmmax.$params.between.max})}}
                  </div>
                </div>
              </div>
            </template>
            <!-- 2 => SERVO, 3 => DAMPER  -->
            <template v-if="editingProfile.aktor == 2 || editingProfile.aktor == 3">
              <div class="pure-u-1-2 control">
                <div class="form-group" :class="{ 'error': $v.editingProfile.SPmin.$invalid}">
                  <input type="number" v-model="editingProfile.SPmin" max="3000" min="0" required />
                  <label class="control-label" for="input">{{$t("pitSPmin")}}</label>
                  <i class="bar"></i>
                  <div class="error-prompt" v-if="$v.editingProfile.SPmin.$invalid">
                    {{$t('v_must_between', {min: $v.editingProfile.SPmin.$params.between.min, max: $v.editingProfile.SPmin.$params.between.max})}}
                  </div>
                </div>
              </div>
              <div class="pure-u-1-2 control">
                <div class="form-group" :class="{ 'error': $v.editingProfile.SPmax.$invalid}">
                  <input type="number" v-model="editingProfile.SPmax" max="3000" min="0" required />
                  <label class="control-label" for="input">{{$t("pitSPmax")}}</label>
                  <i class="bar"></i>
                  <div class="error-prompt" v-if="$v.editingProfile.SPmax.$invalid">
                    {{$t('v_must_between', {min: $v.editingProfile.SPmax.$params.between.min, max: $v.editingProfile.SPmax.$params.between.max})}}
                  </div>
                </div>
              </div>
            </template>
            <template v-if="editingProfile.aktor == 3">
              <div class="form-group select-form-group">
                <select v-model="editingProfile.link">
                  <option value="0">{{$t("pitlinkdegressiv")}}</option>
                  <option value="1">{{$t("pitlinklinear")}}</option>
                </select>
                <label class="control-label" for="select">{{$t("pitlink")}}</label>
                <i class="bar"></i>
              </div>
            </template>
          </form>
        </div>
        <template v-if="editingPMType !== 'manual'">
          <div class="form-section-name" >
            {{$t("pidTitle")}}
          </div>
          <div class="config-form" >
            <form>
              <div class="form-spacing"></div>
              <div class="pure-u-1-3 control">
                <div class="form-group" :class="{ 'error': $v.editingProfile.Kp.$invalid}">
                  <input type="number" min="0" max="150" step="any" v-model="editingProfile.Kp" required />
                  <label class="control-label" for="input">{{$t("pidKp")}}</label>
                  <i class="bar"></i>
                  <div class="error-prompt" v-if="$v.editingProfile.Kp.$invalid">
                    {{$t('v_must_between', {min: $v.editingProfile.Kp.$params.between.min, max: $v.editingProfile.Kp.$params.between.max})}}
                  </div>
                </div>
              </div>
              <div class="pure-u-1-3 control">
                <div class="form-group" :class="{ 'error': $v.editingProfile.Ki.$invalid}">
                  <input type="number" min="0.0" max="5.0" step="0.01" v-model="editingProfile.Ki" required />
                  <label class="control-label" for="input">{{$t("pidKi")}}</label>
                  <i class="bar"></i>
                  <div class="error-prompt" v-if="$v.editingProfile.Ki.$invalid">
                    {{$t('v_must_between', {min: $v.editingProfile.Ki.$params.between.min, max: $v.editingProfile.Ki.$params.between.max})}}
                  </div>
                </div>
              </div>
              <div class="pure-u-1-3 control">
                <div class="form-group" :class="{ 'error': $v.editingProfile.Kd.$invalid}">
                  <input type="number" min="0" max="800" v-model="editingProfile.Kd" required />
                  <label class="control-label" for="input">{{$t("pidKd")}}</label>
                  <i class="bar"></i>
                  <div class="error-prompt" v-if="$v.editingProfile.Kd.$invalid">
                    {{$t('v_must_between', {min: $v.editingProfile.Kd.$params.between.min, max: $v.editingProfile.Kd.$params.between.max})}}
                  </div>
                </div>
              </div>
              <div class="form-group control-at-top" :class="{ 'error': $v.editingProfile.jp.$invalid}">
                <input type="number" min="0" max="100" v-model="editingProfile.jp" required/>
                <label class="control-label" for="input">{{$t("pidJump")}}</label>
                <i class="bar"></i>
                <div class="error-prompt" v-if="$v.editingProfile.jp.$invalid">
                  {{$t('v_must_between', {min: $v.editingProfile.jp.$params.between.min, max: $v.editingProfile.jp.$params.between.max})}}
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
        </template>
      </div>
    </div>
  </div>
</template>

<script>
import EventBus from "../event-bus";
import { between } from 'vuelidate/lib/validators'

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
      editingPMType: null,
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
  validations: {
    pitmaster: {
      pm: {
        $each: {
          value: {
            between: between(0, 100)
          },
          id: {},
          channel: {},
          pid: {},
          set: {
            between: between(-999.9, 999.9)
          },
          typ: {},
          set_color: {},
          value_color: {},
        },
      },
    },
    editingProfile: {
      jp: {
        between: between(10, 100)
      },
      Kd: {
        between: between(0, 800)
      },
      Ki: {
        between: between(0, 5)
      },
      Kp: {
        between: between(0, 150)
      },
      SPmax: {
        between: between(0, 3000)
      },
      SPmin: {
        between: between(0, 3000)
      },
      DCmmax: {
        between: between(0, 100)
      },
      DCmmin: {
        between: between(0, 100)
      }
    }
  },
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
    editProfile: function(profileId, pmType) {
      this.isEditingProfile = true
      this.editingPMType = pmType
      this.editingProfile = this.profiles.find(p => p.id == profileId)
      this.editingProfileIndex = this.profiles.findIndex(p => p.id == profileId)
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
        if (this.$v.editingProfile.$invalid) {
          return;
        }
        this.profiles[this.editingProfileIndex] = this.editingProfile;
        EventBus.$emit("loading", true)
        this.axios.post('/setpid', this.profiles).then(() => {
          EventBus.$emit("loading", false)
          this.isEditingProfile = false
        }).catch(() => {
          EventBus.$emit("loading", false)
        })
      } else {
        if (this.$v.pitmaster.$invalid) {
          return;
        }
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
  margin-top: 0.3em;
  font-size: 1.1em;
}
.form-spacing {
  margin-bottom: 1.2em;
}
</style>
