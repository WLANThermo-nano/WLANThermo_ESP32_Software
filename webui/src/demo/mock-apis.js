import axios from 'axios'
import { MockData } from './mock-data'

const EXCEPTION_LIST = [
  // 'index.php'
]

const getMockError = config => {
  const mockError = new Error()
  mockError.config = config
  return Promise.reject(mockError)
}

// Add a request interceptor
axios.interceptors.request.use(config => {
  if (EXCEPTION_LIST.some(url => config.url.includes(url))) {
    return config
  }
  return getMockError(config)
}, error => Promise.reject(error))

// Add a response interceptor
axios.interceptors.response.use(response => response, error => {
  return getMockResponse(error)
})

const getMockResponse = mockError => {
  const {config} = mockError

  if (process.env.VUE_APP_DEBUG_MOCK_API) {
    if (config.url !== '/data') {
      console.log(config)
    }
  }

  // Handle mocked success
  return Promise.resolve(Object.assign({
    data: {},
    status: 200,
    statusText: 'OK',
    headers: {},
    config,
    isMock: true
  }, {data: MockData.mock(config)}))
}