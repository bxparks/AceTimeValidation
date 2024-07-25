# Changelog

- Unreleased
    - Upgrade TZDB to 2024a
        - https://mm.icann.org/pipermail/tz-announce/2024-February/000081.html
        - "Kazakhstan unifies on UTC+5 beginning 2024-03-01. Palestine springs
          forward a week later after Ramadan. zic no longer pretends to support
          indefinite-past DST. localtime no longer mishandles Ciudad Juárez in
          2422."
- 1.7.1 (2024-01-12, TZDB 2023d)
    - Wrap long invocation lines in `validation_data.*` files.
    - Upgrade TZDB to 2023d
        - https://mm.icann.org/pipermail/tz-announce/2023-December/000080.html
        - "Ittoqqortoormiit, Greenland changes time zones on 2024-03-31. Vostok,
          Antarctica changed time zones on 2023-12-18. Casey, Antarctica changed
          time zones five times since 2020. Code and data fixes for Palestine
          timestamps starting in 2072. A new data file zonenow.tab for
          timestamps starting now."
- 1.7.0 (2023-06-27, TZDB 2023c)
    - tools
        - Add `compare_acetimego`.
        - Fix `int` overflow bugs when calculating the epochSeconds beyond a
          32-bit signed integer:
            - `compare_noda`
            - `compare_java`
        - Add `offset_granularity` to `validation_data.json` to allow
          `acetime_basic.txt` and `acetime_extended.txt` to be compared
          against `acetime_complete.txt` by truncating the expected offset
          values.
    - validation:
        - Set `START_YEAR`, `UNTIL_YEAR` to `[1800,2200)` which covers
          all DST transitions in the IANA TZ database (`[1844,2087]`).
        - Support `--scope complete` and the `zonedbc` database for AceTime.
        - Add validation tests for `[1800,2200)` which covers all DST
          transitions in the IANA TZDB `[1844,2087)`.
            - `diff_acetime_complete`
            - `diff_acetimec`
            - `diff_acetimego`
            - `diff_acetimepy`
            - `diff_hinnant`
        - Add validation tests that handle scope=basic and scope=extended
          subsets:
            - `diff_acetime_basic`
            - `diff_acetime_extended`
        - Implement targets for other third party libraries:
            - `diff_libc` - conformant
            - `diff_noda` - conformant
            - `diff_zoneinfo` - errors on 32 zones
            - `diff_gotime` - errors on 23 zones
            - `diff_java` - errors on ~10 zones
    - Set `baseline.txt` to be `acetime_complete.txt`.
        - This allows us to validation AceTime (complete) against all other
          third party libraries.
- 1.6.0 (2023-05-30, TZDB 2023c)
    - Split the `test_data` component of `validation_data.json` into 2 lists:
        - `transitions` containing DST transitions of the timezone, and
        - `samples` containing samples from `start_year` to `until_year`
    - `./tools/compare_xxx`
        - Update scripts to generate `validation_data.json` with split
          `transitions` and `samples`.
        - Add `compare_acetime` using AceTime library.
        - Rename `compare_c` to `compare_libc`
        - Rename `compare_cpp` to `compare_hinnant`
        - Rename `compare_go` to `compare_gotime`
    - `./validation`
        - Add files to support peer-to-peer validation.
        - Add 'make validation' target which compares raw JSON files from
          `compare_acetime`, `compare_acetimec`, `compare_acetz`, and
          `compare_hinnant`.
    - `./tests/`
        - Move top-level `BasicXxxTest` and `ExtendedXxxtest` to be under new
          `tests/` subdirectory.
        - Rename `{Basic,Extended}XxxTest` to `Xxx{Basic,Extended}Test`.
        - Add AcetimecExtendedTest to `tests` and `runtests` targets.
            - Assumes that the acetimec library will be kept up to date with the
              latest TZDB version.
    - Rename `AceTimePython` library to `acetimepy`.
    - Rename `AceTimeC` library to `acetimec`.
    - Rename `AceTimeGo` library to `acetimego`.
- 1.5.2 (2023-04-01, TZDB 2023c)
    - Upgrade TZDB from 2023b to 2023c.
        - https://mm.icann.org/pipermail/tz-announce/2023-March/000079.html
            - "This release's code and data are identical to 2023a.  In other
              words, this release reverts all changes made in 2023b other than
              commentary, as that appears to be the best of a bad set of
              short-notice choices for modeling this week's daylight saving
              chaos in Lebanon."
    - AceTimeValidation is forced to upgrade to 2023c, because we skipped 2023a
      and went directly to 2023b, which is being rolled back by 2023c.
- 1.5.1 (2023-03-24, TZDB 2023b)
    - Upgrade TZDB from 2022g to 2023b
        - 2023a skipped because it came out only a day earlier.
        - 2023a: https://mm.icann.org/pipermail/tz-announce/2023-March/000077.html
            - Egypt now uses DST again, from April through October.
            - This year Morocco springs forward April 23, not April 30.
            - Palestine delays the start of DST this year.
            - Much of Greenland still uses DST from 2024 on.
            - America/Yellowknife now links to America/Edmonton.
            - tzselect can now use current time to help infer timezone.
            - The code now defaults to C99 or later.
            - Fix use of C23 attributes.
        - 2023b: https://mm.icann.org/pipermail/tz-announce/2023-March/000078.html
            - Lebanon delays the start of DST this year.
- 1.5.0 (2023-02-12, TZDB 2022g)
    - Add `tools/compare_libc`
        - Generate validation data using the GNU libc functions of the C
          language.
    - Rename overloaded `tools/compare_xxx/generate_data.out` to
      `tools/compare_xxx/compare_xxx.out` for easier debugging of Makefiles and
      scripts.
- 1.4.0 (2023-02-02, TZDB 2022g)
    - `*Test/Makefile`
        - Upgrade to new `tzcompiler.sh` which requires explicit `--tzrepo`
          flag, and performs automatic clean up of temporary `tzfiles/`
          directory.
    - `compare_acetimec`
        - Upgrade to AceTimeC v0.6.0 which adds the `AtcTimeZone` struct.
    - `compare_noda`
        - Add `--epoch_year` to support AceTime v2.0.
        - Upgrade to DotNet 6.0.
    - Update the `UNTIL_YEAR` of most validation tests to 2100.
- 1.3.0 (2022-12-04, TZDB 2022g)
    - Add support for AceTime v2 for additional 3rd party timezone libraries:
        - Add `--epoch_year` flags to:
            - `compare_dateutil`, `compare_gotime`, `compare_java`,
              `compare_pytz`, `compare_zoneinfo`
        - Pass `--generate_int16_year` to `tzcompiler.py`:
            - `*DateUtilTest`, `*GoTest`, `*JavaTest`, `*PytzTest`,
              `*ZoneInfoTest`
    - Upgrade to TZDB 2022g
- 1.2.0 (2022-11-04, TZDB 2022f)
    - Support AceTime v2.0.0
        - Add `--epoch_year` flags
            - `compare_hinnant`, `compare_acetimec` and `compare_acetz`
        - Separate the validation start/until year params from the TZDB
          start/until year params
            - `*HinnantDateTest`, `*AceTimeCTest`, `*AcetzTest`
    - Upgrade TZDB from 2022e to 2022f
    - Upgrade TZDB from 2022d to 2022e
- 1.1.3 (2022-10-06, TZDB 2022d)
    - Add `BasicAceTimeCTest` and `ExtendedAceTimeCTest` which validate
      against the [AceTimeC](https://github.com/bxparks/AceTimeC) C library.
    - Update `copytz.sh` to avoid `flock(1)` which does not exist on MacOS.
    - Verify that `make validations` and `make runvalidations` work on MacOS
      11.6.8.
    - Upgrade TZDB from 2022b to 2022d
- 1.1.2 (2022-08-13, TZDB 2022b)
    - Upgrade to TZDB 2022b.
- 1.1.1 (2022-03-20, TZDB 2022a)
    - Update to TZDB 2022a.
    - No changes to code.
- 1.1.0 (2022-01-10, TZDB 2021e)
    - Move `tools/compare_xxx` scripts from `AceTimeTools`.
    - Move `tools/generate_validation` script from `AceTimeTools`.
    - Add validation against the Go lang `time` package:
        - Add `tools/compare_gotime` script
        - Add `BasicGoTimeTest` and `ExtendedGoTimeTest`.
    - Call `Print::setLineModeUnix()` from EpoxyDuino v1.2 to write Unix line
      terminator instead of DOS terminator.
- 1.0.0 (2021-12-08, TZDB 2021e)
    - Add `BasicZoneInfoTest` (2000 until 2050) and `ExtendedZoneInfoTest`
      (1974 until 2050) which validate against the Python 3.9 zoneinfo package.
    - Add new dependency to AceSorting library.
    - Rename `{Basic,Extended}PythonTest` to `{Basic,Extended}PytzTest`.
    - Validates against AceTime v1.9.0.
- 0.1.1 (2021-10-28, TZDB 2021e)
    - Update to TZDB 2021e.
- 0.1 (2021-10-06, TZDB 2021c)
    - Remove `--ignore_buf_size_too_large` flag because `zone_processor.py`
      now calculates the exact maximum buffer sizes.
    - Add references to `BasicAcetzTest` and `ExtendedAcetzTest` into README.md.
    - Upgrade `*AcetzTest` and `*HinnantDateTest` to TZDB 2021c.
- (2021-08-26)
    - Split from [AceTime](https://github.com/bxparks/AceTime) using `git
      subtree split` to preserve history.
