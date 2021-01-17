import axios from 'axios'
import { MockData } from './mock-data'
import EventBus from "../event-bus";
import { SPECIAL_URL_FOR_DEMO_API } from './mock-apis-mobile.models';

const getMockError = config => {
  const mockError = new Error()
  mockError.config = config
  return Promise.reject(mockError)
}

// Add a request interceptor
axios.interceptors.request.use(config => {
  console.log('request interceptors: ')
  console.log(config)
  if (config.url.includes(SPECIAL_URL_FOR_DEMO_API)) {
    console.log('using fake api')
    return getMockError(config)
  }
  console.log('return request config')
  return config;
}, error => Promise.reject(error))

// Add a response interceptor
axios.interceptors.response.use(response => response, error => {
  console.log('response interceptors: ')
  console.log(error.config)
  if (error.config.baseURL.includes(SPECIAL_URL_FOR_DEMO_API)) {
    console.log('using fake response')
    return getMockResponse(error)
  }
  if (!error?.config?.headers?.scan) {
    EventBus.$emit('api-error')
  }
  console.log('normal error')
  return error
})

const getMockResponse = mockError => {
  const {config} = mockError

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