#include <stdio.h>
#include <stdlib.h> // realloc()
#include "test_data.h"

void testCollectionInit(TestCollection *collection)
{
  collection->numItems = 0;
  collection->capacity = 0;
  collection->items = NULL;
  testCollectionResize(collection, 10);
}

void testCollectionClear(TestCollection *collection)
{
  free(collection->items);
  collection->numItems = 0;
  collection->capacity = 0;
  collection->items = NULL;
}

void testCollectionResize(TestCollection *collection, int newsize)
{
  TestItem* newitems = (TestItem*) realloc(
      collection->items, sizeof(TestItem) * newsize);
  if (newitems == NULL) {
    fprintf(stderr, "testCollectionResize(): realloc failure\n");
    exit(1);
  }
  collection->items = newitems;
  collection->capacity = newsize;
}

TestItem *testCollectionNewItem(TestCollection *collection)
{
  if (collection->numItems >= collection->capacity) {
    testCollectionResize(collection, collection->capacity * 2);
  }
  return &collection->items[collection->numItems++];
}

void testCollectionDeleteItem(TestCollection *collection)
{
  collection->numItems--;
}

//-----------------------------------------------------------------------------

void test_data_entry_init(TestEntry *entry)
{
  testCollectionInit(&entry->transitions);
  testCollectionInit(&entry->samples);
}

void testDataEntryClear(TestEntry *entry)
{
  testCollectionClear(&entry->samples);
  testCollectionClear(&entry->transitions);
}

//-----------------------------------------------------------------------------

void testDataInit(TestData *data)
{
  data->num_entries = 0;
  data->capacity = 0;
  data->entries = NULL;
  testDataResize(data, 10);
}

void testDataClear(TestData *data)
{
  for (int i = data->num_entries - 1; i >= 0; i--) {
    testDataEntryClear(&data->entries[i]);
  }
  free(data->entries);

  data->num_entries = 0;
  data->capacity = 0;
  data->entries = NULL;
}

void testDataResize(TestData *data, int newsize)
{
  TestEntry* newentries = (TestEntry*) realloc(
      data->entries, sizeof(TestEntry) * newsize);
  if (newentries == NULL) {
    fprintf(stderr, "testDataResize(): realloc failure\n");
    exit(1);
  }
  data->entries = newentries;
  data->capacity = newsize;
}

TestEntry *testDataNewEntry(TestData *data)
{
  if (data->num_entries >= data->capacity) {
    testDataResize(data, data->capacity * 2);
  }
  TestEntry *entry = &data->entries[data->num_entries++];
  test_data_entry_init(entry);
  return entry;
}

void testDataDeleteEntry(TestData *data)
{
  data->num_entries--;
  TestEntry *entry = &data->entries[data->num_entries];
  testDataEntryClear(entry);
}

//-----------------------------------------------------------------------------

static void printItem(const char *indent, const TestItem *item) {
  printf("%s\"epoch\": %ld,\n", indent, item->epochSeconds);
  printf("%s\"total_offset\": %d,\n", indent, item->utcOffset);
  printf("%s\"dst_offset\": %d,\n", indent, item->dstOffset);
  printf("%s\"y\": %d,\n", indent, item->year);
  printf("%s\"M\": %d,\n", indent, item->month);
  printf("%s\"d\": %d,\n", indent, item->day);
  printf("%s\"h\": %d,\n", indent, item->hour);
  printf("%s\"m\": %d,\n", indent, item->minute);
  printf("%s\"s\": %d,\n", indent, item->second);
  printf("%s\"abbrev\": \"%s\",\n", indent, item->abbrev);
  printf("%s\"type\": \"%c\"\n", indent, item->type);
}

void printJson(
  const TestData *testData,
  int startYear,
  int untilYear,
  int epochYear,
  const char *version,
  const char *tzVersion) {

  const char indent0[] = "  ";
  const char indent1[] = "    ";
  const char indent2[] = "      ";
  const char indent3[] = "        ";
  const char indent4[] = "          ";

  printf("{\n");
  printf("%s\"start_year\": %d,\n", indent0, startYear);
  printf("%s\"until_year\": %d,\n", indent0, untilYear);
  printf("%s\"epoch_year\": %d,\n", indent0, epochYear);
  printf("%s\"source\": \"AceTimeC\",\n", indent0);
  printf("%s\"version\": \"%s\",\n", indent0, version);
  printf("%s\"tz_version\": \"%s\",\n", indent0, tzVersion);
  printf("%s\"has_valid_abbrev\": true,\n", indent0);
  printf("%s\"has_valid_dst\": true,\n", indent0);
  printf("%s\"test_data\": {\n", indent0);

  // Print each zone
  int num_zones = testData->num_entries;
  for (int z = 0; z < num_zones; z++) {
    const TestEntry *entry = &testData->entries[z];
    const char *zone_name = entry->zone_name;
    printf("%s\"%s\": {\n", indent1, zone_name);

    // Print transitions
    printf("%s\"%s\": [\n", indent2, "transitions");
    const TestCollection *collection = &entry->transitions;
    for (int i = 0; i < collection->numItems; i++) {
      const TestItem *item = &collection->items[i];
      printf("%s{\n", indent3);
      printItem(indent4, item);
      printf("%s}%s\n", indent3, (i < collection->numItems - 1) ? "," : "");
    }
    printf("%s],\n", indent2);

    // Print samples
    printf("%s\"%s\": [\n", indent2, "samples");
    collection = &entry->samples;
    for (int i = 0; i < collection->numItems; i++) {
      const TestItem *item = &collection->items[i];
      printf("%s{\n", indent3);
      printItem(indent4, item);
      printf("%s}%s\n", indent3, (i < collection->numItems - 1) ? "," : "");
    }
    printf("%s]\n", indent2);

    printf("%s}%s\n", indent1, (z < testData->num_entries - 1) ? "," : "");
  }

  printf("%s}\n", indent0);
  printf("}\n");
}
