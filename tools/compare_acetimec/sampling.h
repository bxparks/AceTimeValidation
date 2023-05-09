#ifndef COMPARE_ACETIMEC_SAMPLING_H
#define COMPARE_ACETIMEC_SAMPLING_H

#include <acetimec.h>
#include "test_data.h"

/**
 * Add a TestItem for one second before a DST transition, and right at the
 * the DST transition. Calculates the active transitions inside the
 * AtcZoneProcessor structure for each each year from `start_year` to
 * `until_year`. Then extracts the transitions from the AtcTransitionStorage
 * structure. Similar to `compare_acetz/zptdgenerator.py`.
 */
void add_transitions(
    TestCollection *collection,
    const char *zone_name,
    const AtcTimeZone *tz,
    int16_t start_year,
    int16_t until_year,
    int sampling_hours);

/**
 * Add a TestItem for the 1st of each month (using the local time)
 * as a sanity sample, to make sure things are working, even for timezones with
 * no DST transitions.
 */
void add_monthly_samples(
    TestCollection *collection,
    const char *zone_name,
    const AtcTimeZone *tz,
    int16_t start_year,
    int16_t until_year);

#endif
