#include <stdint.h>
#include <string.h> // strncpy()
#include <stdio.h> // printf()
#include <Arduino.h>
#include <AceTime.h>
#include "sampling.h"

using namespace ace_time;

static const int SAMPLING_INTERVAL_HOURS = 22;

static int8_t createTestItemFromEpochSeconds(
    TestItem *ti,
    const TimeZone& tz,
    acetime_t epochSeconds,
    char type,
    int64_t epochOffset) {

  ZonedDateTime zdt = ZonedDateTime::forEpochSeconds(epochSeconds, tz);
  if (zdt.isError()) return 1;

  int64_t unixSeconds = epochSeconds
      + Epoch::secondsToCurrentEpochFromUnixEpoch64();
  ti->epochSeconds = unixSeconds + epochOffset;
  ti->year = zdt.year();
  ti->month = zdt.month();
  ti->day = zdt.day();
  ti->hour = zdt.hour();
  ti->minute = zdt.minute();
  ti->second = zdt.second();
  ti->type = type;

  ZonedExtra extra = ZonedExtra::forEpochSeconds(epochSeconds, tz);
  if (zdt.isError()) return 1;

  strncpy(ti->abbrev, extra.abbrev(), ZonedExtra::kAbbrevSize);
  ti->abbrev[ZonedExtra::kAbbrevSize - 1] = '\0';
  ti->dstOffset = extra.dstOffset().toSeconds();
  ti->utcOffset = extra.timeOffset().toSeconds();

  return 0;
}

static int8_t addTestItemFromEpochSeconds(
    TestCollection *collection,
    const char *zoneName,
    const TimeZone& tz,
    acetime_t epochSeconds,
    char type,
    int64_t epochOffset) {

  (void) zoneName;
  TestItem *ti = testCollectionNewItem(collection);
  int8_t err = createTestItemFromEpochSeconds(ti, tz, epochSeconds, type,
      epochOffset);
  if (err) {
    testCollectionDeleteItem(collection);
    return err;
  }

  return 0;
}

//-----------------------------------------------------------------------------

// Return a non-zero code if a transition is detected from `s` to `t`:
// * -1 - error
// * 0 - no transition
// * 1 - regular transition (total UTC offset is different)
// * 2 - silent transition (both STD or DST changed and canceled each other)
static int8_t isTransition(acetime_t t1, acetime_t t2, const TimeZone &tz)
{
  ZonedExtra ze1 = ZonedExtra::forEpochSeconds(t1, tz);
  if (ze1.isError()) return -1;
  ZonedExtra ze2 = ZonedExtra::forEpochSeconds(t2, tz);
  if (ze2.isError()) return -1;

  // total UTC offsets
  int32_t offset1 = ze1.timeOffset().toSeconds();
  int32_t offset2 = ze2.timeOffset().toSeconds();

  if (offset1 != offset2) {
    // total UTC offset changed
    return 1;
  } else if (ze1.stdOffset().toSeconds() != ze2.stdOffset().toSeconds()) {
    // the STD offset changed, but was canceled out by the DST offset
    return 2;
  }
  return 0;
}

// Do a binary search to find the transition within the interval [left,
// right) to within 1-second accuracy, and return the transition as a tuple of
// (tleft, tright). Returns kInvalidEpochSeconds upon error (which should
// not happen).
//
// Returns the following error code:
// * -1 - error
// * 0 - no transition
// * 1 - transition (total UTC offset is different)
// * 2 - silent transition (both STD and DST changed and cancelled each other)
static int8_t binarySearchTransition(
  acetime_t left, acetime_t right,
  acetime_t* tleft, acetime_t* tright,
  const TimeZone& tz)
{
  int8_t result = 0;
  for (;;) {
    acetime_t delta_seconds = right - left;
    delta_seconds /= 2;
    if (delta_seconds == 0) {
      result = isTransition(left, right, tz);
      break;
    }

    acetime_t mid = left + delta_seconds;
    result = isTransition(left, mid, tz);
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

void addTransitionsForChunk(
    TestCollection *collection,
    const char *zoneName,
    const TimeZone &tz,
    int16_t startYear,
    int16_t untilYear,
    int64_t epochOffset) {

  auto zdt = ZonedDateTime::forComponents(startYear, 1, 1, 0, 0, 0, tz);
  if (zdt.isError()) return;

  acetime_t t = zdt.toEpochSeconds();
  t -= 86400; // go back one day because local TZ may not be UTC

  zdt = ZonedDateTime::forEpochSeconds(t, tz);
  if (zdt.isError()) return;
  for (;;) {
    acetime_t nextt = t + SAMPLING_INTERVAL_HOURS * 3600;
    ZonedDateTime nextzdt = ZonedDateTime::forEpochSeconds(nextt, tz);
    if (nextzdt.isError()) continue;
    if (nextzdt.year() >= untilYear) break;

    // Look for utc offset transition
    int8_t result = isTransition(t, nextt, tz);
    if (result < 0) break;

    // Check if transition found
    if (result > 0) {
      acetime_t left, right;
      int8_t result = binarySearchTransition(t, nextt, &left, &right, tz);
      if (result == 1) {
        // normal transition
        addTestItemFromEpochSeconds(
            collection, zoneName, tz, left, 'A', epochOffset);
        addTestItemFromEpochSeconds(
            collection, zoneName, tz, right, 'B', epochOffset);
      } else if (result == 2) {
        // silent transition
        addTestItemFromEpochSeconds(
            collection, zoneName, tz, left, 'a', epochOffset);
        addTestItemFromEpochSeconds(
            collection, zoneName, tz, right, 'b', epochOffset);
      }
    }

    t = nextt;
  }
}

void addTransitions(
    TestCollection *collection,
    const char *zoneName,
    const TimeZone &tz,
    int16_t startYear,
    int16_t untilYear,
    int64_t epochOffset) {

  // Loop in chunks of 100 years, to avoid overflowing unix_seconds.
  for (int16_t start = startYear; start < untilYear; start += 100) {
    int16_t epochYear = start + 50;
    Epoch::currentEpochYear(epochYear);
    int16_t until = start + 100;
    if (until > untilYear) until = untilYear;
    addTransitionsForChunk(collection, zoneName, tz, start, until, epochOffset);
  }
}

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
void addMonthlySamplesForChunk(
    TestCollection *collection,
    const char* zoneName,
    const TimeZone& tz,
    int16_t startYear,
    int16_t untilYear,
    int64_t epochOffset) {

  for (int y = startYear; y < untilYear; y++) {
    for (int m = 1; m <= 12; m++) {
      char type = 'S';
      for (int d = 2; d <= 28; d++) {
        LocalDateTime ldt = LocalDateTime::forComponents(y, m, d, 0, 0, 0);
        ZonedExtra extra = ZonedExtra::forLocalDateTime(ldt, tz);
        if (extra.type() == ZonedExtra::kTypeExact
            || extra.type() == ZonedExtra::kTypeOverlap) {
          ZonedDateTime zdt = ZonedDateTime::forLocalDateTime(ldt, tz);
          if (!zdt.isError()) {
            acetime_t epochSeconds = zdt.toEpochSeconds();
            if (epochSeconds != LocalDate::kInvalidEpochSeconds) {
              addTestItemFromEpochSeconds(
                  collection, zoneName, tz, epochSeconds, type, epochOffset);
              break;
            }
          }
        }
        type = 'T';
      }
    }
  }
}

void addMonthlySamples(
    TestCollection *collection,
    const char* zoneName,
    const TimeZone& tz,
    int16_t startYear,
    int16_t untilYear,
    int64_t epochOffset) {

  // Loop in chunks of 100 years, to avoid overflowing unix_seconds.
  for (int16_t start = startYear; start < untilYear; start += 100) {
    int16_t epochYear = start + 50;
    Epoch::currentEpochYear(epochYear);
    int16_t until = start + 100;
    if (until > untilYear) until = untilYear;
    addMonthlySamplesForChunk(
        collection, zoneName, tz, start, until, epochOffset);
  }
}
