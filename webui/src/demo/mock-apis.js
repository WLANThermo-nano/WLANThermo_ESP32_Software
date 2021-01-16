import axios from 'axios'
import MockAdapter from 'axios-mock-adapter'
import { MockData } from './mock-data'

if (process.env.VUE_APP_PRODUCT_NAME === 'demo') {

  var mock = new MockAdapter(axios)

  mock.onAny().reply(function (config) {
    
    if (process.env.VUE_APP_DEBUG_MOCK_API) {
      if (config.url !== '/data') {
        console.log(config)
      }
    }

    return Promise.resolve(Object.assign({
      data: {},
      status: 200,
      statusText: 'OK',
      headers: {},
      config,
      isMock: true
    }, {data: MockData.mock(config)}))
  });
}
