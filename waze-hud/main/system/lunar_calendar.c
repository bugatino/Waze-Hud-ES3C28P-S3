#include "lunar_calendar.h"

/* Year table ported from the MIT-licensed npm package `lunar` v2.0.0 by
 * 6tail. The conversion below is a small C implementation of that public
 * encoding. Supported civil dates: 1900-01-01 through 2099-12-31. */
#define LUNAR_YEAR_MIN 1890
#define CIVIL_YEAR_MIN 1900
#define CIVIL_YEAR_MAX 2099

static const uint32_t YEAR_INFO[] = {
    1750378,339370,6020516,405156,314698,4934293,371351,279894,2804405,346837,8124114,419666,
    331429,4961866,394827,297627,3880278,370026,281433,1759058,345938,6019877,412453,313931,
    4936875,377517,288107,1764201,355753,7093650,429714,331045,4954701,395862,303798,3872181,
    370388,282281,1769106,347794,6016294,402731,313943,4936374,379738,296660,2821833,354121,
    7075475,420499,329003,5991003,387757,304490,3881813,371620,281417,1759891,346773,7058733,
    402742,314029,4945322,378290,290213,2825546,363850,8120981,412311,329046,5999285,387797,
    304850,3878565,372389,280138,2796695,338587,7066970,402794,314217,4945746,387922,289573,
    3864139,354891,8123563,410285,320877,5999465,396713,306578,3882277,372005,10246733,428630,
    336566,6006197,403157,315049,4947602,388754,298278,2812502,346711,8123606,418650,321237,
    4953801,395081,304787,3872043,361771,272987,1758554,337258,7060309,412580,322377,4938387,
    379541,288045,3861165,346805,9172394,419282,331173,6004042,396618,306325,3880238,361814,
    273077,1758642,345810,6008485,403237,312907,4934807,371883,288090,2812630,355177,11269970,
    420690,330533,6003275,387659,296107,4916571,361901,273258,1760082,347538,7068965,404773,
    313941,4936877,378038,279989,2813354,356041,8142482,421522,331046,5999190,387671,296150,
    3868373,362325,280393,2797203,337555,7058731,402731,305755,4937050,378218,289637,3864394,
    355146,8125077,420501,320813,5991085,387765,304554,3869605,363941,281930,2808981,339094,
    7059790,402774,314037,4937138,378578,290469,3870282,345675,8113303,410795,320859,5991126,
    387946,304978,3880741,363301,273035,1750171,337067,
};

typedef struct {
    int start_month;
    int start_day;
    bool has_leap;
    int leap_index;
    uint32_t bits;
} year_info_t;

static year_info_t info_of(int year) {
    uint32_t e = YEAR_INFO[year - LUNAR_YEAR_MIN];
    year_info_t info;
    info.start_month = ((e >> 18) & 1) + 1;
    info.start_day = (e >> 13) & 31;
    info.has_leap = (e & (1u << 19)) != 0;
    info.leap_index = info.has_leap ? (int)((e >> 20) & 15) : -1;
    info.bits = e & ((1u << 13) - 1u);
    return info;
}

static int segment_days(const year_info_t *info, int segment) {
    return (info->bits & (1u << segment)) ? 30 : 29;
}

// Howard Hinnant's public-domain civil date algorithms.
static int64_t days_from_civil(int y, int m, int d) {
    y -= m <= 2;
    int era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = (unsigned)(y - era * 400);
    unsigned doy = (unsigned)((153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1);
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return (int64_t)era * 146097 + (int64_t)doe - 719468;
}

static void civil_from_days(int64_t z, int *y, int *m, int *d) {
    z += 719468;
    int64_t era = (z >= 0 ? z : z - 146096) / 146097;
    unsigned doe = (unsigned)(z - era * 146097);
    unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int yy = (int)yoe + (int)era * 400;
    unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    unsigned mp = (5 * doy + 2) / 153;
    *d = (int)(doy - (153 * mp + 2) / 5 + 1);
    *m = (int)(mp + (mp < 10 ? 3 : -9));
    *y = yy + (*m <= 2);
}

bool lunar_from_unix(int64_t unix_seconds, lunar_date_t *out) {
    if (!out) return false;
    int64_t adjusted = unix_seconds + 7 * 3600;
    int64_t vn_days = adjusted / 86400;
    if (adjusted % 86400 != 0 && adjusted < 0) --vn_days;
    int year, month, day;
    civil_from_days(vn_days, &year, &month, &day);
    if (year < CIVIL_YEAR_MIN || year > CIVIL_YEAR_MAX) {
        out->valid = false;
        return false;
    }

    int lunar_year = year;
    year_info_t info = info_of(lunar_year);
    int64_t new_year = days_from_civil(lunar_year, info.start_month, info.start_day);
    if (vn_days < new_year) {
        --lunar_year;
        info = info_of(lunar_year);
        new_year = days_from_civil(lunar_year, info.start_month, info.start_day);
    }

    int64_t diff = vn_days - new_year;
    int segments = info.has_leap ? 13 : 12;
    int64_t accumulated = 0;
    int index = 0;
    int month_length = 0;
    while (index < segments) {
        month_length = segment_days(&info, index);
        accumulated += month_length;
        if (diff < accumulated) break;
        ++index;
    }
    if (index >= segments) {
        out->valid = false;
        return false;
    }

    out->day = (int)(month_length - (accumulated - diff) + 1);
    out->year = lunar_year;
    out->leap_month = 0;
    if (info.has_leap) {
        if (index == info.leap_index + 1) {
            out->leap_month = info.leap_index + 1;
            out->month = info.leap_index + 1;
        } else if (index > info.leap_index) {
            out->month = index;
        } else {
            out->month = index + 1;
        }
    } else {
        out->month = index + 1;
    }
    out->valid = true;
    return true;
}
