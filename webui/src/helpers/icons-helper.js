export default class IconsHelper {
    static getWifiIcon(dbm) {
        if (dbm >= '-80') {
            return 'Wlan100'
        } else if (dbm >= '-95') {
            return 'Wlan66'
        } else if (dbm >= '-105') {
            return 'Wlan33'
        }
    }
}