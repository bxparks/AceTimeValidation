#include <stdint.h>
#include <string.h> // strncpy()
#include <stdio.h> // printf()
#include <acetimec.h>
#include "sampling.h"

static const int SAMPLING_INTERVAL_HOURS = 22;

static int8_t create_test_item_from_epoch_seconds(
    TestItem *ti,
    const AtcTimeZone *tz,
    atc_time_t epoch_seconds,
    char type)
{
  AtcZonedDateTime zdt;
  atc_zoned_date_time_from_epoch_seconds(&zdt, epoch_seconds, tz);
  if (atc_zoned_date_time_is_error(&zdt)) return kAtcErrGeneric;

  ti->epoch_seconds = epoch_seconds;
  ti->year = zdt.year;
  ti->month = zdt.month;
  ti->day = zdt.day;
  ti->hour = zdt.hour;
  ti->minute = zdt.minute;
  ti->second = zdt.second;
  ti->type = type;

  AtcZonedExtra zet;
  atc_zoned_extra_from_epoch_seconds(&zet, epoch_seconds, tz);
  if ((atc_zoned_date_time_is_error(&zdt))) return kAtcErrGeneric;

  strncpy(ti->abbrev, zet.abbrev, kAtcAbbrevSize);
  ti->abbrev[kAtcAbbrevSize - 1] = '\0';
  ti->dst_offset = zet.dst_offset_seconds;
  ti->utc_offset = zet.std_offset_seconds + zet.dst_offset_seconds;

  return kAtcErrOk;
}

static int8_t add_test_item_from_epoch_seconds(
    TestCollection *collection,
    const char *zone_name,
    const AtcTimeZone *tz,
    atc_time_t epoch_seconds,
    char type)
{
  (void) zone_name;
  TestItem *ti = test_collection_new_item(collection);
  int8_t err = create_test_item_from_epoch_seconds(ti, tz, epoch_seconds, type);
  if (err) {
    test_collection_delete_item(collection);
    return err;
  }

  return kAtcErrOk;
}

//-----------------------------------------------------------------------------

// Return a non-zero code if a transition is detected from `s` to `t`:
// * -1 - error
// * 0 - no transition
// * 1 - regular transition (total UTC offset is different)
// * 2 - silent transition (both STD or DST changed and canceled each other)
static int8_t is_transition(atc_time_t t1, atc_time_t t2, const AtcTimeZone *tz)
{
  AtcZonedExtra ze1, ze2;
  atc_zoned_extra_from_epoch_seconds(&ze1, t1, tz);
  if (atc_zoned_extra_is_error(&ze1)) return -1;
  atc_zoned_extra_from_epoch_seconds(&ze2, t2, tz);
  if (atc_zoned_extra_is_error(&ze2)) return -1;

  // total UTC offsets
  int32_t offset1 = ze1.std_offset_seconds + ze1.dst_offset_seconds;
  int32_t offset2 = ze2.std_offset_seconds + ze2.dst_offset_seconds;

  if (offset1 != offset2) {
    // total UTC offset changed
    return 1;
  } else if (ze1.std_offset_seconds != ze2.std_offset_seconds) {
    // the STD offset changed, but was canceled out by the DST offset
    return 2;
  }
  return 0;
}

// Do a binary search to find the transition within the interval [left,
// right) to within 1-second accuracy, and return the transition as a tuple of
// (tleft, tright). Returns kAtcInvalidEpochSeconds upon error (which should
// not happen).
//
// Returns the following error code:
// * -1 - error
// * 0 - no transition
// * 1 - transition (total UTC offset is different)
// * 2 - silent transition (both STD and DST changed and cancelled each other)
static int8_t binary_search_transition(
  atc_time_t left, atc_time_t right,
  atc_time_t *tleft, atc_time_t *tright,
  const AtcTimeZone *tz)
{
  int8_t result = 0;
  for (;;) {
    atc_time_t delta_seconds = right - left;
    delta_seconds /= 2;
    if (delta_seconds == 0) {
      result = is_transition(left, right, tz);
      break;
    }

    atc_time_t mid = left + delta_seconds;
    result = is_transition(left, mid, tz);
    switch (result) {
      case -1:
        return result;
      case 0:
        left = mid;
        break;
      case 1:
      case 2:
        right = mid;
        break;
    }
  }

  *tleft = left;
  *tright = right;
  return result;
}

void add_transitions(
    TestCollection *collection,
    const char *zone_name,
    const AtcTimeZone *tz,
    int16_t start_year,
    int16_t until_year)
{
  AtcLocalDateTime ldt = {start_year, 1, 1, 0, 0, 0, 0 /*fold*/};
  AtcZonedDateTime zdt;
  atc_zoned_date_time_from_local_date_time(&zdt, &ldt, tz);
  if (atc_zoned_date_time_is_error(&zdt)) return;

  atc_time_t t = atc_zoned_date_time_to_epoch_seconds(&zdt);
  t -= 86400; // go back one day because local TZ may not be UTC

  atc_zoned_date_time_from_epoch_seconds(&zdt, t, tz);
  if (atc_zoned_date_time_is_error(&zdt)) return;
  for (;;) {
    atc_time_t nextt = t + SAMPLING_INTERVAL_HOURS * 3600;
    AtcZonedDateTime nextzdt;
    atc_zoned_date_time_from_epoch_seconds(&nextzdt, nextt, tz);
    if (atc_zoned_date_time_is_error(&nextzdt)) continue;
    if (nextzdt.year >= until_year) break;

    // Look for utc offset transition
    int8_t result = is_transition(t, nextt, tz);
    if (result < 0) break;

    // Check if transition found
    if (result > 0) {
      atc_time_t left, right;
      int8_t result = binary_search_transition(t, nextt, &left, &right, tz);
      if (result == 1) {
        // normal transition
        add_test_item_from_epoch_seconds(collection, zone_name, tz, left, 'A');
        add_test_item_from_epoch_seconds(collection, zone_name, tz, right, 'B');
      } else if (result == 2) {
        // silent transition
        add_test_item_from_epoch_seconds(collection, zone_name, tz, left, 'a');
        add_test_item_from_epoch_seconds(collection, zone_name, tz, right, 'b');
      }
    }

    t = nextt;
  }
}

/*
// This version uses the pre-generated transitions created by the
// AtcZoneProcessor. But the problem is that it can contain phantom transitions
// caused by switch to an internal Rule which does not cause a visible change to
// the time. Use the binary search algorithm above is slower, but more accurate.
void add_transitions(
    TestCollection *collection,
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
    AtcTransitionStorage *ts = &tz->zone_processor->transition_storage;
    AtcTransition **begin = atc_transition_storage_get_active_pool_begin(ts);
    AtcTransition **end = atc_transition_storage_get_active_pool_end(ts);
    for (AtcTransition **t = begin; t != end; ++t) {
      // Skip if start year of transition does not match the current. This can
      // happen because we generate transitions over a 14-month interval
      // spanning the current year.
      AtcDateTuple *start = &((*t)->start_dt);
      if (start->year != year) continue;

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
*/

//-----------------------------------------------------------------------------

// Add a sample test point on the *second* of each month instead of the first of
// the month. This prevents Jan 1, 2000 from being converted to a negative epoch
// seconds for certain timezones, which gets converted into a UTC date in 1999
// when epoch seconds is converted back to a ZonedDateTime. The UTC date in 1999
// causes the actual max buffer size of ExtendedZoneProcessor to become
// different than the one predicted by BufSizeEstimator (which samples whole
// years from 2000 until 2050), and causes the
// AceTimeValidation/ExtendedAcetimecTest to fail on the buffer size check.
//
// But if that day of the month (with the time of 00:00) is ambiguous, I use a
// loop to try subsequent days of month to find a day that works.  The first
// attempt is marked with a type 'S'; subsequent attempts are marked with a 'T'.
void add_monthly_samples(
    TestCollection *collection,
    const char *zone_name,
    const AtcTimeZone *tz,
    int16_t start_year,
    int16_t until_year)
{
  for (int y = start_year; y < until_year; y++) {
    for (int m = 1; m <= 12; m++) {
      char type = 'S';
      for (int d = 2; d <= 28; d++) {
        AtcZonedExtra extra;
        AtcLocalDateTime ldt = {y, m, d, 0, 0, 0, 0 /*fold*/};
        atc_zoned_extra_from_local_date_time(&extra, &ldt, tz);
        if (extra.type == kAtcZonedExtraExact
            || extra.type == kAtcZonedExtraOverlap) {
          AtcZonedDateTime zdt;
          atc_zoned_date_time_from_local_date_time(&zdt, &ldt, tz);
          if (!atc_zoned_date_time_is_error(&zdt)) {
            atc_time_t epoch_seconds =
                atc_zoned_date_time_to_epoch_seconds(&zdt);
            if (epoch_seconds != kAtcInvalidEpochSeconds) {
              add_test_item_from_epoch_seconds(
                  collection, zone_name, tz, epoch_seconds, type);
              break;
            }
          }
        }
        type = 'T';
      }
    }
  }
}
