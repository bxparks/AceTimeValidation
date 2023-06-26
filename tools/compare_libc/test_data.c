#include <stdio.h>
#include <stdlib.h> // realloc(), qsort()
#include "test_data.h"

void test_collection_init(struct TestCollection *collection)
{
  collection->num_items = 0;
  collection->capacity = 0;
  collection->items = NULL;
  test_collection_resize(collection, 10);
}

void test_collection_clear(struct TestCollection *collection)
{
  free(collection->items);
  collection->num_items = 0;
  collection->capacity = 0;
  collection->items = NULL;
}

void test_collection_resize(struct TestCollection *collection, int newsize)
{
  struct TestItem *newitems = realloc(
      collection->items,
      sizeof(struct TestItem) * newsize);
  if (newitems == NULL) {
    fprintf(stderr, "test_collection_resize(): realloc failure\n");
    exit(1);
  }
  collection->items = newitems;
  collection->capacity = newsize;
}

struct TestItem *test_collection_new_item(struct TestCollection *collection)
{
  if (collection->num_items >= collection->capacity) {
    test_collection_resize(collection, collection->capacity * 2);
  }
  return &collection->items[collection->num_items++];
}

void test_collection_delete_item(struct TestCollection *collection)
{
  collection->num_items--;
}

static int compare_test_item(const void *a, const void *b)
{
  const struct TestItem *ta = a;
  const struct TestItem *tb = b;
  if (ta->epoch_seconds < tb->epoch_seconds) return -1;
  if (ta->epoch_seconds > tb->epoch_seconds) return 1;
  return 0;
}

void test_collection_sort_items(struct TestCollection *collection)
{
  qsort(
    collection->items,
    collection->num_items,
    sizeof(struct TestItem),
    compare_test_item);
}

//-----------------------------------------------------------------------------

void test_entry_init(struct TestEntry *entry)
{
  test_collection_init(&entry->transitions);
  test_collection_init(&entry->samples);
}

void test_entry_clear(struct TestEntry *entry)
{
  test_collection_clear(&entry->samples);
  test_collection_clear(&entry->transitions);
}

//-----------------------------------------------------------------------------

void test_data_init(struct TestData *data)
{
  data->num_entries = 0;
  data->capacity = 0;
  data->entries = NULL;
  test_data_resize(data, 10);
}

void test_data_clear(struct TestData *data)
{
  for (int i = data->num_entries - 1; i >= 0; i--) {
    test_entry_clear(&data->entries[i]);
  }
  free(data->entries);

  data->num_entries = 0;
  data->capacity = 0;
  data->entries = NULL;
}

void test_data_resize(struct TestData *data, int newsize)
{
  struct TestEntry *newentries = realloc(
      data->entries,
      sizeof(struct TestEntry) * newsize);
  if (newentries == NULL) {
    fprintf(stderr, "test_entry_resize(): realloc failure\n");
    exit(1);
  }
  data->entries = newentries;
  data->capacity = newsize;
}

struct TestEntry *test_data_new_entry(struct TestData *data)
{
  if (data->num_entries >= data->capacity) {
    test_data_resize(data, data->capacity * 2);
  }
  struct TestEntry *entry = &data->entries[data->num_entries++];
  test_entry_init(entry);
  return entry;
}

void test_data_delete_entry(struct TestData *data)
{
  data->num_entries--;
  struct TestEntry *entry = &data->entries[data->num_entries];
  test_entry_clear(entry);
}

//-----------------------------------------------------------------------------

static void print_item(const char *indent, const struct TestItem *item)
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
  const struct TestData *test_data,
  int start_year,
  int until_year,
  int epoch_year,
  const char *source,
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
  printf("%s\"scope\": \"complete\",\n", indent0);
  printf("%s\"source\": \"%s\",\n", indent0, source);
  printf("%s\"version\": \"%s\",\n", indent0, version);
  printf("%s\"tz_version\": \"%s\",\n", indent0, tz_version);
  printf("%s\"has_valid_abbrev\": true,\n", indent0);
  printf("%s\"has_valid_dst\": false,\n", indent0);
  printf("%s\"offset_granularity\": 1,\n", indent0);
  printf("%s\"test_data\": {\n", indent0);

  // Print each zone
  int num_zones = test_data->num_entries;
  for (int z = 0; z < num_zones; z++) {
    const struct TestEntry *entry = &test_data->entries[z];
    const char *zone_name = entry->zone_name;
    printf("%s\"%s\": {\n", indent1, zone_name);

    // Print transitions
    printf("%s\"%s\": [\n", indent2, "transitions");
    const struct TestCollection *collection = &entry->transitions;
    for (int i = 0; i < collection->num_items; i++) {
      const struct TestItem *item = &collection->items[i];
      printf("%s{\n", indent3);
      print_item(indent4, item);
      printf("%s}%s\n", indent3, (i < collection->num_items - 1) ? "," : "");
    }
    printf("%s],\n", indent2);

    // Print samples
    printf("%s\"%s\": [\n", indent2, "samples");
    collection = &entry->samples;
    for (int i = 0; i < collection->num_items; i++) {
      const struct TestItem *item = &collection->items[i];
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
