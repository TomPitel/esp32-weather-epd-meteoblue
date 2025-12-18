#pragma once
#include <time.h>
#include <Arduino.h>

// Converts a string timestamp (YYYY-MM-DD HH:MM or YYYY-MM-DD) to unix time (UTC)
// If only time (HH:MM) is provided, baseDate must be set to a YYYY-MM-DD string for the day.
inline int64_t timestampToUnix(const char* timestamp, const char* baseDate = nullptr) {
    struct tm t = {0};
    if (strlen(timestamp) == 16) { // YYYY-MM-DD HH:MM
        sscanf(timestamp, "%d-%d-%d %d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min);
        t.tm_year -= 1900;
        t.tm_mon -= 1;
    } else if (strlen(timestamp) == 10) { // YYYY-MM-DD
        sscanf(timestamp, "%d-%d-%d", &t.tm_year, &t.tm_mon, &t.tm_mday);
        t.tm_year -= 1900;
        t.tm_mon -= 1;
    } else if (strlen(timestamp) == 5 && baseDate) { // HH:MM with baseDate
        int year, mon, mday;
        sscanf(baseDate, "%d-%d-%d", &year, &mon, &mday);
        t.tm_year = year - 1900;
        t.tm_mon = mon - 1;
        t.tm_mday = mday;
        sscanf(timestamp, "%d:%d", &t.tm_hour, &t.tm_min);
    } else {
        return 0;
    }
    t.tm_isdst = -1;
    return mktime(&t);
}
