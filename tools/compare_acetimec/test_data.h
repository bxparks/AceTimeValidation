#ifndef COMPARE_ACETIMEC_TEST_DATA_H
#define COMPARE_ACETIMEC_TEST_DATA_H

#include <acetimec.h> // kAtcAbbrevSize

/** Buffer size of each line from zones.txt file. */
#define MAX_LINE_SIZE 512

#define ZONE_NAME_SIZE 64

//-----------------------------------------------------------------------------

/** DateTime components. */
typedef struct DateTime {
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
} DateTime;

//-----------------------------------------------------------------------------

/**
 * A test item, containing the epoch_seconds with its expected DateTime
 * components.
 */
typedef struct TestItem {
  long epoch_seconds;
  int utc_offset; // seconds
  int dst_offset; // seconds
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
  char abbrev[kAtcAbbrevSize];
  char type; //'A', 'B', 'a', 'b', 'S'
} TestItem;

/** A growable collection of test items. */
typedef struct TestCollection {
  int capacity;
  int num_items;
  TestItem *items;
} TestCollection;

//-----------------------------------------------------------------------------

/** Initialize the given TestCollection. */
void test_collection_init(TestCollection *collection);

/** Free the given TestCollection. */
void test_collection_clear(TestCollection *collection);

/** Resize the array of items. */
void test_collection_resize(TestCollection *collection, int newsize);

/**
 * Allocate and return the next test item, resizing the TestCollection as
 * needed.
 */
TestItem *test_collection_new_item(TestCollection *collection);

/** Push back the last unused TestItem. */
void test_collection_delete_item(TestCollection *collection);

//-----------------------------------------------------------------------------

/** Test entries for a single zone. */
typedef struct TestEntry {
  char zone_name[ZONE_NAME_SIZE];
  TestCollection transitions;
  TestCollection samples;
} TestEntry;

//-----------------------------------------------------------------------------

/** Array of test entries, for all zones. */
typedef struct TestData {
  int capacity;
  int num_entries;
  TestEntry *entries;
} TestData;

/** Initialize the given TestData. */
void test_data_init(TestData *data);

/** Clear the given TestData. */
void test_data_clear(TestData *data);

/** Resize the array of entries. */
void test_data_resize(TestData *data, int newsize);

/**
 * Allocate and return the next test data entry for a single zone, resizing
 * TestData as needed.
 */
TestEntry *test_data_new_entry(TestData *data);

/** Roll back the last unused TestEntry. */
void test_data_delete_entry(TestData *data);

//-----------------------------------------------------------------------------

// TODO: Remove
/** Sort the TestItems of each TestEntry according to epochSeconds. */
//void sort_test_data(TestData *test_data);

/**
 * Generate the JSON output on STDOUT which will be redirect into
 * 'validation_data.json' file. Adopted from GenerateData.java.
 */
void print_json(
  const TestData *test_data,
  int start_year,
  int until_year,
  int epoch_year,
  const char *version, // library version
  const char *tz_version); // TZDB version

#endif
