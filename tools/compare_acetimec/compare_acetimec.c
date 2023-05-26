/*
 * Generate the validation JSON output for the zones given on the STDIN. The
 * transition time and UTC offsets are calculated using the acetimec library
 * (https://github.com/bxparks/acetimec).
 *
 * Usage:
 * $ ./compare_acetimec.out
 *    --start_year start
 *    --until_year until
 *    --epoch_year year
 *    --zonedb (zonedb|zonedball)
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
const char *zonedb = NULL; // "zonedb", "zonedball"

AtcZoneRegistrar registrar;

/** Insert TestItems for the given 'zone_name' into test_data. */
int8_t process_zone(
    AtcZoneProcessor *processor,
    TestData *test_data,
    int i,
    const char *zone_name)
{
  fprintf(stderr, "[%d] Zone %s\n", i, zone_name);
  const AtcZoneInfo *zone_info = atc_registrar_find_by_name(
      &registrar, zone_name);
  if (zone_info == NULL) {
    fprintf(stderr, "ERROR: Zone %s: not found\n", zone_name);
    return kAtcErrGeneric;
  }

  AtcTimeZone tz = {zone_info, processor};

  // Create entry for a single zone
  TestEntry *entry = test_data_new_entry(test_data);
  strncpy(entry->zone_name, zone_name, ZONE_NAME_SIZE - 1);
  entry->zone_name[ZONE_NAME_SIZE - 1] = '\0';

  // Number of seconds to add to unix seconds to get the requested epoch
  // seconds.
  int64_t epoch_offset = -(int64_t) 86400
      * atc_local_date_to_unix_days(epoch_year, 1, 1);

  add_transitions(
      &entry->transitions,
      zone_name,
      &tz,
      start_year,
      until_year,
      epoch_offset);

  add_monthly_samples(
      &entry->samples,
      zone_name,
      &tz,
      start_year,
      until_year,
      epoch_offset);

  return kAtcErrOk;
}

/**
 * Read the list of zones from the 'zones.txt' in the stdin. Ignore blank lines
 * and comments (starting with '#'), and process each zone, one per line.
 */
int8_t process_zones(AtcZoneProcessor *processor, TestData *test_data) {
  char line[MAX_LINE_SIZE];
  int i = 0;
  while (1) {
    // fgets() always NUL-terminates
    char *s = fgets(line, MAX_LINE_SIZE, stdin);
    if (s == NULL) break;

    size_t len = strlen(s);
    if (len == 0) break;

    // Trim trailing newline if any.
    if (line[len - 1] == '\n') {
      line[len - 1] = '\0';
      len--;
    }

    // Skip over blank lines
    if (line[0] == '\0') continue;

    // Skip over comments
    if (line[0] == '#') continue;

    int8_t err = process_zone(processor, test_data, i, line);
    if (err) {
      fprintf(stderr, "Error processor zone '%s'\n", line);
      return err;
    }
    i++;
  }

  return kAtcErrOk;
}

//-----------------------------------------------------------------------------

void usage_and_exit() {
  fprintf(stderr,
    "Usage: compare_acetimec.out [--install_dir {dir}]\n"
    "   --start_year start --until_year until --epoch_year year\n"
    "   --zonedb (zonedb|zonedball)\n"
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
  const char *db = "";

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
    } else if (argEquals(argv[0], "--zonedb")) {
      SHIFT(argc, argv);
      if (argc == 0) usage_and_exit();
      db = argv[0];
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
  if (strlen(db) == 0) {
    fprintf(stderr, "Required flag: --zonedb\n");
    usage_and_exit();
  }

  start_year = atoi(start);
  until_year = atoi(until);
  epoch_year = atoi(epoch);
  zonedb = db;

  // Configure the current epoch year.
  atc_set_current_epoch_year(epoch_year);

  // Set up registry.
  if (strcmp(zonedb, "zonedb") == 0) {
    atc_registrar_init(
        &registrar,
        kAtcZoneAndLinkRegistry,
        kAtcZoneAndLinkRegistrySize);
  } else if (strcmp(zonedb, "zonedball") == 0) {
    atc_registrar_init(
        &registrar,
        kAtcAllZoneAndLinkRegistry,
        kAtcAllZoneAndLinkRegistrySize);
  } else {
    fprintf(stderr, "Invalid zonedb '%s'\n", zonedb);
    usage_and_exit();
  }

  // Initialize an AtcZoneProcessor instance.
  AtcZoneProcessor processor;
  atc_processor_init(&processor);

  // Process the zones on the STDIN.
  TestData test_data;
  test_data_init(&test_data);
  int8_t err = process_zones(&processor, &test_data);
  if (err) exit(1);

  print_json(&test_data, start_year, until_year, epoch_year,
    ACE_TIME_C_VERSION_STRING, kAtcZoneContext.tz_version);

  // Cleanup
  test_data_clear(&test_data);

  return 0;
}
