export default class DateHelper {
    static stringTimestampInSecondToDate(value) {
        return new Date(parseInt(value * 1000))
    }

    static toTimestamp(value) {
        return parseInt(value * 1000)
    }
}