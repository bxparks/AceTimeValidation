/*
 * Generate the validation JSON output for the zones given on the STDIN. The
 * transition time and UTC offsets are calculated using the AceTime library
 * (https://github.com/bxparks/AceTime).
 *
 * Usage:
 * $ ./compare_acetime.out
 *    --start_year start
 *    --until_year until
 *    --epoch_year year
 *    < zones.txt
 *    > validation_data.json
 */

#include <string.h> // strcmp()
#include <stdio.h> // fgets()
#include <Arduino.h>
#include <AceTime.h>
#include "test_data.h"
#include "sampling.h"

using namespace ace_time;

// Command line arguments
int16_t startYear = 2000;
int16_t untilYear = 2100;
int16_t epochYear = 2050;

const int kScopeTypeBasic = 0;
const int kScopeTypeExtended = 1;
const int kScopeTypeComplete = 2;
int scopeType = kScopeTypeExtended;
const char *scopeString = "extended";

// Cache and buffers for AceTime
constexpr uint8_t CACHE_SIZE = 2;

BasicZoneProcessorCache<CACHE_SIZE> basicZoneProcessorCache;
BasicZoneManager basicZoneManager(
    zonedb::kZoneAndLinkRegistrySize,
    zonedb::kZoneAndLinkRegistry,
    basicZoneProcessorCache);

ExtendedZoneProcessorCache<CACHE_SIZE> extendedZoneProcessorCache;
ExtendedZoneManager extendedZoneManager(
    zonedbx::kZoneAndLinkRegistrySize,
    zonedbx::kZoneAndLinkRegistry,
    extendedZoneProcessorCache);

CompleteZoneProcessorCache<CACHE_SIZE> completeZoneProcessorCache;
CompleteZoneManager completeZoneManager(
    zonedbc::kZoneAndLinkRegistrySize,
    zonedbc::kZoneAndLinkRegistry,
    completeZoneProcessorCache);

//-----------------------------------------------------------------------------

/** Insert TestItems for the given 'zoneName' into test_data. */
int8_t processZone(TestData *testData, int i, const char *zoneName) {
  fprintf(stderr, "[%d] Zone %s", i, zoneName);
  TimeZone tz;
  if (scopeType == kScopeTypeBasic) {
    tz = basicZoneManager.createForZoneName(zoneName);
  } else if (scopeType == kScopeTypeExtended) {
    tz = extendedZoneManager.createForZoneName(zoneName);
  } else if (scopeType == kScopeTypeComplete) {
    tz = completeZoneManager.createForZoneName(zoneName);
  }

  // Create entry for a single zone
  TestEntry *entry = testDataNewEntry(testData);
  strncpy(entry->zone_name, zoneName, ZONE_NAME_SIZE - 1);
  entry->zone_name[ZONE_NAME_SIZE - 1] = '\0';

  // If zone is not supported, retain the empty entry.
  if (tz.isError()) {
    fprintf(stderr, ": not found\n");
    return 1;
  } else {
    fprintf(stderr, "\n");
  }

  // Number of seconds to add to unix seconds to get the requested epoch
  // seconds.
  int64_t epochOffset = - LocalDate::forComponents(epochYear, 1, 1)
      .toUnixSeconds64();

  addTransitions(
      &entry->transitions,
      zoneName,
      tz,
      startYear,
      untilYear,
      epochOffset);

  addMonthlySamples(
      &entry->samples,
      zoneName,
      tz,
      startYear,
      untilYear,
      epochOffset);

  return 0;
}

/**
 * Read the list of zones from the 'zones.txt' in the stdin. Ignore blank lines
 * and comments (starting with '#'), and process each zone, one per line.
 */
int8_t readAndProcessZones(TestData *testData) {
  char line[MAX_LINE_SIZE];
  int i = 0;
  while (true) {
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

    processZone(testData, i, line);
    i++;
  }

  return 0;
}

//-----------------------------------------------------------------------------

void usageAndExit() {
  fprintf(stderr,
    "Usage: compare_acetime.out\n"
    "   --start_year start --until_year until --epoch_year year\n"
    "   < zones.txt\n");
  exit(1);
}

static void shift(int& argc, const char* const*& argv) {
  argc--;
  argv++;
}

static bool argEquals(const char *s, const char *t) {
  return strcmp(s, t) == 0;
}

void processCommandLine(int argc, const char* const* argv) {
  // Parse command line flags.
  const char *start = "";
  const char *until = "";
  const char *epoch = "";
  const char *scope = "";

  shift(argc, argv);
  while (argc > 0) {
    if (argEquals(argv[0], "--start_year")) {
      shift(argc, argv);
      if (argc == 0) usageAndExit();
      start = argv[0];
    } else if (argEquals(argv[0], "--until_year")) {
      shift(argc, argv);
      if (argc == 0) usageAndExit();
      until = argv[0];
    } else if (argEquals(argv[0], "--epoch_year")) {
      shift(argc, argv);
      if (argc == 0) usageAndExit();
      epoch = argv[0];
    } else if (argEquals(argv[0], "--scope")) {
      shift(argc, argv);
      if (argc == 0) usageAndExit();
      scope = argv[0];
    } else if (argEquals(argv[0], "--")) {
      shift(argc, argv);
      break;
    } else if (strncmp(argv[0], "-", 1) == 0) {
      fprintf(stderr, "Unknonwn flag '%s'\n", argv[0]);
      usageAndExit();
    } else {
      break;
    }
    shift(argc, argv);
  }

  if (strlen(start) == 0) {
    fprintf(stderr, "Required flag: --start_year\n");
    usageAndExit();
  }
  if (strlen(until) == 0) {
    fprintf(stderr, "Required flag: --until_year\n");
    usageAndExit();
  }
  if (strlen(epoch) == 0) {
    fprintf(stderr, "Required flag: --epoch_year\n");
    usageAndExit();
  }
  if (strlen(scope) == 0) {
    fprintf(stderr, "Required flag: --scope\n");
    usageAndExit();
  }

  startYear = atoi(start);
  untilYear = atoi(until);
  epochYear = atoi(epoch);
  if (strcmp(scope, "basic") == 0) {
    scopeType = kScopeTypeBasic;
    scopeString = scope;
  } else if (strcmp(scope, "extended") == 0) {
    scopeType = kScopeTypeExtended;
    scopeString = scope;
  } else if (strcmp(scope, "complete") == 0) {
    scopeType = kScopeTypeComplete;
    scopeString = scope;
  } else {
    fprintf(stderr, "Invalid --scope: %s\n", scope);
    usageAndExit();
  }

  // Configure the current epoch year.
  Epoch::currentEpochYear(epochYear);

  // Process the zones on the STDIN.
  TestData testData;
  testDataInit(&testData);
  int8_t err = readAndProcessZones(&testData);
  if (err) exit(1);

  printJson(&testData, startYear, untilYear, epochYear, scope,
    ACE_TIME_VERSION_STRING, zonedbx::kZoneContext.tzVersion);

  // Cleanup
  testDataClear(&testData);
}

//-----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // some boards reboot twice
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // For Leonardo/Micro

#if defined(EPOXY_DUINO)
  processCommandLine(epoxy_argc, epoxy_argv);
  exit(0);
#endif
}

void loop() {}
