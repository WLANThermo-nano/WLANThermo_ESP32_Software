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
            if (level >= '75') return 'chargeHigh'
        }

        // default
        return 'chargeHigh'
    }
}