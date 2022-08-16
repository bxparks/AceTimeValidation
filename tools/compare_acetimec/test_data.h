#ifndef COMPARE_ACETIMEC_TEST_DATA_H
#define COMPARE_ACETIMEC_TEST_DATA_H

#define MAX_ABBREV_SIZE 7 /* 6 + NUL */

/** Difference between Unix epoch (1970-01-1) and AceTime Epoch (2000-01-01). */
#define SECONDS_SINCE_UNIX_EPOCH 946684800

/** Longest line length in the input zones.txt file. */
#define MAX_LINE_LENGTH 400

/** DateTime components. */
struct DateTime {
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
};

/**
 * A test item, containing the epoch_seconds with its expected DateTime
 * components.
 */
struct TestItem {
  long epoch_seconds;
  int utc_offset; // seconds
  int dst_offset; // seconds
  char abbrev[MAX_ABBREV_SIZE];
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
  char type; //'A', 'B', 'S', 'T' or 'Y'
};

#define ZONE_NAME_SIZE 64
#define MAX_NUM_ITEMS 200
#define MAX_NUM_ZONES 600

/** Test entries for a single zone. */
struct TestDataEntry {
  char zone_name[ZONE_NAME_SIZE];
  int num_items;
  struct TestItem items[MAX_NUM_ITEMS];
};

/** Array of test entries, for all zones. */
struct TestData {
  int num_zones;
  struct TestDataEntry entries[MAX_NUM_ZONES];
};

#endif
