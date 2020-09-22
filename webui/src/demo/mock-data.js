import { Data } from './mocks/data'

const mock = {
    '/settings': Data.settings,
    '/data': Data.data,
    '/networklist': Data.networklist,
    '/bluetooth': Data.bluetooth,
    '/newtoken': 'random-demo-token' + Math.random()
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
            }
            return mock[config.url]
        }
    }
}