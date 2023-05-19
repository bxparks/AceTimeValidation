#include <stdbool.h>
#include <stdint.h>
#include <string.h> // strncpy()
#include <time.h> // mktime(), timegm(), localtime_r()
#include "sampling.h"

time_t seconds_to_ace_time_epoch_from_unix_epoch;

static time_t convert_ace_time_to_unix_time(long epoch_seconds)
{
  return (int64_t)epoch_seconds + seconds_to_ace_time_epoch_from_unix_epoch;
}

static long convert_unix_time_to_ace_time(time_t unix_seconds)
{
  return (long)(unix_seconds - seconds_to_ace_time_epoch_from_unix_epoch);
}

// Convert time (y, d, h, h, m, s) in the current TZ to unix seconds.
static int64_t to_unix_seconds(int y, int mon, int d, int h, int m, int s)
{
  struct tm tms;
  tms.tm_year = y - 1900;
  tms.tm_mon = mon - 1;
  tms.tm_mday = d;
  tms.tm_hour = h;
  tms.tm_min = m;
  tms.tm_sec = s;
  tms.tm_isdst = -1; // unknown, tell mktime() to figure it out
  return mktime(&tms);
}

void set_ace_time_epoch_year(int epoch_year)
{
  struct tm tms;
  tms.tm_year = epoch_year - 1900;
  tms.tm_mon = 1 - 1;
  tms.tm_mday = 1;
  tms.tm_hour = 0;
  tms.tm_min = 0;
  tms.tm_sec = 0;
  tms.tm_isdst = -1; // unknown, tell mktime() to figure it out

  // timegm() is a non-POSIX extension. It converts struct tm to unix seconds
  // using *UTC*. In contrast, mktime() converts the 'struct tm` to unix seconds
  // using the current *TZ*.
  seconds_to_ace_time_epoch_from_unix_epoch = timegm(&tms);
}

//-----------------------------------------------------------------------------

struct LocalDateTime {
  int16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  int32_t offset;
};

static struct LocalDateTime to_local_date_time(time_t unix_seconds)
{
  struct tm tms;
  localtime_r(&unix_seconds, &tms);
  struct LocalDateTime ldt;
  ldt.year = (int16_t)(tms.tm_year + 1900);
  ldt.month = (uint8_t)(tms.tm_mon + 1);
  ldt.day = (uint8_t)tms.tm_mday;
  ldt.hour = (uint8_t)tms.tm_hour;
  ldt.minute = (uint8_t)tms.tm_min;
  ldt.second = (uint8_t)tms.tm_sec;
  ldt.offset = (long)tms.tm_gmtoff;
  return ldt;
}

//-----------------------------------------------------------------------------

static int8_t create_test_item_from_epoch_seconds(
    struct TestItem *ti,
    time_t epoch_seconds,
    char type)
{
  struct tm tms;
  time_t unix_seconds = convert_ace_time_to_unix_time(epoch_seconds);
  localtime_r(&unix_seconds, &tms);

  ti->epoch_seconds = epoch_seconds;
  ti->year = tms.tm_year + 1900;
  ti->month = tms.tm_mon + 1;
  ti->day = tms.tm_mday;
  ti->hour = tms.tm_hour;
  ti->minute = tms.tm_min;
  ti->second = tms.tm_sec;
  ti->utc_offset = tms.tm_gmtoff;
  ti->type = type;

  if (tms.tm_isdst) {
    strncpy(ti->abbrev, tzname[1], MAX_ABBREV_SIZE);
  } else {
    strncpy(ti->abbrev, tzname[0], MAX_ABBREV_SIZE);
  }
  ti->abbrev[MAX_ABBREV_SIZE - 1] = '\0';

  // Don't know how to get this info from libc's time functions.
  ti->dst_offset = 0;

  return 0;
}

static int8_t add_test_item_from_epoch_seconds(
    struct TestCollection *collection,
    const char *zone_name,
    long epoch_seconds,
    char type)
{
  (void) zone_name;
  struct TestItem *ti = test_collection_new_item(collection);
  int8_t err = create_test_item_from_epoch_seconds(ti, epoch_seconds, type);
  if (err) {
    test_collection_delete_item(collection);
    return err;
  }

  return 0;
}

// Add a sample test point on the *second* of each month instead of the first of
// the month. This prevents Jan 1, 2000 from being converted to a negative epoch
// seconds for certain timezones, which gets converted into a UTC date in 1999
// when epoch seconds is converted back to a ZonedDateTime. The UTC date in 1999
// causes the actual max buffer size of ExtendedZoneProcessor to become
// different than the one predicted by BufSizeEstimator (which samples whole
// years from 2000 until 2050), and causes the
// AceTimeValidation/ExtendedAcetimecTest to fail on the buffer size check.
//
// But if the second of the month (with the time of 00:00) is in a gap, use a
// loop to try subsequent days to find a day that works. The first attempt is
// tagged with an 'S'; subsequent attempts are tagged with a 'T'.
void add_monthly_samples(
    struct TestCollection *collection,
    const char *zone_name,
    int16_t start_year,
    int16_t until_year)
{
  for (int y = start_year; y < until_year; y++) {
    for (int m = 1; m <= 12; m++) {
      char type = 'S';
      for (int d = 2; d <= 28; d++) {
        time_t unix_seconds = to_unix_seconds(y, m, d, 0, 0, 0);
        struct LocalDateTime ldt = to_local_date_time(unix_seconds);
        bool is_gap = ldt.year != y || ldt.month != m || ldt.day != d
            || ldt.hour != 0 || ldt.minute != 0 || ldt.second != 0;

        if (!is_gap) {
          long epoch_seconds = convert_unix_time_to_ace_time(unix_seconds);
          add_test_item_from_epoch_seconds(
              collection, zone_name, epoch_seconds, type);
          break;
        }
        type = 'T';
      }
    }
  }
}

//-----------------------------------------------------------------------------

static bool is_transition(
  struct LocalDateTime *ldt1, struct LocalDateTime *ldt2)
{
  return ldt1->offset != ldt2->offset;
}

// Do a binary search to find the (known) transition in the interval [left,
// right) to within 1 second accuracy.
static void binary_search_transition(
  time_t left, time_t right,
  time_t *t_left, time_t * t_right)
{
  struct LocalDateTime ldt_left = to_local_date_time(left);
  for (;;) {
    time_t delta_seconds = right - left;
    delta_seconds /= 2;
    if (delta_seconds == 0) break;

    time_t mid = left + delta_seconds;
    struct LocalDateTime ldt_mid = to_local_date_time(mid);
    if (is_transition(&ldt_left, &ldt_mid)) {
      right = mid;
    } else {
      left = mid;
      ldt_left = ldt_mid;
    }
  }

  *t_left = left;
  *t_right = right;
}

void add_transitions(
    struct TestCollection *collection,
    const char *zone_name,
    int16_t start_year,
    int16_t until_year,
    int interval_hours)
{
  time_t t = to_unix_seconds(start_year, 1, 1, 0, 0, 0);
  t -= 86400; // go back one day because local TZ may not be UTC
  struct LocalDateTime ldt = to_local_date_time(t);

  for (;;) {
    time_t t_next = t + interval_hours * 3600;
    struct LocalDateTime ldt_next = to_local_date_time(t_next);
    if (ldt_next.year >= until_year) break;

    // Look for utc offset transition
    if (is_transition(&ldt, &ldt_next)) {
      time_t left, right;
      binary_search_transition(t, t_next, &left, &right);
      long left_epoch_seconds = convert_unix_time_to_ace_time(left);
      long right_epoch_seconds = convert_unix_time_to_ace_time(right);
      add_test_item_from_epoch_seconds(
          collection, zone_name, left_epoch_seconds, 'A');
      add_test_item_from_epoch_seconds(
          collection, zone_name, right_epoch_seconds, 'B');
    }

    t = t_next;
    ldt = ldt_next;
  }
}
