/*
 * Generate the validation JSON output for the zones given on the STDIN. The
 * transition time and UTC offsets are calculated using Howard Hinnant's date.h
 * and tz.h library. The Hinnant date library requires the --tz_version flag
 * even though we don't need it here.
 *
 * Usage:
 * $ ./generate_data.out
 *    [--tz_version {version}]
 *    [--start_year start]
 *    [--until_year until]
 *    < zones.txt
 *    > validation_data.json
 */

#include <stdlib.h> // exit()
#include <string.h> // strcmp(), strncmp()
#include <stdio.h> // printf(), fprintf()
#include <acetimec.h>
#include "test_data.h"
#include "val_data.h"

// Command line arguments
int16_t start_year = 2000;
int16_t until_year = 2050;

bool is_registry_sorted;

const char *tz_version = "";

//-----------------------------------------------------------------------------

#if 0
/** Sort the TestItems according to epochSeconds. */
void sortTestData(TestData& testData) {
  for (auto& p : testData) {
    sort(p.second.begin(), p.second.end(),
      [](const TestItem& a, const TestItem& b) {
        return a.epochSeconds < b.epochSeconds;
      }
    );
  }
}
#endif

/**
 * Generate the JSON output on STDOUT which will be redirect into
 * 'validation_data.json' file. Adopted from GenerateData.java.
 */
void print_json(const struct TestData *test_data) {
  const char indent0[] = "  ";
  const char indent1[] = "    ";
  const char indent2[] = "      ";
  const char indent3[] = "        ";

  printf("{\n");
  printf("%s\"start_year\": %d,\n", indent0, start_year);
  printf("%s\"until_year\": %d,\n", indent0, until_year);
  printf("%s\"source\": \"AceTimeC\",\n", indent0);
  printf("%s\"version\": \"%s\",\n", indent0, ACE_TIME_C_VERSION_STRING);
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

/** Insert TestItems for the given 'zone_name' into test_data. */
void process_zone(
    struct AtcZoneProcessing *processing,
    struct TestData *test_data,
    const char *zone_name)
{
  const struct AtcZoneInfo *zone_info = atc_registrar_find_by_name(
      kAtcZoneAndLinkRegistry,
      kAtcZoneAndLinkRegistrySize,
      zone_name,
      is_registry_sorted);

  if (zone_info == NULL) {
    fprintf(stderr, "Zone %s: NOT found\n", zone_name);
    return;
  }

  struct TestDataEntry *entry = test_data_next_entry(test_data);
  add_transitions(
      entry,
      zone_name,
      processing,
      zone_info,
      start_year,
      until_year);
  add_monthly_samples(
      entry,
      zone_name,
      processing,
      zone_info,
      start_year,
      until_year);
}

/**
 * Read the list of zones from the 'zones.txt' in the stdin. Ignore blank lines
 * and comments (starting with '#'), and process each zone, one per line.
 */
void read_and_process_zone(
    struct AtcZoneProcessing *processing,
    struct TestData *test_data)
{
  char line[MAX_LINE_LENGTH];
  while (1) {
    char *p = fgets(line, MAX_LINE_LENGTH, stdin);
    if (! p) break;
    const char *delim = " ";
    char *word = strtok(line, delim);
    // Strip trailing \n
    size_t len = strlen(word);
    if (len && word[len-1] == '\n') {
      word[len-1] = '\0';
    }
    if (strcmp(word, "") == 0) continue;
    if (word[0] == '#') continue;

    process_zone(processing, test_data, word);
  }
}

//-----------------------------------------------------------------------------

void usage_and_exit() {
  fprintf(stderr,
    "Usage: generate_data [--install_dir {dir}] [--tz_version {version}]\n"
    "   [--start_year start] [--until_year until]\n"
    "   < zones.txt\n");
  exit(1);
}

#define SHIFT(argc, argv) \
  do { \
    argc--; \
    argv++; \
  } while (0)

static bool argEquals(const char *s, const char *t) {
  return strcmp(s, t) == 0;
}

int main(int argc, const char* const* argv) {
  // Parse command line flags.
  const char *start = "2000";
  const char *until = "2050";

  SHIFT(argc, argv);
  while (argc > 0) {
    if (argEquals(argv[0], "--start_year")) {
      SHIFT(argc, argv);
      if (argc == 0) usage_and_exit();
      start = argv[0];
    } else if (argEquals(argv[0], "--until_year")) {
      SHIFT(argc, argv);
      if (argc == 0) usage_and_exit();
      until = argv[0];
    } else if (argEquals(argv[0], "--tz_version")) {
      SHIFT(argc, argv);
      if (argc == 0) usage_and_exit();
      tz_version = argv[0];
    } else if (argEquals(argv[0], "--")) {
      SHIFT(argc, argv);
      break;
    } else if (strncmp(argv[0], "-", 1) == 0) {
      fprintf(stderr, "Unknonwn flag '%s'\n", argv[0]);
      usage_and_exit();
    } else {
      break;
    }
    SHIFT(argc, argv);
  }

  start_year = atoi(start);
  until_year = atoi(until);

  // Set up registry
  is_registry_sorted = atc_registrar_is_registry_sorted(
      kAtcZoneAndLinkRegistry,
      kAtcZoneAndLinkRegistrySize);

  // Initialize val_data module.
  struct AtcZoneProcessing processing;
  atc_processing_init(&processing);

  // Process the zones on the STDIN
  fprintf(stderr, "Reading zones and generating validation data\n");
  struct TestData test_data;
  test_data_init(&test_data);
  read_and_process_zone(&processing, &test_data);
  //sortTestData(testData);

  fprintf(stderr, "Writing validation data\n");
  print_json(&test_data);

  fprintf(stderr, "Cleaning up\n");
  test_data_free(&test_data);

  fprintf(stderr, "Done\n");
  return 0;
}
