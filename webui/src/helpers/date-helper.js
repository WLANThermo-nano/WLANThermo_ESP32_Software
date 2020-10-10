export default class DateHelper {
    static stringTimestampInSecondToDate(value) {
        return new Date(parseInt(value * 1000))
    }
}