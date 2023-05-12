#ifndef COMPARE_ACETIME_TEST_DATA_H
#define COMPARE_ACETIME_TEST_DATA_H

#define MAX_ABBREV_SIZE 7 /* 6 + NUL */

/** Difference between Unix epoch (1970-01-1) and AceTime Epoch (2000-01-01). */
#define SECONDS_SINCE_UNIX_EPOCH 946684800

/** Buffer size of each line from zones.txt file. */
#define MAX_LINE_SIZE 512

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
 * A test item, containing the epochSeconds with its expected DateTime
 * components.
 */
struct TestItem {
  long epochSeconds;
  int utcOffset; // seconds
  int dstOffset; // seconds
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
  int numItems;
  TestItem *items;
};

//-----------------------------------------------------------------------------

/** Initialize the given TestCollection. */
void testCollectionInit(TestCollection *collection);

/** Free the given TestCollection. */
void testCollectionClear(TestCollection *collection);

/** Resize the array of items. */
void testCollectionResize(TestCollection *collection, int newsize);

/**
 * Allocate and return the next test item, resizing the TestCollection as
 * needed.
 */
TestItem *testCollectionNewItem(TestCollection *collection);

/** Push back the last unused TestItem. */
void testCollectionDeleteItem(TestCollection *collection);

//-----------------------------------------------------------------------------

/** Test entries for a single zone. */
struct TestEntry {
  char zone_name[ZONE_NAME_SIZE];
  TestCollection transitions;
  TestCollection samples;
};

//-----------------------------------------------------------------------------

/** Array of test entries, for all zones. */
struct TestData {
  int capacity;
  int num_entries;
  TestEntry *entries;
};

/** Initialize the given TestData. */
void testDataInit(TestData *data);

/** Clear the given TestData. */
void testDataClear(TestData *data);

/** Resize the array of entries. */
void testDataResize(TestData *data, int newsize);

/**
 * Allocate and return the next test data entry for a single zone, resizing
 * TestData as needed.
 */
TestEntry *testDataNewEntry(TestData *data);

/** Roll back the last unused TestEntry. */
void testDataDeleteEntry(TestData *data);

//-----------------------------------------------------------------------------

/**
 * Generate the JSON output on STDOUT which will be redirect into
 * 'validation_data.json' file. Adopted from GenerateData.java.
 */
void printJson(
  const TestData *testData,
  int startYear,
  int untilYear,
  int epochYear,
  const char *version, // library version
  const char *tzVersion); // TZDB version

#endif
