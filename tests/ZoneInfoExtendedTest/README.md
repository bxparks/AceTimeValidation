# ZoneInfoExtendedTest

This unit test compares the DST transitions calculated by the
`ExtendedZoneProcessor` class with the `validation_data.cpp` file generated by
the [tools/compare_zoneinfo](../../tools/compare_zoneinfo) program which uses
the [zoneinfo](https://docs.python.org/3/library/zoneinfo.html) library first
included with Python 3.9. The
[backports.zoneinfo](https://pypi.org/project/backports.zoneinfo/) package is
used for Python 3.7 and 3.8.

## Running the Test

The validation test runs on a Linux machine using the
[EpoxyDuino](https://github.com/bxparks/EpoxyDuino) adapter layer.
Assuming that you have `g++` and `make` installed, just type:

```
$ make clean
$ make

$ make run
TestRunner started on 377 test(s).
Test ExtendedTransitionTest_Africa_Abidjan passed.
...
Test ExtendedTransitionTest_WET passed.
TestRunner duration: 2.292 seconds.
TestRunner summary: 377 passed, 0 failed, 0 skipped, 0 timed out, out of 377
test(s).
```
