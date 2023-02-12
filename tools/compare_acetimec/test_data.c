#include <stdio.h>
#include <stdlib.h> // realloarray()
#include "test_data.h"

void test_data_entry_init(struct TestDataEntry *entry)
{
  entry->num_items = 0;
  entry->capacity = 0;
  entry->items = NULL;
  test_data_entry_resize_items(entry, 10);
}

void test_data_entry_free(struct TestDataEntry *entry)
{
  free(entry->items);
  entry->num_items = 0;
  entry->capacity = 0;
  entry->items = NULL;
}

void test_data_entry_resize_items(struct TestDataEntry *entry, int newsize)
{
  struct TestItem *newitems = realloc(
      entry->items,
      sizeof(struct TestItem) * newsize);
  if (newitems == NULL) {
    fprintf(stderr, "test_data_entry_resize_items(): realloc failure\n");
    exit(1);
  }
  entry->items = newitems;
  entry->capacity = newsize;
}

struct TestItem *test_data_entry_next_item(struct TestDataEntry *entry)
{
  if (entry->num_items >= entry->capacity) {
    test_data_entry_resize_items(entry, entry->capacity * 2);
  }
  return &entry->items[entry->num_items++];
}

void test_data_entry_free_item(struct TestDataEntry *entry)
{
  entry->num_items--;
}

//-----------------------------------------------------------------------------

void test_data_init(struct TestData *data)
{
  data->num_entries = 0;
  data->capacity = 0;
  data->entries = NULL;
  test_data_resize_entries(data, 10);
}

void test_data_free(struct TestData *data)
{
  for (int i = data->num_entries - 1; i >= 0; i--) {
    test_data_entry_free(&data->entries[i]);
  }
  free(data->entries);

  data->num_entries = 0;
  data->capacity = 0;
  data->entries = NULL;
}

void test_data_resize_entries(struct TestData *data, int newsize)
{
  struct TestDataEntry *newentries = realloc(
      data->entries,
      sizeof(struct TestDataEntry) * newsize);
  if (newentries == NULL) {
    fprintf(stderr, "test_data_entry_resize_entries(): realloc failure\n");
    exit(1);
  }
  data->entries = newentries;
  data->capacity = newsize;
}

struct TestDataEntry *test_data_next_entry(struct TestData *data)
{
  if (data->num_entries >= data->capacity) {
    test_data_resize_entries(data, data->capacity * 2);
  }
  struct TestDataEntry *entry = &data->entries[data->num_entries++];
  test_data_entry_init(entry);
  return entry;
}

void test_data_free_entry(struct TestData *data)
{
  data->num_entries--;
}

//-----------------------------------------------------------------------------

static int compare_test_item(const void *a, const void *b)
{
  const struct TestItem *ta = a;
  const struct TestItem *tb = b;
  if (ta->epoch_seconds < tb->epoch_seconds) return -1;
  if (ta->epoch_seconds > tb->epoch_seconds) return 1;
  return 0;
}

void sort_test_data(struct TestData *test_data)
{
  for (int i = 0; i < test_data->num_entries; i++) {
    struct TestDataEntry *entry = &test_data->entries[i];
    qsort(
        entry->items,
        entry->num_items,
        sizeof(struct TestItem),
        compare_test_item);
  }
}

void print_json(
  const struct TestData *test_data,
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

  printf("{\n");
  printf("%s\"start_year\": %d,\n", indent0, start_year);
  printf("%s\"until_year\": %d,\n", indent0, until_year);
  printf("%s\"epoch_year\": %d,\n", indent0, epoch_year);
  printf("%s\"source\": \"AceTimeC\",\n", indent0);
  printf("%s\"version\": \"%s\",\n", indent0, version);
  printf("%s\"tz_version\": \"%s\",\n", indent0, tz_version);
  printf("%s\"has_valid_abbrev\": true,\n", indent0);
  printf("%s\"has_valid_dst\": true,\n", indent0);
  printf("%s\"test_data\": {\n", indent0);

  // Print each zone
  int num_zones = test_data->num_entries;
  for (int z = 0; z < num_zones; z++) {
    const struct TestDataEntry *entry = &test_data->entries[z];
    const char *zone_name = entry->zone_name;
    printf("%s\"%s\": [\n", indent1, zone_name);

    // Print each testItem
    for (int i = 0; i < entry->num_items; i++) {
      const struct TestItem *item = &entry->items[i];
      printf("%s{\n", indent2);
      {
        printf("%s\"epoch\": %ld,\n", indent3, item->epoch_seconds);
        printf("%s\"total_offset\": %d,\n", indent3, item->utc_offset);
        printf("%s\"dst_offset\": %d,\n", indent3, item->dst_offset);
        printf("%s\"y\": %d,\n", indent3, item->year);
        printf("%s\"M\": %d,\n", indent3, item->month);
        printf("%s\"d\": %d,\n", indent3, item->day);
        printf("%s\"h\": %d,\n", indent3, item->hour);
        printf("%s\"m\": %d,\n", indent3, item->minute);
        printf("%s\"s\": %d,\n", indent3, item->second);
        printf("%s\"abbrev\": \"%s\",\n", indent3, item->abbrev);
        printf("%s\"type\": \"%c\"\n", indent3, item->type);
      }
      printf("%s}%s\n", indent2, (i < entry->num_items - 1) ? "," : "");
    }

    printf("%s]%s\n", indent1, (z < test_data->num_entries - 1) ? "," : "");
  }

  printf("%s}\n", indent0);
  printf("}\n");
}
