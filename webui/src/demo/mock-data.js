import { Data } from './mocks/data'

const mock = {
    '/settings': Data.settings,
    '/data': Data.data,
    '/networklist': Data.networklist,
    '/getbluetooth': Data.getbluetooth,
    '/getpush': Data.getpush
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
            } else if (config.url === '/setpush') {
                mock['/getpush'] = config.data
            }
            return mock[config.url]
        }
    }
}