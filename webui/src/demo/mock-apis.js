import axios from 'axios'
import { MockData } from './mock-data'

// axios 1.x: rejected request interceptors no longer flow through response error
// interceptors. Use a custom adapter instead to intercept at the transport layer.
axios.defaults.adapter = async (config) => {
  if (process.env.VUE_APP_DEBUG_MOCK_API && config.url !== '/data') {
    console.log(config)
  }
  return {
    data: MockData.mock(config),
    status: 200,
    statusText: 'OK',
    headers: {},
    config,
    request: {}
  }
}
