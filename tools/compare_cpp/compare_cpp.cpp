/*
 * Generate the validation JSON output for the zones given on the STDIN. The
 * transition time and UTC offsets are calculated using Howard Hinnant's date.h
 * and tz.h library. The Hinnant date library requires the --tz_version flag
 * even though we don't need it here.
 *
 * Usage:
 * $ ./generate_data.out
 *    [--install_dir {dir}]
 *    [--tz_version {version}]
 *    --start_year start
 *    --until_year until
 *    --epoch_year year
 *    < zones.txt
 *    > validation_data.json
 */

#include <iostream> // getline()
#include <map> // map<>
#include <vector> // vector<>
#include <algorithm> // sort()
#include <string.h> // strcmp(), strncmp()
#include <stdio.h> // printf(), fprintf()
#include <chrono>
#include <date/date.h>
#include <date/tz.h> // time_zone

using namespace date;
using namespace std::chrono;
using namespace std;

/** DateTime components. */
struct DateTime {
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
};

/**
 * A test item, containing the epochSeconds with its expected DateTime
 * components.
 */
struct TestItem {
  long epochSeconds;
  int utcOffset; // seconds
  int dstOffset; // seconds
  string abbrev;
  int year;
  unsigned month;
  unsigned day;
  int hour;
  int minute;
  int second;
  char type; //'A', 'B', 'a', 'b', 'S'
};

/** Collection of test items. */
typedef vector<TestItem> TestCollection;

/** Test data for a single zone. */
struct TestEntry {
  TestCollection transitions;
  TestCollection samples;
};

typedef map<string, TestEntry> TestData;

/**
 * Difference between Unix epoch (1970-01-01) and AceTime epoch (2000-01-01).
 */
long secondsToAceTimeEpochFromUnixEpoch = 946684800;

// Command line arguments
int startYear = 2000;
int untilYear = 2100;
int epochYear = 2050;

/**
 * Convert a zoned_time<> (which is an aggregation of time_zone and sys_time<>,
 * and sys_time<> is an alias for a std::chrono::time_point<>) into components.
 * See
 * https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes#components_to_time_point
 * which describes how to convert a time_point<> into components. (I don't know
 * why it has to be so complicated...)
 */
DateTime toDateTime(local_time<seconds> lt) {
  auto daypoint = floor<days>(lt);
  auto ymd = year_month_day(daypoint);
  auto tod = make_time(lt - daypoint);
  return DateTime{
    int(ymd.year()),
    unsigned(ymd.month()),
    unsigned(ymd.day()),
    (int)tod.hours().count(),
    (int)tod.minutes().count(),
    (int)tod.seconds().count()
  };
}

/**
 * Convert the Unix epoch seconds into a ZonedDateTime, then convert that into
 * TestItem that has the Date/Time components broken out, along with the
 * expected DST offset and abbreviation.
 *
 * According to https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes
 * sys_info has the following structure:
 *
 * struct sys_info
 * {
 *     second_point         begin;
 *     second_point         end;
 *     std::chrono::seconds offset;
 *     std::chrono::minutes save;
 *     std::string          abbrev;
 * };
 */
TestItem toTestItem(const time_zone& tz, sys_seconds st, char type) {
  sys_info info = tz.get_info(st);
  seconds unixSeconds = floor<seconds>(st.time_since_epoch());
  zoned_time<seconds> zdt = make_zoned(&tz, st);
  local_time<seconds> lt = zdt.get_local_time();
  DateTime dateTime = toDateTime(lt);
  long epochSeconds = (long) unixSeconds.count()
      - secondsToAceTimeEpochFromUnixEpoch;
  return TestItem{
      epochSeconds,
      (int)info.offset.count(),
      (int)info.save.count() * 60,
      info.abbrev,
      dateTime.year,
      dateTime.month,
      dateTime.day,
      dateTime.hour,
      dateTime.minute,
      dateTime.second,
      type
  };
}

void addTestItem(TestCollection& testData, const string& zoneName,
    const TestItem& item) {
}

/**
 * Check if (a, b) defines a transition. The return values are:
 * * 0 - no transition
 * * 1 - normal transition
 * * 2 - silent transition (STD and DST canceled each other out)
 */
int isTransition(sys_seconds a, sys_seconds b, const time_zone& tz) {
    sys_info a_info = tz.get_info(a);
    sys_info b_info = tz.get_info(b);

    int a_total_offset = a_info.offset.count();
    int b_total_offset = b_info.offset.count();

    int a_dst_offset = a_info.save.count() * 60;
    int b_dst_offset = b_info.save.count() * 60;

    if (a_total_offset != b_total_offset) {
      return 1;
    } else if (a_dst_offset != b_dst_offset) {
      return 2;
    } else {
      return 0;
    }
}

/**
 * Add a TestItem for one second before a DST transition, and right at the
 * the DST transition.
 */
void addTransitions(TestCollection& collection, const time_zone& tz,
    int startYear, int untilYear) {
  sys_seconds curr = sys_days{January/1/startYear} + seconds(0);
  sys_seconds end = sys_days{January/1/untilYear} + seconds(0);

  while (curr < end) {
    // One second before the DST transition.
    sys_seconds before = curr - seconds(1);

    // Check that (before, curr) pair is a real transition instead of a phantom
    // artifact of the implementation details of the Hinnant date library.
    int status = isTransition(before, curr, tz);

    if (status > 0) {
      // One second before transition.
      auto item = toTestItem(tz, before, (status == 1) ? 'A' : 'a');
      collection.push_back(item);

      // At transition.
      item = toTestItem(tz, curr, (status == 1) ? 'B' : 'b');
      collection.push_back(item);
    }

    sys_info info = tz.get_info(curr);
    curr = info.end;
  }
}

/**
 * Add a TestItem for the 1st of each month (using the local time)
 * as a sanity sample, to make sure things are working, even for timezones with
 * no DST transitions. See
 * https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes#obtaining-a-time_point-from-ymd-hms-components
 * to get code for converting date/time components to a time_point<> (aka
 * sys_time<>).
 */
void addMonthlySamples(TestCollection& collection, const time_zone& tz,
    int startYear, int untilYear) {

  for (int y = startYear; y < untilYear; y++) {
    for (int m = 1; m <= 12; m++) {
      // Add a sample test point on the *second* of each month instead of the
      // first of the month. This prevents Jan 1, 2000 from being converted to a
      // negative epoch seconds for certain timezones, which gets converted into
      // a UTC date in 1999 when ExtendedZoneProcessor is used to convert the
      // epoch seconds back to a ZonedDateTime. The UTC date in 1999 causes the
      // actual max buffer size of ExtendedZoneProcessor to become different
      // than the one predicted by BufSizeEstimator (which samples whole years
      // from 2000 until 2050), and causes the
      // AceTimeValidation/ExtendedHinnantDateTest to fail on the buffer size
      // check.
      //
      // But if that day of the month (with the time of 00:00) is ambiguous, the
      // Hinnant date library throws an exception. Unfortunately, I cannot
      // understand the documentation to figure out how to do what I want, so
      // just punt and use the next day. Use a loop to try every subsequent day
      // of month up to the 28th (which exists in all months).
      for (int d = 2; d <= 28; d++) {
        local_days ld = local_days{month(m)/d/year(y)};
        try {
          zoned_time<seconds> zdt = make_zoned(&tz, ld + seconds(0));

          sys_seconds ss = zdt.get_sys_time();
          TestItem item = toTestItem(tz, ss, 'S');
          collection.push_back(item);
          // One sample per month is enough, so break as soon as we get one.
          break;

        } catch (...) {
          continue; // to next day if error
        }
      }
    }
  }
}

/** Insert TestItems for the given 'zoneName' into testData. */
void processZone(TestData& testData, const string& zoneName,
    int startYear, int untilYear) {
  auto* tzp = locate_zone(zoneName);
  if (tzp == nullptr) {
    fprintf(stderr, "Zone %s not found\n", zoneName.c_str());
    return;
  }

  TestEntry& entry = testData[zoneName];
  addTransitions(entry.transitions, *tzp, startYear, untilYear);
  addMonthlySamples(entry.samples, *tzp, startYear, untilYear);
}

/** Process each zoneName in zones and insert into testData map. */
void processZones(TestData &testData, const vector<string>& zones) {
  int i = 0;
  for (string zoneName : zones) {
    fprintf(stderr, "[%d] %s\n", i, zoneName.c_str());
    processZone(testData, zoneName, startYear, untilYear);
    i++;
  }
}

/**
 * Trim from start (in place). See https://stackoverflow.com/questions/216823
 */
inline void ltrim(string &s) {
	s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch) {
			return !isspace(ch);
	}));
}

/** Read the 'zones.txt' from the stdin, and process each zone. */
vector<string> readZones() {
  vector<string> zones;
  string line;
  while (getline(cin, line)) {
		ltrim(line);
    if (line.empty()) continue;
    if (line[0] == '#') continue;
    zones.push_back(line);
  }

  return zones;
}

/** Sort the TestItems according to epochSeconds. */
/*
void sortTestData(TestData& testData) {
  for (auto& p : testData) {
    sort(p.second.begin(), p.second.end(),
      [](const TestItem& a, const TestItem& b) {
        return a.epochSeconds < b.epochSeconds;
      }
    );
  }
}
*/

void printTestItem(const char* indent, const TestItem& item) {
  printf("%s\"epoch\": %ld,\n", indent, item.epochSeconds);
  printf("%s\"total_offset\": %d,\n", indent, item.utcOffset);
  printf("%s\"dst_offset\": %d,\n", indent, item.dstOffset);
  printf("%s\"y\": %d,\n", indent, item.year);
  printf("%s\"M\": %d,\n", indent, item.month);
  printf("%s\"d\": %d,\n", indent, item.day);
  printf("%s\"h\": %d,\n", indent, item.hour);
  printf("%s\"m\": %d,\n", indent, item.minute);
  printf("%s\"s\": %d,\n", indent, item.second);
  printf("%s\"abbrev\": \"%s\",\n", indent, item.abbrev.c_str());
  printf("%s\"type\": \"%c\"\n", indent, item.type);
}

/**
 * Generate the JSON output on STDOUT which will be redirect into
 * 'validation_data.json' file. Adopted from GenerateData.java.
 */
void printJson(const TestData& testData) {
  string indentUnit = "  ";

  // Version of Hinnant Date library
  string version = "3.0.0";

  // TZDB version
  string tzVersion = date::get_tzdb().version.c_str();

  const char indent0[] = "  ";
  const char indent1[] = "    ";
  const char indent2[] = "      ";
  const char indent3[] = "        ";
  const char indent4[] = "          ";

  printf("{\n");
  printf("%s\"start_year\": %d,\n", indent0, startYear);
  printf("%s\"until_year\": %d,\n", indent0, untilYear);
  printf("%s\"epoch_year\": %d,\n", indent0, epochYear);
  printf("%s\"source\": \"Hinnant Date\",\n", indent0);
  printf("%s\"version\": \"%s\",\n", indent0, version.c_str());
  printf("%s\"tz_version\": \"%s\",\n", indent0, tzVersion.c_str());
  printf("%s\"has_valid_abbrev\": true,\n", indent0);
  printf("%s\"has_valid_dst\": true,\n", indent0);
  printf("%s\"test_data\": {\n", indent0);

  // Print each zone
  int zoneCount = 1;
  int numZones = testData.size();
  for (const auto& zoneEntry : testData) {
    string zoneName = zoneEntry.first;
    const TestEntry& entry = zoneEntry.second;
    printf("%s\"%s\": {\n", indent1, zoneName.c_str());

    // Print transitions
    int itemCount = 1;
    const TestCollection& transitions = entry.transitions;
    printf("%s\"%s\": [\n", indent2, "transitions");
    for (const TestItem& item : transitions) {
      printf("%s{\n", indent3);
      printTestItem(indent4, item);
      printf("%s}%s\n", indent3,
          (itemCount < (int)transitions.size()) ? "," : "");
      itemCount++;
    }
    printf("%s],\n", indent2);

    // Print samples
    itemCount = 1;
    const TestCollection& samples = entry.samples;
    printf("%s\"%s\": [\n", indent2, "samples");
    for (const TestItem& item : samples) {
      printf("%s{\n", indent3);
      printTestItem(indent4, item);
      printf("%s}%s\n", indent3, (itemCount < (int)samples.size()) ? "," : "");
      itemCount++;
    }
    printf("%s]\n", indent2);

    printf("%s}%s\n", indent1, (zoneCount < numZones) ? "," : "");
    zoneCount++;
  }

  printf("%s}\n", indent0);
  printf("}\n");
}

void usageAndExit() {
  fprintf(stderr,
    "Usage: generate_data [--install_dir {dir}] [--tz_version {version}]\n"
    "   --start_year start --until_year until --epoch_year year\n"
    "   < zones.txt\n");
  exit(1);
}

void shift(int& argc, const char* const*& argv) {
  argc--;
  argv++;
}

bool argEquals(const char* s, const char* t) {
  return strcmp(s, t) == 0;
}

int main(int argc, const char* const* argv) {
  // Parse command line flags.
  string startYearStr = "";
  string untilYearStr = "";
  string epochYearStr = "";
  string tzVersion = "";
  string installDir = "";

  shift(argc, argv);
  while (argc > 0) {
    if (argEquals(argv[0], "--start_year")) {
      shift(argc, argv);
      if (argc == 0) usageAndExit();
      startYearStr = argv[0];
    } else if (argEquals(argv[0], "--until_year")) {
      shift(argc, argv);
      if (argc == 0) usageAndExit();
      untilYearStr = argv[0];
    } else if (argEquals(argv[0], "--epoch_year")) {
      shift(argc, argv);
      if (argc == 0) usageAndExit();
      epochYearStr = argv[0];
    } else if (argEquals(argv[0], "--tz_version")) {
      shift(argc, argv);
      if (argc == 0) usageAndExit();
      tzVersion = argv[0];
    } else if (argEquals(argv[0], "--install_dir")) {
      shift(argc, argv);
      if (argc == 0) usageAndExit();
      installDir = argv[0];
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

  // Following commented out because tzVersion can be empty if using a tagless
  // branch for the TZDB.
  // if (tzVersion.empty()) {
  //   fprintf(stderr, "Must give --tz_version flag for Hinnant Date'\n");
  //   usageAndExit();
  // }

  if (startYearStr.empty()) {
    fprintf(stderr, "Flag required: --start_year\n");
    usageAndExit();
  }
  if (untilYearStr.empty()) {
    fprintf(stderr, "Flag required: --until_year\n");
    usageAndExit();
  }
  if (epochYearStr.empty()) {
    fprintf(stderr, "Flag required: --epoch_year\n");
    usageAndExit();
  }

  startYear = atoi(startYearStr.c_str());
  untilYear = atoi(untilYearStr.c_str());
  epochYear = atoi(epochYearStr.c_str());

  // Set the install directory if specified. Otherwise the default is
  // ~/Downloads/tzdata on a Linux or MacOS machine. See
  // https://howardhinnant.github.io/date/tz.html#Installation.
  if (! installDir.empty()) {
    set_install(installDir);
  }

  // Explicitly download load the TZ Database at the specified version if
  // --tz_version is given. This works even if AUTO_DOWNLOAD=0. See
  // https://github.com/HowardHinnant/date/wiki/Examples-and-Recipes#thoughts-on-reloading-the-iana-tzdb-for-long-running-programs
  // and https://howardhinnant.github.io/date/tz.html#database.
  if (! tzVersion.empty()) {
    if (! remote_download(tzVersion)) {
      fprintf(stderr, "Failed to download TZ Version %s\n", tzVersion.c_str());
      exit(1);
    }
    if (! remote_install(tzVersion)) {
      fprintf(stderr, "Failed to install TZ Version %s\n", tzVersion.c_str());
      exit(1);
    }
  }

  // Calculate the number of seconds from Unix epoch to the AceTime epoch.
  // Why is this calculation so darned difficult in C++? See
  // https://stackoverflow.com/questions/67829275
  auto aceTimeEpoch = sys_days(year(epochYear)/1/1);
  secondsToAceTimeEpochFromUnixEpoch =
      86400 * (long) aceTimeEpoch.time_since_epoch().count();

  // Install the TZ database. Caution: If the source directory is pointed to
  // the raw https://github.com/eggert/tz/ repo, it is not in the form that is
  // expected (I think the 'version' file is missing), so the version returned
  // by get_tzdb() will be in correct.
  reload_tzdb();
  if (tzVersion.empty()) {
    fprintf(stderr, "Loaded existing TZ Version %s\n",
        date::get_tzdb().version.c_str());
  } else {
    fprintf(stderr, "Loaded TZ Version %s\n", tzVersion.c_str());
  }

  // Process the zones on the STDIN
  vector<string> zones = readZones();

  fprintf(stderr, "Generating validation data\n");
  TestData testData;
  processZones(testData, zones);
  //sortTestData(testData);

  fprintf(stderr, "Writing validation data\n");
  printJson(testData);

  fprintf(stderr, "Done\n");
  return 0;
}
