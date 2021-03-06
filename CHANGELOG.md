# Changelog

* Unreleased
* v1.1.1 (2022-03-20, TZDB 2022a)
    * Update to TZDB 2022a.
        * https://mm.icann.org/pipermail/tz-announce/2022-March.txt
        * "Palestine will spring forward on 2022-03-27, not -03-26."
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
        * https://mm.icann.org/pipermail/tz-announce/2021-October/000069.html
        * Palestine will fall back 10-29 (not 10-30) at 01:00.
* v0.1 (2021-10-06, TZDB 2021c)
    * Remove `--ignore_buf_size_too_large` flag because `zone_processor.py`
      now calculates the exact maximum buffer sizes.
    * Add references to `BasicAcetzTest` and `ExtendedAcetzTest` into README.md.
    * Upgrade `*AcetzTest` and `*HinnantDateTest` to TZDB 2021c.
* (2021-08-26)
    * Split from [AceTime](https://github.com/bxparks/AceTime) using `git
      subtree split` to preserve history.
