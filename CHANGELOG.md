# Changelog

* Unreleased
    * Upgrade TZDB from 2022d to 2022e
        * https://mm.icann.org/pipermail/tz-announce/2022-October/000074.html
            * Jordan and Syria switch from +02/+03 with DST to year-round +03.
* v1.1.3 (2022-10-06, TZDB 2022d)
    * Add `BasicAceTimeCTest` and `ExtendedAceTimeCTest` which validate
      against the [AceTimeC](https://github.com/bxparks/AceTimeC) C library.
    * Update `copytz.sh` to avoid `flock(1)` which does not exist on MacOS.
    * Verify that `make validations` and `make runvalidations` work on MacOS
      11.6.8.
    * Upgrade TZDB from 2022b to 2022d
        * 2022c
            * https://mm.icann.org/pipermail/tz-announce/2022-August/000072.html
                * Work around awk bug in FreeBSD, macOS, etc.
                * Improve tzselect on intercontinental Zones.
            * Skipped because there were no changes that affected AceTime.
        * 2022d
            * https://mm.icann.org/pipermail/tz-announce/2022-September/000073.html
                * Palestine transitions are now Saturdays at 02:00.
                * Simplify three Ukraine zones into one.
* v1.1.2 (2022-08-13, TZDB 2022b)
    * Upgrade to TZDB 2022b.
        * https://mm.icann.org/pipermail/tz-announce/2022-August/000071.html
            * Chile's DST is delayed by a week in September 2022.
            * Iran no longer observes DST after 2022.
            * Rename Europe/Kiev to Europe/Kyiv.
            * Finish moving duplicate-since-1970 zones to 'backzone'.
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
