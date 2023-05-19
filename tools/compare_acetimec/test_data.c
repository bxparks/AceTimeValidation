#include <stdio.h>
#include <stdlib.h> // realloc()
#include "test_data.h"

void test_collection_init(TestCollection *collection)
{
  collection->num_items = 0;
  collection->capacity = 0;
  collection->items = NULL;
  test_collection_resize(collection, 10);
}

void test_collection_clear(TestCollection *collection)
{
  free(collection->items);
  collection->num_items = 0;
  collection->capacity = 0;
  collection->items = NULL;
}

void test_collection_resize(TestCollection *collection, int newsize)
{
  TestItem *newitems = realloc(collection->items, sizeof(TestItem) * newsize);
  if (newitems == NULL) {
    fprintf(stderr, "test_collection_resize(): realloc failure\n");
    exit(1);
  }
  collection->items = newitems;
  collection->capacity = newsize;
}

TestItem *test_collection_new_item(TestCollection *collection)
{
  if (collection->num_items >= collection->capacity) {
    test_collection_resize(collection, collection->capacity * 2);
  }
  return &collection->items[collection->num_items++];
}

void test_collection_delete_item(TestCollection *collection)
{
  collection->num_items--;
}

//-----------------------------------------------------------------------------

void test_data_entry_init(TestEntry *entry)
{
  test_collection_init(&entry->transitions);
  test_collection_init(&entry->samples);
}

void test_data_entry_clear(TestEntry *entry)
{
  test_collection_clear(&entry->samples);
  test_collection_clear(&entry->transitions);
}

//-----------------------------------------------------------------------------

void test_data_init(TestData *data)
{
  data->num_entries = 0;
  data->capacity = 0;
  data->entries = NULL;
  test_data_resize(data, 10);
}

void test_data_clear(TestData *data)
{
  for (int i = data->num_entries - 1; i >= 0; i--) {
    test_data_entry_clear(&data->entries[i]);
  }
  free(data->entries);

  data->num_entries = 0;
  data->capacity = 0;
  data->entries = NULL;
}

void test_data_resize(TestData *data, int newsize)
{
  TestEntry *newentries = realloc(data->entries, sizeof(TestEntry) * newsize);
  if (newentries == NULL) {
    fprintf(stderr, "test_data_resize(): realloc failure\n");
    exit(1);
  }
  data->entries = newentries;
  data->capacity = newsize;
}

TestEntry *test_data_new_entry(TestData *data)
{
  if (data->num_entries >= data->capacity) {
    test_data_resize(data, data->capacity * 2);
  }
  TestEntry *entry = &data->entries[data->num_entries++];
  test_data_entry_init(entry);
  return entry;
}

void test_data_delete_entry(TestData *data)
{
  data->num_entries--;
  TestEntry *entry = &data->entries[data->num_entries];
  test_data_entry_clear(entry);
}

//-----------------------------------------------------------------------------

/*
static int compare_test_item(const void *a, const void *b)
{
  const TestItem *ta = a;
  const TestItem *tb = b;
  if (ta->epoch_seconds < tb->epoch_seconds) return -1;
  if (ta->epoch_seconds > tb->epoch_seconds) return 1;
  return 0;
}

void test_collection_sort_items(TestCollection *test_data)
{
  for (int i = 0; i < test_data->num_entries; i++) {
    TestCollection *collection = &test_data->entries[i];
    qsort(
        entry->items,
        entry->num_items,
        sizeof(TestItem),
        compare_test_item);
  }
}
*/

static void print_item(const char *indent, const TestItem *item)
{
  printf("%s\"epoch\": %ld,\n", indent, item->epoch_seconds);
  printf("%s\"total_offset\": %d,\n", indent, item->utc_offset);
  printf("%s\"dst_offset\": %d,\n", indent, item->dst_offset);
  printf("%s\"y\": %d,\n", indent, item->year);
  printf("%s\"M\": %d,\n", indent, item->month);
  printf("%s\"d\": %d,\n", indent, item->day);
  printf("%s\"h\": %d,\n", indent, item->hour);
  printf("%s\"m\": %d,\n", indent, item->minute);
  printf("%s\"s\": %d,\n", indent, item->second);
  printf("%s\"abbrev\": \"%s\",\n", indent, item->abbrev);
  printf("%s\"type\": \"%c\"\n", indent, item->type);
}

void print_json(
  const TestData *test_data,
  int start_year,
  int until_year,
  int epoch_year,
  const char *version,
  const char *tz_version)
{

  const char indent0[] = "  ";
  const char indent1[] = "    ";
  const char indent2[] = "      ";
  const char indent3[] = "        ";
  const char indent4[] = "          ";

  printf("{\n");
  printf("%s\"start_year\": %d,\n", indent0, start_year);
  printf("%s\"until_year\": %d,\n", indent0, until_year);
  printf("%s\"epoch_year\": %d,\n", indent0, epoch_year);
  printf("%s\"source\": \"acetimec\",\n", indent0);
  printf("%s\"version\": \"%s\",\n", indent0, version);
  printf("%s\"tz_version\": \"%s\",\n", indent0, tz_version);
  printf("%s\"has_valid_abbrev\": true,\n", indent0);
  printf("%s\"has_valid_dst\": true,\n", indent0);
  printf("%s\"test_data\": {\n", indent0);

  // Print each zone
  int num_zones = test_data->num_entries;
  for (int z = 0; z < num_zones; z++) {
    const TestEntry *entry = &test_data->entries[z];
    const char *zone_name = entry->zone_name;
    printf("%s\"%s\": {\n", indent1, zone_name);

    // Print transitions
    printf("%s\"%s\": [\n", indent2, "transitions");
    const TestCollection *collection = &entry->transitions;
    for (int i = 0; i < collection->num_items; i++) {
      const TestItem *item = &collection->items[i];
      printf("%s{\n", indent3);
      print_item(indent4, item);
      printf("%s}%s\n", indent3, (i < collection->num_items - 1) ? "," : "");
    }
    printf("%s],\n", indent2);

    // Print samples
    printf("%s\"%s\": [\n", indent2, "samples");
    collection = &entry->samples;
    for (int i = 0; i < collection->num_items; i++) {
      const TestItem *item = &collection->items[i];
      printf("%s{\n", indent3);
      print_item(indent4, item);
      printf("%s}%s\n", indent3, (i < collection->num_items - 1) ? "," : "");
    }
    printf("%s]\n", indent2);

    printf("%s}%s\n", indent1, (z < test_data->num_entries - 1) ? "," : "");
  }

  printf("%s}\n", indent0);
  printf("}\n");
}
