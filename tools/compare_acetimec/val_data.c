#include <stdint.h>
#include <string.h> // strncpy()
#include <stdio.h> // printf()
#include <acetimec.h>
#include "val_data.h"

static void create_test_item_from_epoch_seconds(
    struct TestItem *ti,
    struct AtcZoneProcessing *processing,
    const struct AtcZoneInfo *zone_info,
    atc_time_t epoch_seconds,
    char type)
{
  struct AtcZonedDateTime zdt;
  atc_zoned_date_time_from_epoch_seconds(
      processing,
      zone_info,
      epoch_seconds,
      &zdt);
  struct AtcZonedExtra zet;
  atc_zoned_extra_from_epoch_seconds(
      processing,
      zone_info,
      epoch_seconds,
      &zet);

  ti->epoch_seconds = epoch_seconds;
  ti->year = zdt.year;
  ti->month = zdt.month;
  ti->day = zdt.day;
  ti->hour = zdt.hour;
  ti->minute = zdt.minute;
  ti->second = zdt.second;
  ti->type = type;

  strncpy(ti->abbrev, zet.abbrev, kAtcAbbrevSize);
  ti->abbrev[kAtcAbbrevSize - 1] = '\0';
  ti->utc_offset = zet.utc_offset_minutes;
  ti->dst_offset = zet.dst_offset_minutes;
}

static void add_test_item_from_epoch_seconds(
    struct TestDataEntry *test_entry,
    const char *zone_name,
    struct AtcZoneProcessing *processing,
    const struct AtcZoneInfo *zone_info,
    atc_time_t epoch_seconds,
    char type)
{
  (void) zone_name;
  struct TestItem *ti = test_data_entry_next_item(test_entry);
  create_test_item_from_epoch_seconds(
      ti, processing, zone_info, epoch_seconds, type);
}

void add_transitions(
    struct TestDataEntry *test_entry,
    const char *zone_name,
    struct AtcZoneProcessing *processing,
    const struct AtcZoneInfo *zone_info,
    int16_t start_year,
    int16_t until_year)
{
  strncpy(test_entry->zone_name, zone_name, ZONE_NAME_SIZE);
  test_entry->zone_name[ZONE_NAME_SIZE - 1] = '\0';

  for (int16_t year = start_year; year < until_year; ++year) {
    atc_processing_init_for_year(processing, zone_info, year);
    struct AtcTransitionStorage *ts = &processing->transition_storage;
    struct AtcTransition **begin =
        atc_transition_storage_get_active_pool_begin(ts);
    struct AtcTransition **end =
        atc_transition_storage_get_active_pool_end(ts);
    for (struct AtcTransition **t = begin; t != end; ++t) {
      // Skip if start year of transition does not match the current. This can
      // happen because we generate transitions over a 14-month interval
      // spanning the current year.
      struct AtcDateTuple *start = &((*t)->start_dt);
      if (start->year_tiny + kAtcEpochYear != year) continue;

      // Skip if the UTC year bleeds under or over the boundaries.
      if ((*t)->transition_time_u.year_tiny + kAtcEpochYear < start_year) {
        continue;
      }
      if ((*t)->transition_time_u.year_tiny + kAtcEpochYear >= until_year) {
        continue;
      }

      atc_time_t epoch_seconds = (*t)->start_epoch_seconds;

      // Add a test data just before the transition
      add_test_item_from_epoch_seconds(
          test_entry,
          zone_name,
          processing,
          zone_info,
          epoch_seconds - 1,
          'A');

      // Add a test data at the transition itself (which will
      // normally be shifted forward or backwards).
      add_test_item_from_epoch_seconds(
          test_entry,
          zone_name,
          processing,
          zone_info,
          epoch_seconds,
          'B');
    }
  }
}

void add_monthly_samples(
    struct TestDataEntry *test_entry,
    const char *zone_name,
    struct AtcZoneProcessing *processing,
    const struct AtcZoneInfo *zone_info,
    int16_t start_year,
    int16_t until_year)
{
  for (int y = start_year; y < until_year; y++) {
    for (int m = 1; m <= 12; m++) {
      // Add a sample test point on the *second* of each month instead of the
      // first of the month. This prevents Jan 1, 2000 from being converted to a
      // negative epoch seconds for certain timezones, which gets converted into
      // a UTC date in 1999 when ExtendedZoneProcessor is used to convert the
      // epoch seconds back to a ZonedDateTime. The UTC date in 1999 causes the
      // actual max buffer size of ExtendedZoneProcessor to become different
      // than the one predicted by BufSizeEstimator (which samples whole years
      // from 2000 until 2050), and causes the
      // AceTimeValidation/ExtendedHinnantDateTest to fail on the buffer size
      // check.
      //
      // But if that day of the month (with the time of 00:00) is ambiguous.
      // I use a loop to try every subsequent day of month up to the 28th (which
      // exists in all months).
      for (int d = 2; d <= 28; d++) {
        struct AtcZonedDateTime zdt;
        bool status = atc_zoned_date_time_from_components(
            processing,
            zone_info,
            y, m, d, 0, 0, 0,
            0 /*fold*/,
            &zdt);
        if (! status) continue;

        atc_time_t epoch_seconds = atc_zoned_date_time_to_epoch_seconds(&zdt);
        add_test_item_from_epoch_seconds(
            test_entry,
            zone_name,
            processing,
            zone_info,
            epoch_seconds,
            'S');
      }
    }

    // Add the last day of the year...
    struct AtcZonedDateTime zdt;
    bool status = atc_zoned_date_time_from_components(
        processing,
        zone_info,
        y, 12, 31, 0, 0, 0,
        0 /*fold*/,
        &zdt);
    if (! status) continue;
    atc_time_t epoch_seconds = atc_zoned_date_time_to_epoch_seconds(&zdt);
    add_test_item_from_epoch_seconds(
        test_entry,
        zone_name,
        processing,
        zone_info,
        epoch_seconds,
        'Y');
  }
}
