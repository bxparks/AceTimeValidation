#ifndef COMPARE_ACETIME_SAMPLING_H
#define COMPARE_ACETIME_SAMPLING_H

#include "test_data.h"

namespace ace_time {
class TimeZone;
}

/**
 * Add a TestItem for one second before a DST transition, and right at the
 * the DST transition.
 */
void addTransitions(
    TestCollection *collection,
    const char *zoneName,
    const ace_time::TimeZone& tz,
    int16_t startYear,
    int16_t untilYear,
    int64_t epochOffset);

/**
 * Add a TestItem for the 1st of each month (using the local time)
 * as a sanity sample, to make sure things are working, even for timezones with
 * no DST transitions.
 */
void addMonthlySamples(
    TestCollection *collection,
    const char *zoneName,
    const ace_time::TimeZone& tz,
    int16_t startYear,
    int16_t untilYear,
    int64_t epochOffset);

#endif
