import Vue from 'vue'
import App from './App.vue'
import axios from 'axios'
import VueAxios from 'vue-axios'
import VueI18n from 'vue-i18n'
import Vuelidate from 'vuelidate'
import en from './i18n/en'
import de from './i18n/de'

if (process.env.VUE_APP_DEMO_MODE) {
  require('./demo/mock-apis')
}

Vue.use(Vuelidate)
Vue.use(VueAxios, axios)
Vue.use(VueI18n)


const i18n = new VueI18n({
  locale: 'en',
  messages: {
    en, de
  },
})

Vue.config.productionTip = false

new Vue({
  i18n,
  render: h => h(App),
}).$mount('#app')
