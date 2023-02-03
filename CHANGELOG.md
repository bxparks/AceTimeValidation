# Changelog

* Unreleased
* v1.4.0 (2023-02-02, TZDB 2022g)
    * `*Test/Makefile`
        * Upgrade to new `tzcompiler.sh` which requires explicit `--tzrepo`
          flag, and performs automatic clean up of temporary `tzfiles/`
          directory.
    * `compare_acetimec`
        * Upgrade to AceTimeC v0.6.0 which adds the `AtcTimeZone` struct.
    * `compare_noda`
        * Add `--epoch_year` to support AceTime v2.0.
        * Upgrade to DotNet 6.0.
    * Update the `UNTIL_YEAR` of most validation tests to 2100.
* v1.3.0 (2022-12-04, TZDB 2022g)
    * Add support for AceTime v2 for additional 3rd party timezone libraries:
        * Add `--epoch_year` flags to:
            * `compare_dateutil`, `compare_go`, `compare_java`, `compare_pytz`,
              `compare_zoneinfo`
        * Pass `--generate_int16_year` to `tzcompiler.py`:
            * `*DateUtilTest`, `*GoTest`, `*JavaTest`, `*PytzTest`,
              `*ZoneInfoTest`
    * Upgrade to TZDB 2022g
* v1.2.0 (2022-11-04, TZDB 2022f)
    * Support AceTime v2.0.0
        * Add `--epoch_year` flags
            * `compare_cpp`, `compare_acetimec` and `compare_acetz`
        * Separate the validation start/until year params from the TZDB
          start/until year params
            * `*HinnantDateTest`, `*AceTimeCTest`, `*AcetzTest`
    * Upgrade TZDB from 2022e to 2022f
    * Upgrade TZDB from 2022d to 2022e
* v1.1.3 (2022-10-06, TZDB 2022d)
    * Add `BasicAceTimeCTest` and `ExtendedAceTimeCTest` which validate
      against the [AceTimeC](https://github.com/bxparks/AceTimeC) C library.
    * Update `copytz.sh` to avoid `flock(1)` which does not exist on MacOS.
    * Verify that `make validations` and `make runvalidations` work on MacOS
      11.6.8.
    * Upgrade TZDB from 2022b to 2022d
* v1.1.2 (2022-08-13, TZDB 2022b)
    * Upgrade to TZDB 2022b.
* v1.1.1 (2022-03-20, TZDB 2022a)
    * Update to TZDB 2022a.
    * No changes to code.
* v1.1.0 (2022-01-10, TZDB 2021e)
    * Move `tools/compare_xxx` scripts from `AceTimeTools`.
    * Move `tools/generate_validation` script from `AceTimeTools`.
    * Add validation against the Go lang `time` package:
        * Add `tools/compare_go` scrip
        * Add `BasicGoTest` and `ExtendedGoTest`.
    * Call `Print::setLineModeUnix()` from EpoxyDuino v1.2 to write Unix line
      terminator instead of DOS terminator.
* v1.0.0 (2021-12-08, TZDB 2021e)
    * Add `BasicZoneInfoTest` (2000 until 2050) and `ExtendedZoneInfoTest`
      (1974 until 2050) which validate against the Python 3.9 zoneinfo package.
    * Add new dependency to AceSorting library.
    * Rename `{Basic,Extended}PythonTest` to `{Basic,Extended}PytzTest`.
    * Validates against AceTime v1.9.0.
* v0.1.1 (2021-10-28, TZDB 2021e)
    * Update to TZDB 2021e.
* v0.1 (2021-10-06, TZDB 2021c)
    * Remove `--ignore_buf_size_too_large` flag because `zone_processor.py`
      now calculates the exact maximum buffer sizes.
    * Add references to `BasicAcetzTest` and `ExtendedAcetzTest` into README.md.
    * Upgrade `*AcetzTest` and `*HinnantDateTest` to TZDB 2021c.
* (2021-08-26)
    * Split from [AceTime](https://github.com/bxparks/AceTime) using `git
      subtree split` to preserve history.
