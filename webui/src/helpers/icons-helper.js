export default class IconsHelper {
    static getWifiIcon(dbm) {
        if (dbm >= '-80') {
            return 'wifi100'
        } else if (dbm >= '-95') {
            return 'wifi50'
        } else if (dbm >= '-105') {
            return 'wifi10'
        }
    }

    static getBatteryIcon(level, isCharging) {
        if (isCharging) {
            if (level >= '95') return 'charge100'
            if (level >= '75') return 'charge75'
            if (level >= '50') return 'charge50'
            if (level >= '25') return 'charge25'
            else 'charge10'
        }

        if (level >= '95') return 'battery100'
        if (level >= '75') return 'battery75'
        if (level >= '50') return 'battery50'
        if (level >= '25') return 'battery25'
        else 'battery10'
    }
}