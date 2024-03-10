export default class IconsHelper {
    static getWifiIcon(dbm) {
        if (dbm >= '-80') {
            return 'bg-light-wifi-100 dark:bg-wifi-100'
        } else if (dbm >= '-95') {
            return 'bg-light-wifi-50 dark:bg-wifi-50'
        } else if (dbm >= '-105') {
            return 'bg-light-wifi-10 dark:bg-wifi-10'
        }
    }

    static getBatteryIcon(level, isCharging) {
        if (isCharging) {
            if (level >= '95') return 'bg-light-charge-100 dark:bg-charge-100'
            if (level >= '75') return 'bg-light-charge-75 dark:bg-charge-75'
            if (level >= '50') return 'bg-light-charge-50 dark:bg-charge-50'
            if (level >= '25') return 'bg-light-charge-25 dark:bg-charge-25'
            else return 'bg-light-charge-10 dark:bg-charge-10'
        }

        if (level >= '95') return 'bg-light-battery-100 dark:bg-battery-100'
        if (level >= '75') return 'bg-light-battery-75 dark:bg-battery-75'
        if (level >= '50') return 'bg-light-battery-50 dark:bg-battery-50'
        if (level >= '25') return 'bg-light-battery-25 dark:bg-battery-25'
        else return 'bg-light-battery-10 dark:bg-battery-10'
    }
}