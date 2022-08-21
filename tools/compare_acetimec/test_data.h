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

/** Test entries for a single zone. */
struct TestDataEntry {
  char zone_name[ZONE_NAME_SIZE];
  int capacity;
  int num_items;
  struct TestItem *items;
};

/** Array of test entries, for all zones. */
struct TestData {
  int capacity;
  int num_entries;
  struct TestDataEntry *entries;
};

//-----------------------------------------------------------------------------

/** Initialize the given TestDataEntry. */
void test_data_entry_init(struct TestDataEntry *entry);

/** Free the given TestDataEntry. */
void test_data_entry_free(struct TestDataEntry *entry);

/** Resize the array of items. */
void test_data_entry_resize_items(struct TestDataEntry *entry, int newsize);

/** Return the next test item if available. */
struct TestItem *test_data_entry_next_item(struct TestDataEntry *entry);

//-----------------------------------------------------------------------------

/** Initialize the given TestData. */
void test_data_init(struct TestData *data);

/** Free the given TestData. */
void test_data_free(struct TestData *data);

/** Resize the array of entries. */
void test_data_resize_entries(struct TestData *data, int newsize);

/** Return the next test data entry. */
struct TestDataEntry *test_data_next_entry(struct TestData *data);

#endif
