/*
 * Generate the validation JSON output for the zones given on the STDIN. The
 * transition time and UTC offsets are calculated using the C libc library.
 *
 * Usage:
 * $ ./generate_data.out
 *    --start_year start
 *    --until_year until
 *    --epoch_year year
 *    < zones.txt
 *    > validation_data.json
 */

#include <stdlib.h> // exit(), atoi(), setenv()
#include <string.h> // strcmp(), strncmp()
#include <stdio.h> // printf(), fprintf(), stdin
#include <stdint.h> // uint8_t
#include <stdbool.h> // bool
#include <time.h> // tzset()

//gnu_get_libc_version(), https://stackoverflow.com/questions/9705660
#include <gnu/libc-version.h>

#include "test_data.h"
#include "sampling.h"

#define MAX_LINE_SIZE 512
#define SAMPLING_INTERVAL_HOURS 22

//-----------------------------------------------------------------------------

/** Shift the command line arguments to the left by one position. */
#define SHIFT(argc, argv) \
do { \
  argc--; \
  argv++; \
} while (0)

/** Print usage and exit with status code. (0 means success). */
static void usage_and_exit(int status)
{
  fprintf(stderr,
    "Usage: generate_data.out [--help]\n"
    "   --start_year start --until_year until --epoch_year year\n"
    "   < zones.txt > validation_data.json\n"
  );
  exit(status);
}

/** Compare 2 C-strings. */
static bool arg_equals(const char* s, const char* t)
{
  return strcmp(s, t) == 0;
}

/** User-defined option parameters. */
int start_year = 0;
int until_year = 0;
int epoch_year = 0;

/**
 * Parse command line flags.
 * Returns the index of the first argument after the flags.
 */
static int parse_flags(int argc, char **argv)
{
  int argc_original = argc;
  SHIFT(argc, argv);

  const char *start = "";
  const char *until = "";
  const char *epoch = "";
  while (argc > 0) {
    if (arg_equals(argv[0], "--start_year")) {
      SHIFT(argc, argv);
      if (argc == 0) usage_and_exit(1);
      start = argv[0];
    } else if (arg_equals(argv[0], "--until_year")) {
      SHIFT(argc, argv);
      if (argc == 0) usage_and_exit(1);
      until = argv[0];
    } else if (arg_equals(argv[0], "--epoch_year")) {
      SHIFT(argc, argv);
      if (argc == 0) usage_and_exit(1);
      epoch = argv[0];
    } else if (arg_equals(argv[0], "--")) {
      SHIFT(argc, argv);
      break;
    } else if (arg_equals(argv[0], "--help")) {
      usage_and_exit(0);
      break;
    } else if (argv[0][0] == '-') {
      fprintf(stderr, "Unknonwn flag '%s'\n", argv[0]);
      usage_and_exit(1);
    } else {
      break;
    }
    SHIFT(argc, argv);
  }

  if (strlen(start) == 0) {
    fprintf(stderr, "Required flag: --start_year\n");
    usage_and_exit(1);
  }
  if (strlen(until) == 0) {
    fprintf(stderr, "Required flag: --until_year\n");
    usage_and_exit(1);
  }
  if (strlen(epoch) == 0) {
    fprintf(stderr, "Required flag: --epoch_year\n");
    usage_and_exit(1);
  }

  start_year = atoi(start);
  until_year = atoi(until);
  epoch_year = atoi(epoch);

  return argc_original - argc;
}

//-----------------------------------------------------------------------------

/**
 * Set the timezone as currently specified by the "TZ" environment variable.
 * Returns 0 if the time zone is valid, 1 otherwise.
 */
uint8_t set_time_zone(const char *zone_name) {
  setenv("TZ", zone_name, 1 /*overwrite*/);

  // Update the following **global** variables:
  //
  // * tzname[0]: the abbreviation in standard time
  // * tzname[1]: the abbreviation in DST time
  //
  // If -D _XOPEN_SOURCE is set, these globals are set:
  //
  // * timezone: the UTC offset in seconds with the opposite sign
  // * daylight: is 1 if the time zone has ever had daylight savings
  // (What a mess.)
  tzset();

  // tzset() does not set an error status, so we don't know if the ZONE_NAME is
  // valid or not. So we use the following heuristics: If the zone does not
  // exist, then tzset() will set the zone to UTC, so daylight offset will be
  // 0. But there are legitimate timezones which track UTC. But when the zone
  // is invalid, it seems like tzname[0] is set to a truncated version of the
  // original zone name, and tzname[1] is set to an empty string.
  bool invalid = strncmp(tzname[0], zone_name, strlen(tzname[0])) == 0
      && tzname[1][0] == '\0';

  return invalid;
}

uint8_t process_zone(struct TestData *test_data, int i, const char *zone_name)
{
  fprintf(stderr, "[%d] Zone %s\n", i, zone_name);
  uint8_t err = set_time_zone(zone_name);
  if (err) {
    fprintf(stderr, "\tERROR: Not found\n");
    return err;
  }

  struct TestDataEntry *entry = test_data_next_entry(test_data);
  test_data_entry_init(entry);
  strncpy(entry->zone_name, zone_name, ZONE_NAME_SIZE - 1);
  entry->zone_name[ZONE_NAME_SIZE - 1] = '\0';

  add_transitions(entry, zone_name, start_year, until_year,
    SAMPLING_INTERVAL_HOURS);
  add_monthly_samples(entry, zone_name, start_year, until_year);
  test_data_entry_sort_items(entry);

  return 0;
}

// Read zones from the stdin.
uint8_t process_zones(struct TestData *test_data)
{
  char line[MAX_LINE_SIZE];
  int i = 0;
  for (;;) {
    // fgets() always NUL-terminates
    const char *s = fgets(line, MAX_LINE_SIZE, stdin);
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

    int8_t err = process_zone(test_data, i, line);
    if (err) return err;
    i++;
  }

  return 0;
}

//-----------------------------------------------------------------------------

int main(int argc, char **argv )
{
  int args = parse_flags(argc, argv);
  set_ace_time_epoch_year(epoch_year);

  struct TestData test_data;
  test_data_init(&test_data);
  int8_t err = process_zones(&test_data);
  if (err) {
    fprintf(stderr, "ERROR: code %d\n", err);
    exit(1);
  }

  print_json(
    &test_data, start_year, until_year, epoch_year,
    "libc" /*source*/,
    gnu_get_libc_version() /*version*/,
    "2022g?" /*tz_version*/);

  test_data_free(&test_data);

  return 0;
}
