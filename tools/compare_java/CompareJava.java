/*
 * Copyright 2019 Brian T. Park
 *
 * MIT License
 */

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.time.ZonedDateTime;
import java.time.format.TextStyle;
import java.time.zone.ZoneOffsetTransition;
import java.time.zone.ZoneRules;
import java.time.zone.ZoneRulesException;
import java.time.zone.ZoneRulesProvider;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeMap;
import java.util.TreeSet;

/**
 * Generate 'validation_data.json' from list of zones in the 'zones.txt' file.
 *
 * <pre>
 * {@code
 * $ javac CompareJava.java
 * $ java CompareJava [--start_year start] [--until_year until]
 *      [--epoch_year year] [--validate_dst] [--print_zones]
 *      < zones.txt
 *      > validation_data.json
 * }
 * </pre>
 *
 * The zones.txt file is a list of fully qualified zone names (e.g. "America/Los_Angeles") listed
 * one zone per line. It will normally be generated programmatically using:
 *
 * <pre>
 * {@code
 * $ ../../tools/tzcompiler.sh --tag 2019a --action zonedb --language zonelist
 * }
 * </pre>
 */
public class CompareJava {
  // Number of seconds from Unix epoch (1970-01-01T00:00:00Z) to AceTime epoch
  // (usually 2050-01-01T00:00:00Z). Non-static, will be calculated in the constructor.
  private static long secondsToAceTimeEpochFromUnixEpoch = 946684800;

  public static void main(String[] args) throws IOException {
    String invocation = "java CompareJava " + String.join(" ", args);

    // Parse command line flags
    int argc = args.length;
    int argi = 0;
    if (argc == 0) {
      usageAndExit();
    }
    String start = "2000";
    String until = "2100";
    String epoch = "2050";
    boolean printZones = false;
    while (argc > 0) {
      String arg0 = args[argi];
      if ("--start_year".equals(arg0)) {
        {argc--; argi++; arg0 = args[argi];} // shift-left
        start = arg0;
      } else if ("--until_year".equals(arg0)) {
        {argc--; argi++; arg0 = args[argi];} // shift-left
        until = arg0;
      } else if ("--epoch_year".equals(arg0)) {
        {argc--; argi++; arg0 = args[argi];} // shift-left
        epoch = arg0;
      } else if ("--print_zones".equals(arg0)) {
        printZones = true;
      } else if ("--".equals(arg0)) {
        break;
      } else if (arg0.startsWith("-")) {
        System.err.printf("Unknown flag '%s'%n", arg0);
        usageAndExit();
      } else if (!arg0.startsWith("-")) {
        break;
      }
      {argc--; argi++;} // shift-left
    }

    if (printZones) {
      printAvailableZoneIds();
    } else {
      // Validate --start_year and --end_year.
      // Should check for NumberFormatException but too much overhead for this simple tool.
      int startYear = Integer.parseInt(start);
      int untilYear = Integer.parseInt(until);
      int epochYear = Integer.parseInt(epoch);

      List<String> zones = readZones();
      CompareJava generator = new CompareJava(
          invocation, startYear, untilYear, epochYear);
      Map<String, TestEntry> testData = generator.createTestData(zones);
      generator.printJson(testData);
    }
  }

  private static void usageAndExit() {
    System.err.println("Usage: java CompareJava [--start_year {start}]");
    System.err.println("    [--until_year {until}] [--validate_dst]");
    System.err.println("    < zones.txt");
    System.err.println("    > validation_data.json");
    System.exit(1);
  }

  /** Print out the list of ZoneIds in the java.time database. */
  static void printAvailableZoneIds() {
    Set<String> allZones = ZoneId.getAvailableZoneIds();
    System.out.printf("Found %s ids total:\n", allZones.size());
    SortedSet<String> selectedIds = new TreeSet<>();
    int numZones = 0;
    for (String id : allZones) {

      // Not sure why I wanted to filter these out.
      /*
      if (id.startsWith("Etc")) continue;
      if (id.startsWith("SystemV")) continue;
      if (id.startsWith("US")) continue;
      if (id.startsWith("Canada")) continue;
      if (id.startsWith("Brazil")) continue;
      if (!id.contains("/")) continue;
      */

      selectedIds.add(id);
    }
    System.out.printf("Selected %s ids:\n", selectedIds.size());
    for (String id : selectedIds) {
      System.out.println("  " + id);
    }
  }

  /**
   * Return the list of zone names from the System.in.
   * Ignore empty lines and comment lines starting with '#'.
   */
  private static List<String> readZones() throws IOException {
    List<String> zones = new ArrayList<>();
    try (BufferedReader reader = new BufferedReader(new InputStreamReader(System.in))) {
      String line;
      while ((line = reader.readLine()) != null) {
        line = line.trim();
        if (line.isEmpty()) continue;
        if (line.startsWith("#")) continue;
        zones.add(line);
      }
    }
    return zones;
  }

  /** Constructor. */
  private CompareJava(String invocation, int startYear, int untilYear, int epochYear) {
    this.invocation = invocation;
    this.startYear = startYear;
    this.untilYear = untilYear;
    this.epochYear = epochYear;

    LocalDateTime ldt = LocalDateTime.of(epochYear, 1, 1, 0, 0, 0);
    secondsToAceTimeEpochFromUnixEpoch = ldt.toEpochSecond(ZoneOffset.UTC);
  }

  /**
   * Create list of TestItems for each zone in this.zones. If the zone is missing from java.time,
   * create an entry with a null value to indicate that the zone is missing. E.g "Asia/Qostanay"
   * exists in 2019a but is missing from (openjdk version "11.0.3" 2019-04-16).
   */
  private Map<String, TestEntry> createTestData(List<String> zones) {
    Map<String, TestEntry> testData = new TreeMap<>();
    int i = 0;
    for (String zoneName : zones) {
      System.err.printf("[%d] %s\n", i, zoneName);
      ZoneId zoneId;
      try {
        zoneId = ZoneId.of(zoneName);
      } catch (ZoneRulesException e) {
        System.err.printf("Zone '%s' not found%n", zoneName);
        continue;
      }
      TestEntry entry = createTestEntry(zoneId);
      testData.put(zoneName, entry);

      i++;
    }
    return testData;
  }

  /** Return a list of TestItems for zoneId sorted by increasing epochSeconds. */
  private TestEntry createTestEntry(ZoneId zoneId) {
    Instant startInstant = ZonedDateTime.of(startYear, 1, 1, 0, 0, 0, 0, zoneId).toInstant();
    Instant untilInstant = ZonedDateTime.of(untilYear, 1, 1, 0, 0, 0, 0, zoneId).toInstant();

    TestEntry entry = new TestEntry();
    entry.transitions = createTransitions(zoneId, startInstant, untilInstant);
    entry.samples = createSamples(zoneId, startInstant, untilInstant);

    return entry;
  }

  private static List<TestItem> createTransitions(
      ZoneId zoneId, Instant startInstant, Instant untilInstant) {

    List<TestItem> items = new ArrayList<>();
    ZonedDateTime untilDateTime = ZonedDateTime.ofInstant(untilInstant, zoneId);
    ZoneRules rules = zoneId.getRules();
    Instant prevInstant = startInstant;
    int untilYear = untilDateTime.getYear();
    while (true) {
      // Exit if no more transitions
      ZoneOffsetTransition transition = rules.nextTransition(prevInstant);
      if (transition == null) {
        break;
      }
      // Exit if we get to untilYear.
      LocalDateTime transitionDateTime = transition.getDateTimeBefore();
      if (transitionDateTime.getYear() >= untilYear) {
        break;
      }

      // Get transition time
      Instant currentInstant = transition.getInstant();
      Instant beforeInstant = currentInstant.minusSeconds(1);

      // Get UTC offsets
      long currentDst = rules.getDaylightSavings(currentInstant).getSeconds();
      long currentOffset = rules.getOffset(currentInstant).getTotalSeconds();
      long beforeDst = rules.getDaylightSavings(beforeInstant).getSeconds();
      long beforeOffset = rules.getOffset(beforeInstant).getTotalSeconds();

      // Skip phantom transitions which are implementation artifacts.
      if (currentOffset == beforeOffset && currentDst == beforeDst) {
        continue;
      }

      // 'A': One second before the transition
      // 'B': Right after the DST transition
      // 'a': silent transition where only the DST offset changes
      // 'b': silent transition where only the DST offset changes
      char aType = (beforeOffset != currentOffset) ? 'A': 'a';
      char bType = (beforeOffset != currentOffset) ? 'B': 'b';
      items.add(createTestItem(beforeInstant, zoneId, aType));
      items.add(createTestItem(currentInstant, zoneId, bType));

      prevInstant = currentInstant;
    }
    return items;
  }

  /**
   * Add sample test items from startInstant to untilInstant for zoneId. Use the *second* of each
   * month instead of the first of the month. This prevents Jan 1, 2000 from being converted to a
   * negative epoch seconds for certain timezones, which gets converted into a UTC date in 1999 when
   * ExtendedZoneProcessor is used to convert the epoch seconds back to a ZonedDateTime. The UTC
   * date in 1999 causes the actual max buffer size of ExtendedZoneProcessor to become different
   * than the one predicted by BufSizeEstimator (which samples whole years from 2000 until 2050),
   * and causes the AceTimeValidation/ExtendedJavaTest to fail on the buffer size check.
  */
  private static List<TestItem> createSamples(
      ZoneId zoneId, Instant startInstant, Instant untilInstant) {

    ZonedDateTime startDateTime = ZonedDateTime.ofInstant(startInstant, zoneId);
    ZonedDateTime untilDateTime = ZonedDateTime.ofInstant(untilInstant, zoneId);

    List<TestItem> items = new ArrayList<>();
    for (int year = startDateTime.getYear(); year < untilDateTime.getYear(); year++) {
      for (int month = 1; month <= 12; month++) {
        char type = 'S';
        for (int day = 2; day <= 28; day++) {
          LocalDateTime ldt = LocalDateTime.of(year, month, day, 0, 0, 0);
          ZonedDateTime zdt = ZonedDateTime.of(ldt, zoneId);
          Instant instant = zdt.toInstant();
          ZonedDateTime rdt = ZonedDateTime.ofInstant(instant, zoneId);

          // check not a gap
          if (rdt.getYear() == year
              && rdt.getMonth().getValue() == month
              && rdt.getDayOfMonth() == day
              && rdt.getHour() == 0
              && rdt.getMinute() == 0
              && rdt.getSecond() == 0) {
            items.add(createTestItem(zdt.toInstant(), zoneId, type));
            break;
          }

          // continue trying subsequent days
          type = 'T';
        }
      }
    }
    return items;
  }

  /** Create a test item using the instant to determine the offsets. */
  private static TestItem createTestItem(Instant instant, ZoneId zoneId, char type) {
    // Calculate the offsets using the instant
    ZoneRules rules = zoneId.getRules();
    Duration dst = rules.getDaylightSavings(instant);
    ZoneOffset offset = rules.getOffset(instant);

    // Convert instant to dateTime components.
    ZonedDateTime dateTime = ZonedDateTime.ofInstant(instant, zoneId);

    // Get abbreviation. See https://stackoverflow.com/questions/56167361. It looks like Java's
    // abbreviations are completely different than the abbreviations used in the TZ Database files.
    // For example, PST or PDT for America/Los_Angeles is returned as "PT", which seems brain-dead
    // since no one in America uses the abbreviation "PT.
    String abbrev = zoneId.getDisplayName(TextStyle.SHORT_STANDALONE, Locale.US);

    TestItem item = new TestItem();
    item.epochSeconds = (int) (instant.getEpochSecond() - secondsToAceTimeEpochFromUnixEpoch);
    item.utcOffset = offset.getTotalSeconds();
    item.dstOffset = (int) dst.getSeconds();
    item.year = dateTime.getYear();
    item.month = dateTime.getMonthValue();
    item.day = dateTime.getDayOfMonth();
    item.hour = dateTime.getHour();
    item.minute = dateTime.getMinute();
    item.second = dateTime.getSecond();
    item.abbrev = abbrev;
    item.type = type;

    return item;
  }

  /**
   * Print the JSON representation of the testData to the System.out.  Normally, it will be
   * redirected to a file named 'validation_data.json'. We serialize JSON manually to avoid pulling
   * in any external dependencies, The TestData format is pretty simple.
   */
  private void printJson(Map<String, TestEntry> testData) throws IOException {
    String indent0 = "  ";
    String indent1 = "    ";
    String indent2 = "      ";
    String indent3 = "        ";
    String indent4 = "          ";

    try (PrintWriter writer = new PrintWriter(System.out)) {
      // JDK version
      String version = System.getProperty("java.version");

      // Get TZDB version.
      // https://stackoverflow.com/questions/7956044
      String tzDbVersion = ZoneRulesProvider.getVersions("UTC").lastEntry().getKey();

      writer.println("{");
      writer.printf("%s\"start_year\": %s,\n", indent0, startYear);
      writer.printf("%s\"until_year\": %s,\n", indent0, untilYear);
      writer.printf("%s\"epoch_year\": %s,\n", indent0, epochYear);
      writer.printf("%s\"source\": \"Java11/java.time\",\n", indent0);
      writer.printf("%s\"version\": \"%s\",\n", indent0, version);
      writer.printf("%s\"tz_version\": \"%s\",\n", indent0, tzDbVersion);
      // Set 'has_valid_abbrev' to false because java.time abbreviations seem completely different
      // than the ones provided by the TZ Database files.
      writer.printf("%s\"has_valid_abbrev\": false,\n", indent0);
      writer.printf("%s\"has_valid_dst\": true,\n", indent0);
      writer.printf("%s\"test_data\": {\n", indent0);

      // Print each zone
      int zoneCount = 1;
      int numZones = testData.size();
      for (Map.Entry<String, TestEntry> entry : testData.entrySet()) {
        // Print the zone name
        writer.printf("%s\"%s\": {\n", indent1, entry.getKey());
        TestEntry testEntry = entry.getValue();

        // Print the transitions
        writer.printf("%s\"%s\": [\n", indent2, "transitions");
        int itemCount = 1;
        List<TestItem> items = testEntry.transitions;
        for (TestItem item : items) {
          writer.printf("%s{\n", indent3);
          printTestItem(writer, indent4, item);
          writer.printf("%s}%s\n", indent3, (itemCount < items.size()) ? "," : "");
          itemCount++;
        }
        writer.printf("%s],\n", indent2);

        // Print the samples
        writer.printf("%s\"%s\": [\n", indent2, "samples");
        items = testEntry.samples;
        itemCount = 1;
        for (TestItem item : items) {
          writer.printf("%s{\n", indent3);
          printTestItem(writer, indent4, item);
          writer.printf("%s}%s\n", indent3, (itemCount < items.size()) ? "," : "");
          itemCount++;
        }
        writer.printf("%s]\n", indent2);

        writer.printf("%s}%s\n", indent1, (zoneCount < numZones) ? "," : "");
        zoneCount++;
      }
      writer.printf("%s}\n", indent0);

      writer.printf("}\n");
    }
  }

  private void printTestItem(PrintWriter writer, String indent, TestItem item) throws IOException {
    writer.printf("%s\"epoch\": %d,\n", indent, item.epochSeconds);
    writer.printf("%s\"total_offset\": %d,\n", indent, item.utcOffset);
    writer.printf("%s\"dst_offset\": %d,\n", indent, item.dstOffset);
    writer.printf("%s\"y\": %d,\n", indent, item.year);
    writer.printf("%s\"M\": %d,\n", indent, item.month);
    writer.printf("%s\"d\": %d,\n", indent, item.day);
    writer.printf("%s\"h\": %d,\n", indent, item.hour);
    writer.printf("%s\"m\": %d,\n", indent, item.minute);
    writer.printf("%s\"s\": %d,\n", indent, item.second);
    writer.printf("%s\"abbrev\": \"%s\",\n", indent, item.abbrev);
    writer.printf("%s\"type\": \"%s\"\n", indent, item.type);
  }

  // constructor parameters
  private final String invocation;
  private final int startYear;
  private final int untilYear;
  private final int epochYear;
}

class TestEntry {
  List<TestItem> transitions;
  List<TestItem> samples;
}

class TestItem {
  int epochSeconds; // seconds from AceTime epoch (2000-01-01T00:00:00Z)
  int utcOffset; // total UTC offset in seconds
  int dstOffset; // DST shift from standard offset in seconds
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  String abbrev;
  char type;
}
