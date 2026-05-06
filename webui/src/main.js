import { createApp } from 'vue'
import App from './App.vue'
import axios from 'axios'
import VueAxios from 'vue-axios'
import { createI18n } from 'vue-i18n'
import en from './i18n/en'
import de from './i18n/de'
import router from './router'

require(process.env.VUE_APP_DEMO_FILE_PATH)

axios.defaults.timeout = 5000

const i18n = createI18n({
  locale: 'en',
  legacy: true,
  messages: { en, de },
})

const app = createApp(App)
app.use(VueAxios, axios)
app.use(i18n)
app.use(router)
app.mount('#app')
