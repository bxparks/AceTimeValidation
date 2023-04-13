/*
 * Generate the validation JSON output for the zones given on the STDIN. The
 * transition time and UTC offsets are calculated using the AceTimeC library
 * (https://github.com/bxparks/AceTimeC).
 *
 * Usage:
 * $ ./compare_acetimec.out
 *    --start_year start
 *    --until_year until
 *    --epoch_year year
 *    < zones.txt
 *    > validation_data.json
 */

#include <stdbool.h>
#include <stdlib.h> // exit(), qsort()
#include <string.h> // strcmp(), strncmp()
#include <stdio.h> // printf(), fprintf()
#include <acetimec.h>
#include "test_data.h"
#include "sampling.h"

// Command line arguments
int16_t start_year = 2000;
int16_t until_year = 2100;
int16_t epoch_year = 2050;

AtcZoneRegistrar registrar;

/** Insert TestItems for the given 'zone_name' into test_data. */
int8_t process_zone(
    struct AtcZoneProcessor *processor,
    struct TestData *test_data,
    const char *zone_name)
{
  const struct AtcZoneInfo *zone_info = atc_registrar_find_by_name(
      &registrar, zone_name);
  if (zone_info == NULL) {
    fprintf(stderr, "ERROR: Zone %s: not found\n", zone_name);
    return kAtcErrGeneric;
  }

  AtcTimeZone tz = {zone_info, processor};
  struct TestDataEntry *entry = test_data_next_entry(test_data);
  add_transitions(
      entry,
      zone_name,
      &tz,
      start_year,
      until_year);
  add_monthly_samples(
      entry,
      zone_name,
      &tz,
      start_year,
      until_year);
  return kAtcErrOk;
}

/**
 * Read the list of zones from the 'zones.txt' in the stdin. Ignore blank lines
 * and comments (starting with '#'), and process each zone, one per line.
 */
int8_t read_and_process_zone(
    struct AtcZoneProcessor *processor,
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

    int8_t err = process_zone(processor, test_data, word);
    if (err) {
      fprintf(stderr, "Error processor zone '%s'\n", word);
      return err;
    }
  }

  return kAtcErrOk;
}

//-----------------------------------------------------------------------------

void usage_and_exit() {
  fprintf(stderr,
    "Usage: compare_acetimec.out [--install_dir {dir}]\n"
    "   --start_year start --until_year until --epoch_year year\n"
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
  const char *start = "";
  const char *until = "";
  const char *epoch = "";

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
    } else if (argEquals(argv[0], "--epoch_year")) {
      SHIFT(argc, argv);
      if (argc == 0) usage_and_exit();
      epoch = argv[0];
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

  if (strlen(start) == 0) {
    fprintf(stderr, "Required flag: --start_year\n");
    usage_and_exit();
  }
  if (strlen(until) == 0) {
    fprintf(stderr, "Required flag: --until_year\n");
    usage_and_exit();
  }
  if (strlen(epoch) == 0) {
    fprintf(stderr, "Required flag: --epoch_year\n");
    usage_and_exit();
  }

  start_year = atoi(start);
  until_year = atoi(until);
  epoch_year = atoi(epoch);

  // Configure the current epoch year.
  atc_set_current_epoch_year(epoch_year);

  // Set up registry.
  atc_registrar_init(
      &registrar,
      kAtcZoneAndLinkRegistry,
      kAtcZoneAndLinkRegistrySize);

  // Initialize an AtcZoneProcessor instance.
  struct AtcZoneProcessor processor;
  atc_processor_init(&processor);

  // Process the zones on the STDIN.
  struct TestData test_data;
  test_data_init(&test_data);
  int8_t err = read_and_process_zone(&processor, &test_data);
  if (err) exit(1);

  // Sort test items by epoch seconds, and print the JSON.
  sort_test_data(&test_data);
  print_json(&test_data, start_year, until_year, epoch_year,
    ACE_TIME_C_VERSION_STRING, kAtcZoneContext.tz_version);

  // Cleanup
  test_data_free(&test_data);

  return 0;
}
