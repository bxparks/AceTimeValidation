# ExtendedPytzTest

This unit test compares the DST transitions calculated by the
`ExtendedZoneProcessor` class with the `validation_data.cpp` file generated by
[compare_pytz](https://github.com/bxparks/AceTimeTools/tree/master/compare_pytz)
which uses the [pytz](https://pypi.org/project/pytz/) library.

## Running the Test

The Python tool generates about 200,000 data points spanning the year 2000 to
2038. It is too large to run on any Arduino board that I am aware of, including
the ESP32.

The unit test does run on a Linux machine using the
[EpoxyDuino](https://github.com/bxparks/EpoxyDuino) adapter layer.
Assuming that you have `g++` and `make` installed, just type:

```
$ make

$ make runtests
TestRunner started on 348 test(s).
Test ExtendedTransitionTest_Africa_Abidjan passed.
...
Test ExtendedTransitionTest_Pacific_Wallis passed.
TestRunner duration: 0.113 seconds.
TestRunner summary: 348 passed, 0 failed, 0 skipped, 0 timed out, out of 348
test(s).

$ make clean
```