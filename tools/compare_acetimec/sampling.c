#include <stdint.h>
#include <string.h> // strncpy()
#include <stdio.h> // printf()
#include <acetimec.h>
#include "sampling.h"

static int8_t create_test_item_from_epoch_seconds(
    struct TestItem *ti,
    const AtcTimeZone *tz,
    atc_time_t epoch_seconds,
    char type)
{
  struct AtcZonedDateTime zdt;
  int8_t err = atc_zoned_date_time_from_epoch_seconds(&zdt, epoch_seconds, tz);
  if (err) return err;

  ti->epoch_seconds = epoch_seconds;
  ti->year = zdt.year;
  ti->month = zdt.month;
  ti->day = zdt.day;
  ti->hour = zdt.hour;
  ti->minute = zdt.minute;
  ti->second = zdt.second;
  ti->type = type;

  struct AtcZonedExtra zet;
  err = atc_zoned_extra_from_epoch_seconds(&zet, epoch_seconds, tz);
  if (err) return err;

  strncpy(ti->abbrev, zet.abbrev, kAtcAbbrevSize);
  ti->abbrev[kAtcAbbrevSize - 1] = '\0';
  ti->dst_offset = zet.dst_offset_seconds;
  ti->utc_offset = zet.std_offset_seconds + zet.dst_offset_seconds;

  return kAtcErrOk;
}

static int8_t add_test_item_from_epoch_seconds(
    struct TestCollection *collection,
    const char *zone_name,
    const AtcTimeZone *tz,
    atc_time_t epoch_seconds,
    char type)
{
  (void) zone_name;
  struct TestItem *ti = test_collection_new_item(collection);
  int8_t err = create_test_item_from_epoch_seconds(
      ti, tz, epoch_seconds, type);
  if (err) {
    test_collection_delete_item(collection);
    return err;
  }

  return kAtcErrOk;
}

void add_transitions(
    struct TestCollection *collection,
    const char *zone_name,
    const AtcTimeZone *tz,
    int16_t start_year,
    int16_t until_year)
{
  // Use the internal AtcZoneProcessor and AtcTransitionStorage objects to
  // obtain the DST transitions. These objects are not intended for normal
  // public consumption, but they are useful in this situation.

  // Explicitly initialize the zone_processor because we are by-passing the
  // normal atc_time_zone_xxx() functions which perform this initialization.
  atc_processor_init_for_zone_info(tz->zone_processor, tz->zone_info);

  // Scan the years, initializing the zone_procssor for each year.
  for (int16_t year = start_year; year < until_year; ++year) {
    atc_processor_init_for_year(tz->zone_processor, year);
    struct AtcTransitionStorage *ts = &tz->zone_processor->transition_storage;
    struct AtcTransition **begin =
        atc_transition_storage_get_active_pool_begin(ts);
    struct AtcTransition **end =
        atc_transition_storage_get_active_pool_end(ts);
    for (struct AtcTransition **t = begin; t != end; ++t) {
      // Skip if start year of transition does not match the current. This can
      // happen because we generate transitions over a 14-month interval
      // spanning the current year.
      struct AtcDateTuple *start = &((*t)->start_dt);
      if (start->year != year) continue;

      // Skip if the UTC year bleeds under or over the boundaries.
      if ((*t)->transition_time_u.year < start_year) {
        continue;
      }
      if ((*t)->transition_time_u.year >= until_year) {
        continue;
      }

      atc_time_t epoch_seconds = (*t)->start_epoch_seconds;

      // Add a test data just before the transition
      int8_t err = add_test_item_from_epoch_seconds(
          collection, zone_name, tz, epoch_seconds - 1, 'A');
      if (err) continue;

      // Add a test data at the transition itself (which will
      // normally be shifted forward or backwards).
      err = add_test_item_from_epoch_seconds(
          collection, zone_name, tz, epoch_seconds, 'B');
      if (err) continue;
    }
  }
}

void add_monthly_samples(
    struct TestCollection *collection,
    const char *zone_name,
    const AtcTimeZone *tz,
    int16_t start_year,
    int16_t until_year)
{
  for (int y = start_year; y < until_year; y++) {
    for (int m = 1; m <= 12; m++) {
      // Add a sample test point on the *second* of each month instead of the
      // first of the month. This prevents Jan 1, 2000 from being converted to a
      // negative epoch seconds for certain timezones, which gets converted into
      // a UTC date in 1999 when epoch seconds is converted back to a
      // ZonedDateTime. The UTC date in 1999 causes the actual max buffer
      // size of ExtendedZoneProcessor to become different than the one
      // predicted by BufSizeEstimator (which samples whole years from 2000
      // until 2050), and causes the AceTimeValidation/ExtendedAceTimeCTest
      // to fail on the buffer size check.
      //
      // But if that day of the month (with the time of 00:00) is ambiguous, I
      // use a loop to try subsequent days of month to find a day that works.
      for (int d = 2; d <= 28; d++) {
        struct AtcZonedDateTime zdt;
        struct AtcLocalDateTime ldt = {y, m, d, 0, 0, 0, 0 /*fold*/};
        int8_t err = atc_zoned_date_time_from_local_date_time(&zdt, &ldt, tz);
        if (err) continue;

        atc_time_t epoch_seconds = atc_zoned_date_time_to_epoch_seconds(&zdt);
        if (epoch_seconds == kAtcInvalidEpochSeconds) continue;
        add_test_item_from_epoch_seconds(
            collection, zone_name, tz, epoch_seconds, 'S');
        break;
      }
    }

    // Add the last day of the year...
    struct AtcZonedDateTime zdt;
    struct AtcLocalDateTime ldt = {y, 12, 31, 0, 0, 0, 0 /*fold*/ };
    int8_t err = atc_zoned_date_time_from_local_date_time(
        &zdt, &ldt, tz);
    if (err) continue;

    atc_time_t epoch_seconds = atc_zoned_date_time_to_epoch_seconds(&zdt);
    if (epoch_seconds == kAtcInvalidEpochSeconds) continue;
    add_test_item_from_epoch_seconds(
        collection, zone_name, tz, epoch_seconds, 'Y');
  }
}
