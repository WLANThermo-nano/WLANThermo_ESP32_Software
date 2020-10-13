import { Data } from './mocks/data'
import {ChartData} from './mocks/chartData'

const mock = {
    '/settings': Data.settings,
    '/data': Data.data,
    '/networklist': Data.networklist,
    '/bluetooth': Data.bluetooth,
    'https://dev-api.wlanthermo.de/index.php': ChartData
}

export class MockData {
    static mock = (config) => {
        if (config.method === 'get') {
            return mock[config.url]
        } else if (config.method === 'post') {
            if (config.url === '/setchannels') {
                const channelToUpdateIndex = mock['/data'].channel.findIndex(c => c.number === config.data.number)
                mock['/data'].channel[channelToUpdateIndex] = config.data
            } else if (config.url === '/setsystem') {
                mock['/settings'].system = config.data
            } else if (config.url === '/newtoken') {
                mock['/newtoken'] = 'random-demo-token' + Math.floor(Math.random() * Math.floor(10000000000000))
            }
            return mock[config.url]
        }
    }
}