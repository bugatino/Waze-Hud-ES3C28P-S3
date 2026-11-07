#include "system/lunar_calendar.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static int64_t days_from_civil(int year, int month, int day) {
    year -= month <= 2;
    int era = (year >= 0 ? year : year - 399) / 400;
    unsigned year_of_era = (unsigned)(year - era * 400);
    unsigned day_of_year = (unsigned)((153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1);
    unsigned day_of_era = year_of_era * 365 + year_of_era / 4 - year_of_era / 100 + day_of_year;
    return (int64_t)era * 146097 + (int64_t)day_of_era - 719468;
}

static int64_t vietnam_midnight(int year, int month, int day) {
    return days_from_civil(year, month, day) * 86400 - 7 * 3600;
}

static void expect_lunar(int year, int month, int day,
                         int lunar_day, int lunar_month, int lunar_year) {
    lunar_date_t actual = {0};
    assert(lunar_from_unix(vietnam_midnight(year, month, day), &actual));
    assert(actual.day == lunar_day);
    assert(actual.month == lunar_month);
    assert(actual.year == lunar_year);
}

int main(void) {
    expect_lunar(2024, 2, 10, 1, 1, 2024);
    expect_lunar(2025, 1, 29, 1, 1, 2025);
    expect_lunar(2026, 2, 17, 1, 1, 2026);
    expect_lunar(2024, 9, 17, 15, 8, 2024);
    expect_lunar(2025, 1, 28, 29, 12, 2024);
    puts("lunar calendar fixtures passed");
    return 0;
}
