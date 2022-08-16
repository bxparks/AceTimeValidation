#include <stdint.h>
#include <string.h> // strncpy()
#include <stdio.h> // printf()
#include <acetimec.h>
#include "val_data.h"

static struct AtcZoneProcessing processing;

void val_data_init()
{
  atc_processing_init(&processing);
}

static void create_test_item_from_epoch_seconds(
    struct TestItem *ti,
    const struct AtcZoneInfo *zone_info,
    atc_time_t epoch_seconds,
    char type)
{
  struct AtcZonedDateTime zdt;
  atc_zoned_date_time_from_epoch_seconds(
      &processing,
      zone_info,
      epoch_seconds,
      &zdt);
  struct AtcZonedExtra zet;
  atc_zoned_extra_from_epoch_seconds(
      &processing,
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
    const struct AtcZoneInfo *zone_info,
    atc_time_t epoch_seconds,
    char type)
{
  if (test_entry->num_items >= MAX_NUM_ITEMS) return;

  struct TestItem *ti = &test_entry->items[test_entry->num_items];
  create_test_item_from_epoch_seconds(ti, zone_info, epoch_seconds, type);

  // Increment to next item.
  test_entry->num_items++;
  if (test_entry->num_items >= MAX_NUM_ITEMS) {
    fprintf(stderr, "Error: %s: exceeded test items\n", zone_name);
  }
}

void add_transitions(
    struct TestDataEntry *test_entry,
    const char *zone_name,
    const struct AtcZoneInfo *zone_info,
    int16_t start_year,
    int16_t until_year)
{
  strncpy(test_entry->zone_name, zone_name, ZONE_NAME_SIZE);
  test_entry->zone_name[ZONE_NAME_SIZE - 1] = '\0';

  for (int16_t year = start_year; year < until_year; ++year) {
    atc_processing_init_for_year(&processing, zone_info, year);
    struct AtcTransitionStorage *ts = &processing.transition_storage;
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
          zone_info,
          epoch_seconds - 1,
          'A');

      // Add a test data at the transition itself (which will
      // normally be shifted forward or backwards).
      add_test_item_from_epoch_seconds(
          test_entry,
          zone_name,
          zone_info,
          epoch_seconds,
          'B');
    }
  }
}
