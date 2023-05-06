/*
 * Copyright 2021 Brian T. Park
 *
 * MIT License
 */

using System;
using System.Collections.Generic;
using System.IO;
using NodaTime;
using NodaTime.Extensions;
using NodaTime.Text;
using NodaTime.TimeZones;

namespace compare_noda
{
    class Program
    {
        // Usage:
        // $ dotnet run -- [--help]
        //      --start_year start
        //      --until_year until
        //      --epoch_year epoch
        //      [--nzd_file {file}]
        //      < zones.txt
        //      > validation_data.json
        //
        // Based on the equivalent Java program 'compare_java'.
        //
        static void Main(string[] args)
        {
            // Parse command line flags
            int argc = args.Length;
            int argi = 0;
            /*
            if (argc == 0) {
                UsageAndExit(1);
            }
            */
            string start = "";
            string until = "";
            string epoch = "";
            string nzdFilePath = "";
            while (argc > 0)
            {
                string arg0 = args[argi];
                if ("--start_year".Equals(arg0))
                {
                    {argc--; argi++; arg0 = args[argi];} // shift-left
                    start = arg0;
                }
                else if ("--until_year".Equals(arg0))
                {
                    {argc--; argi++; arg0 = args[argi];} // shift-left
                    until = arg0;
                }
                else if ("--epoch_year".Equals(arg0))
                {
                    {argc--; argi++; arg0 = args[argi];} // shift-left
                    epoch = arg0;
                }
                else if ("--nzd_file".Equals(arg0))
                {
                    {argc--; argi++; arg0 = args[argi];} // shift-left
                    nzdFilePath = arg0;
                }
                else if ("--help".Equals(arg0))
                {
                    UsageAndExit(0);
                    break;
                }
                else if ("--".Equals(arg0))
                {
                    break;
                }
                else if (arg0.StartsWith("-"))
                {
                    Console.Error.WriteLine($"Unknown flag '{arg0}'");
                    UsageAndExit(1);
                }
                else if (! arg0.StartsWith("-"))
                {
                    break;
                }
                {argc--; argi++;} // shift-left
            }
            if (string.IsNullOrEmpty(start)) {
                UsageAndExit(1);
            }
            if (string.IsNullOrEmpty(until)) {
                UsageAndExit(1);
            }
            if (string.IsNullOrEmpty(epoch)) {
                UsageAndExit(1);
            }

            int startYear = int.Parse(start);
            int untilYear = int.Parse(until);
            int epochYear = int.Parse(epoch);

            // https://nodatime.org/3.0.x/userguide/tzdb
            IDateTimeZoneProvider provider;
            if (! string.IsNullOrEmpty(nzdFilePath)) {
                // Read the TZDB file from disk.
				using (var stream = File.OpenRead(nzdFilePath))
				{
					var source = TzdbDateTimeZoneSource.FromStream(stream);
					provider = new DateTimeZoneCache(source);
				}
            }
            else
            {
                provider = DateTimeZoneProviders.Tzdb;
            }

            List<string> zones = ReadZones();
            GenerateData generator = new GenerateData(startYear, untilYear, epochYear, provider);
            IDictionary<string, TestEntry> testData = generator.CreateTestData(zones);
            generator.PrintJson(testData);
        }

        private static void UsageAndExit(int exitCode)
        {
            string usage = "Usage: compare_noda --start_year {year} "
                + "--until_year {year} --epoch_year {year} [--nzd_file file] "
                + "< zones.txt";
            if (exitCode == 0)
            {
                Console.WriteLine(usage);
            } else {
                Console.Error.WriteLine(usage);
            }
            Environment.Exit(exitCode);
        }

        private static List<string> ReadZones()
        {
            var zones = new List<string>();
            string line;
            while ((line = Console.ReadLine()) != null)
            {
                line = line.Trim();
                if (String.IsNullOrEmpty(line)) continue;
                if (line.StartsWith('#')) continue;
                zones.Add(line);
            }
            return zones;
        }
    }

    class GenerateData
    {
        // Number of seconds from Unix epoch (1970-01-01T00:00:00Z) to AceTime epoch (usually
        // 2050-01-01T00:00:00Z). Non-static, will be calculated in the constructor.
        private static int secondsToAceTimeEpochFromUnixEpoch = 946684800;

        private const string indent0 = "  ";
        private const string indent1 = "    ";
        private const string indent2 = "      ";
        private const string indent3 = "        ";
        private const string indent4 = "          ";

        public GenerateData(int startYear, int untilYear, int epochYear, IDateTimeZoneProvider provider)
        {
            this.startYear = startYear;
            this.untilYear = untilYear;
            this.epochYear = epochYear;
            this.dateTimeZoneProvider = provider;

            var epochInstant = new LocalDateTime(epochYear, 1, 1, 0, 0)
                .InUtc().ToInstant();
            secondsToAceTimeEpochFromUnixEpoch = (int) epochInstant.ToUnixTimeSeconds();
        }

        public IDictionary<string, TestEntry> CreateTestData(List<string> zones)
        {
            var testData = new SortedDictionary<string, TestEntry>();
            foreach (string zone in zones)
            {
                DateTimeZone tz = dateTimeZoneProvider[zone];
                var startInstant = new LocalDateTime(startYear, 1, 1, 0, 0)
                    .InZoneLeniently(tz).ToInstant();
                var untilInstant = new LocalDateTime(untilYear, 1, 1, 0, 0)
                    .InZoneLeniently(tz).ToInstant();

                var testEntry = new TestEntry();
                testEntry.transitions = CreateTransitions(tz, startInstant, untilInstant);
                testEntry.samples = CreateSamples(tz, startInstant, untilInstant);
                testData.Add(zone, testEntry);
            }
            return testData;
        }

        private List<TestItem> CreateTransitions(DateTimeZone tz, Instant startInstant, Instant untilInstant) {
            var items = new List<TestItem>();
            var intervals = tz.GetZoneIntervals(startInstant, untilInstant);
            foreach (ZoneInterval zi in intervals)
            {
                if (zi.HasStart)
                {
                    var isoStart = zi.IsoLocalStart;
                    if (isoStart.Year > startYear)
                    {
                        // A: One minute before the transition
                        // B: Right after the DST transition.
                        Instant after = zi.Start;
                        Duration oneMinute = Duration.FromMinutes(1);
                        Instant before = after - oneMinute;
                        items.Add(CreateTestItem(tz, before, 'A'));
                        items.Add(CreateTestItem(tz, after, 'B'));
                    }
                }
            }
            return items;
        }

        private static TestItem CreateTestItem(DateTimeZone tz, Instant instant, char type)
        {
            ZoneInterval zi = tz.GetZoneInterval(instant);
            ZonedDateTime zdt = instant.InZone(tz);

            var testItem = new TestItem();
            testItem.epochSeconds = ToAceTimeEpochSeconds(instant.ToUnixTimeSeconds());
            testItem.utcOffset = zi.WallOffset.Seconds;
            testItem.dstOffset = zi.Savings.Seconds;
            testItem.year = zdt.Year;
            testItem.month = zdt.Month;
            testItem.day = zdt.Day;
            testItem.hour = zdt.Hour;
            testItem.minute = zdt.Minute;
            testItem.second = zdt.Second;
            testItem.abbrev = zi.Name;
            testItem.type = type;
            return testItem;
        }

        private List<TestItem> CreateSamples(
                DateTimeZone tz, Instant startInstant, Instant untilInstant) {

            var items = new List<TestItem>();
            ZonedDateTime startDt = startInstant.InZone(tz);
            ZonedDateTime untilDt = untilInstant.InZone(tz);

            for (int year = startDt.Year; year < untilDt.Year; year++)
            {
                // Add a sample test point on the *second* of each month instead of the first of the
                // month. This prevents Jan 1, 2000 from being converted to a negative epoch seconds
                // for certain timezones, which gets converted into a UTC date in 1999 when
                // ExtendedZoneProcessor is used to convert the epoch seconds back to a
                // ZonedDateTime. The UTC date in 1999 causes the actual max buffer size of
                // ExtendedZoneProcessor to become different than the one predicted by
                // BufSizeEstimator (which samples whole years from 2000 until 2050), and causes the
                // AceTimeValidation/ExtendedNodaTest to fail on the buffer size check.
                for (int month = 1; month <= 12; month++)
                {
                    ZonedDateTime zdt = new LocalDateTime(year, month, 2, 0, 0).InZoneLeniently(tz);
                    items.Add(CreateTestItem(tz, zdt.ToInstant(), 'S'));
                }

                // Add the last day and hour of the year
                ZonedDateTime lastdt = new LocalDateTime(year, 12, 31, 23, 0).InZoneLeniently(tz);
                items.Add(CreateTestItem(tz, lastdt.ToInstant(), 'Y'));
            }
            return items;
        }

        // Serialize to JSON manually, for 2 reasons:
        // a) to reduce external dependencies,
        // b) to follow the Java code.
        public void PrintJson(IDictionary<string, TestEntry> testData)
        {
            string tzVersion = dateTimeZoneProvider.VersionId;

            Console.WriteLine("{");
            Console.WriteLine($"{indent0}\"start_year\": {startYear},");
            Console.WriteLine($"{indent0}\"until_year\": {untilYear},");
            Console.WriteLine($"{indent0}\"epoch_year\": {epochYear},");
            Console.WriteLine($"{indent0}\"source\": \"NodaTime\",");
            Console.WriteLine($"{indent0}\"version\": \"3.1\",");
            Console.WriteLine($"{indent0}\"tz_version\": \"{tzVersion}\",");
            Console.WriteLine($"{indent0}\"has_valid_abbrev\": true,");
            Console.WriteLine($"{indent0}\"has_valid_dst\": true,");
            Console.WriteLine($"{indent0}\"test_data\": {{");

            int zoneCount = 1;
            int numZones = testData.Count;
            foreach (KeyValuePair<string, TestEntry> entry in testData)
            {
                // Print the zone name
                Console.WriteLine($"{indent1}\"{entry.Key}\": {{");
                var testEntry = entry.Value;

                // Print transitions
                Console.WriteLine($"{indent2}\"transitions\": [");
                int itemCount = 1;
                List<TestItem> items = testEntry.transitions;
                foreach (TestItem item in items)
                {
                    Console.WriteLine($"{indent3}{{");
                    PrintItem(indent4, item);
                    string innerComma = itemCount < items.Count ? "," : "";
                    Console.WriteLine($"{indent3}}}{innerComma}");
                    itemCount++;
                }
                Console.WriteLine($"{indent2}],");

                // Print samples
                Console.WriteLine($"{indent2}\"samples\": [");
                itemCount = 1;
                items = testEntry.samples;
                foreach (TestItem item in items)
                {
                    Console.WriteLine($"{indent3}{{");
                    PrintItem(indent4, item);
                    string innerComma = itemCount < items.Count ? "," : "";
                    Console.WriteLine($"{indent3}}}{innerComma}");
                    itemCount++;
                }
                Console.WriteLine($"{indent2}]");

                string outerComma = zoneCount < numZones ? "," : "";
                Console.WriteLine($"{indent1}}}{outerComma}");
                zoneCount++;
            }

            Console.WriteLine($"{indent0}}}");
            Console.WriteLine("}");
        }

        private void PrintItem(string indent, TestItem item)
        {
            Console.WriteLine($"{indent}\"epoch\": {item.epochSeconds},");
            Console.WriteLine($"{indent}\"total_offset\": {item.utcOffset},");
            Console.WriteLine($"{indent}\"dst_offset\": {item.dstOffset},");
            Console.WriteLine($"{indent}\"y\": {item.year},");
            Console.WriteLine($"{indent}\"M\": {item.month},");
            Console.WriteLine($"{indent}\"d\": {item.day},");
            Console.WriteLine($"{indent}\"h\": {item.hour},");
            Console.WriteLine($"{indent}\"m\": {item.minute},");
            Console.WriteLine($"{indent}\"s\": {item.second},");
            Console.WriteLine($"{indent}\"abbrev\": \"{item.abbrev}\",");
            Console.WriteLine($"{indent}\"type\": \"{item.type}\"");
        }

        private static int ToAceTimeEpochSeconds(long unixEpochSeconds)
        {
            return (int) (unixEpochSeconds - secondsToAceTimeEpochFromUnixEpoch);
        }

        private readonly int startYear;
        private readonly int untilYear;
        private readonly int epochYear;
        private readonly IDateTimeZoneProvider dateTimeZoneProvider;
    }

    struct TestEntry
    {
        internal List<TestItem> transitions;
        internal List<TestItem> samples;
    }

    struct TestItem
    {
        internal int epochSeconds; // seconds from AceTime epoch (2000-01-01T00:00:00Z)
        internal int utcOffset; // total UTC offset in seconds
        internal int dstOffset; // DST shift from standard offset in seconds
        internal int year;
        internal int month;
        internal int day;
        internal int hour;
        internal int minute;
        internal int second;
        internal string abbrev;
        internal char type;
    }
}
