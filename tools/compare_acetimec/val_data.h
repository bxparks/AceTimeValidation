#ifndef COMPARE_ACETIMEC_VAL_DATA_H
#define COMPARE_ACETIMEC_VAL_DATA_H

#include <acetimec.h>
#include "test_data.h"

/**
 * Add a TestItem for one second before a DST transition, and right at the
 * the DST transition. Calculates the active transitions inside the
 * AtcZoneProcessing structure for each each year from `start_year` to
 * `until_year`. Then extracts the transitions from the AtcTransitionStorage
 * structure. Similar to `compare_acetz/zptdgenerator.py`.
 */
void add_transitions(
    struct TestDataEntry *test_entry,
    const char *zone_name,
    struct AtcZoneProcessing *processing,
    const struct AtcZoneInfo *info,
    int16_t start_year,
    int16_t until_year);

/**
 * Add a TestItem for the 1st of each month (using the local time)
 * as a sanity sample, to make sure things are working, even for timezones with
 * no DST transitions.
 */
void add_monthly_samples(
    struct TestDataEntry *test_entry,
    const char *zone_name,
    struct AtcZoneProcessing *processing,
    const struct AtcZoneInfo *zone_info,
    int16_t start_year,
    int16_t until_year);

#endif
