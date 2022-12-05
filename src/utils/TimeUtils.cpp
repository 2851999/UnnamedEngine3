#include "TimeUtils.h"

/*****************************************************************************
 * utils_time
 *****************************************************************************/

std::string utils_time::getTimeAsString() {
    time_t t = time(NULL);
    struct tm timeInfo;
    localtime_s(&timeInfo, &t);

    std::string hour = utils_string::str(timeInfo.tm_hour);
    std::string minute = utils_string::str(timeInfo.tm_min);
    std::string second = utils_string::str(timeInfo.tm_sec);

    if (hour.length() == 1)
        hour = "0" + hour;
    if (minute.length() == 1)
        minute = "0" + minute;
    if (second.length() == 1)
        second = "0" + second;

    return hour + ":" + minute + ":" + second;
}