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

void test_data_entry_pushback_item(struct TestDataEntry *entry)
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

void test_data_pushback_entry(struct TestData *data)
{
  data->num_entries--;
}
