# AcetzBasicTest

This unit test compares the DST transitions calculated by the
`BasicZoneProcessor` class with the `validation_data.cpp` file generated by the
[tools/compare_acetz](../../tools/compare_acetz) program which uses the `acetz`
class of the [AceTimePython](https://github.com/bxparks/AceTimePython) library.

## Running the Test

The unit test does run on a Linux machine using the
[EpoxyDuino](https://github.com/bxparks/EpoxyDuino) adapter layer.
Assuming that you have `g++` and `make` installed, just type:

```
$ make clean
$ make

$ make run
TestRunner started on 266 test(s).
Test BasicTransitionTest_Africa_Abidjan passed.
...
Test BasicTransitionTest_WET passed.
TestRunner duration: 1.614 seconds.
TestRunner summary: 266 passed, 0 failed, 0 skipped, 0 timed out, out of 266 test(s).
```