import Vue from 'vue'
import VueRouter from 'vue-router'
import Home from '../components/Home.vue'
import Wlan from '../components/Wlan.vue'
import System from '../components/System.vue'
import Bluetooth from '../components/Bluetooth.vue'
import Pitmaster from '../components/Pitmaster'
import About from '../components/About'
import IoT from '../components/IoT'
import PushNotification from '../components/PushNotification'
const Scan = require(process.env.VUE_APP_SCAN_COMPONENT).default;

Vue.use(VueRouter)

const routes = [
  {
    path: '/',
    name: 'home',
    component: Home
  },
  {
    path: '/wlan',
    name: 'wlan',
    component: Wlan
  },
  {
    path: '/system',
    name: 'system',
    component: System
  },
  {
    path: '/bluetooth',
    name: 'bluetooth',
    component: Bluetooth
  },
  {
    path: '/pitmaster',
    name: 'pitmaster',
    component: Pitmaster
  },
  {
    path: '/about',
    name: 'about',
    component: About
  },
  {
    path: '/iot',
    name: 'iot',
    component: IoT
  },
  {
    path: '/notification',
    name: 'notification',
    component: PushNotification
  },
  {
    path: '/scan',
    name: 'scan',
    component: Scan
  },
]

const router = new VueRouter({
  mode: 'history',
  base: process.env.BASE_URL,
  routes
})

export default router
