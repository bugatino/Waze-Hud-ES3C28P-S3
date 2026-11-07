#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Vietnamese lunar calendar. Lunar dates are always evaluated in UTC+7,
// independently from the phone timezone.
typedef struct {
    int day;
    int month;
    int year;
    int leap_month;
    bool valid;
} lunar_date_t;

bool lunar_from_unix(int64_t unix_seconds, lunar_date_t *out);

#ifdef __cplusplus
}
#endif

