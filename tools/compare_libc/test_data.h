#ifndef COMPARE_ACETIMEC_TEST_DATA_H
#define COMPARE_ACETIMEC_TEST_DATA_H

#define MAX_ABBREV_SIZE 7 /* 6 + NUL */

/** Difference between Unix epoch (1970-01-1) and AceTime Epoch (2000-01-01). */
#define SECONDS_SINCE_UNIX_EPOCH 946684800

/** Longest line length in the input zones.txt file. */
#define MAX_LINE_LENGTH 400

#define ZONE_NAME_SIZE 64

//-----------------------------------------------------------------------------

/** DateTime components. */
struct DateTime {
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
};

//-----------------------------------------------------------------------------

/**
 * A test item, containing the epoch_seconds with its expected DateTime
 * components.
 */
struct TestItem {
  long epoch_seconds;
  int utc_offset; // total offset seconds
  int dst_offset; // DST offset seconds
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
  char abbrev[MAX_ABBREV_SIZE];
  char type; //'A', 'B', 'a', 'b', 'S'
};

/** A growable collection of test items. */
struct TestCollection {
  int capacity;
  int num_items;
  struct TestItem *items;
};

//-----------------------------------------------------------------------------

/** Initialize the given TestCollection. */
void test_collection_init(struct TestCollection *collection);

/** Clear the given TestCollection. */
void test_collection_clear(struct TestCollection *collection);

/** Resize the array of items. */
void test_collection_resize(struct TestCollection *collection, int newsize);

/**
 * Allocate and return the next test item, resizing the TestCollection as
 * needed.
 */
struct TestItem *test_collection_new_item(struct TestCollection *collection);

/** Delete the last unused TestItem. */
void test_collection_delete_item(struct TestCollection *collection);

// TODO: Remove
/** Sort the items in the test_data entry. */
void test_collection_sort_items(struct TestCollection *collection);

//-----------------------------------------------------------------------------

/** Test items for a single zone, composed of transitions and samples. */
struct TestEntry {
  char zone_name[ZONE_NAME_SIZE];
  struct TestCollection transitions;
  struct TestCollection samples;
};

/** Initialize the given TestEntry for a zone. */
void test_entry_init(struct TestEntry *entry);

/** Clear the given TestEntry for a zone. */
void test_entry_clear(struct TestEntry *entry);

//-----------------------------------------------------------------------------

/** Array of test entries, for all zones. */
struct TestData {
  int capacity;
  int num_entries;
  struct TestEntry *entries;
};

/** Initialize the given TestData. */
void test_data_init(struct TestData *data);

/** Clear the given TestData. */
void test_data_clear(struct TestData *data);

/** Resize the array of entries. */
void test_data_resize(struct TestData *data, int newsize);

/**
 * Allocate and return the next test data entry for a single zone,
 * resizing TestData as needed.
 */
struct TestEntry *test_data_new_entry(struct TestData *data);

/** Roll back the last unused TestEntry. */
void test_data_delete_entry(struct TestData *data);

/** Sort the TestItems of each TestEntry according to epochSeconds. */
void sort_test_data(struct TestData *test_data);

//-----------------------------------------------------------------------------

/**
 * Generate the JSON output on STDOUT which will be redirect into
 * 'validation_data.json' file. Adopted from GenerateData.java.
 */
void print_json(
  const struct TestData *test_data,
  int start_year,
  int until_year,
  int epoch_year,
  const char *source, // source of the TZ validation data.
  const char *version, // version of the TZ source
  const char *tz_version); // TZDB version

#endif
