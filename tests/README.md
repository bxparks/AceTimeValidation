# Validation Tests

This directory contains a number of [AUnit](https://github.com/bxparks/AUnit)
tests which validates the [AceTime](https://github.com/bxparks/AceTime) Arduino
library against a number of reference timezone libraries:

- [acetimec](https://github.com/bxparks/acetimec) for C
- [dateutil](https://pypi.org/project/python-dateutil/) for Python
- [Go Lang time](https://pkg.go.dev/time) for Go lang
- [Hinnant date](https://github.com/HowardHinnant/date) for C++
- [Java 11 Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html) for Java
- [pytz](https://pypi.org/project/pytz/) for Python
- [zoneinfo](https://docs.python.org/3/library/zoneinfo.html) library for Python

## File Structure

Each test is composed of:

* static files checked into git
    * `XxxTest.ino`
    * `Makefile` for EpoxyDuino
* programmatically generated files from the **reference** library
    * `validation_data.json`
    * `validation_data.h`
    * `validation_data.cpp`
    * `validation_tests.cpp`

The `validation_data.json` file is produced from the **reference** library by
one of the `compare_xxx` scripts or binaries in the [tools](../tools) directory.
It contains the list of DST transitions for each timezone as determined by the
library invoked by the `compare_xxx` script.

The `validation_tests.cpp` file contains a list of unit tests which compare the
expected timezone transition data from the **reference** library against the
same values dynamically calculated at the given date and time using AceTime.

The unit tests are expected to be compiled and executed in a Linux environment
using the [EpoxyDuino](https://github.com/bxparks/EpoxyDuino) adapter layer
because the generated binary will almost always be too large to run on the
microcontroller itself.

## Running the Tests

1)) rebuild the `AceTimeValidation/tools` binaries:
```
$ cd AceTimeValidation/tools
$ make clean
$ make
```

2)) Build a specific test. For example, `HinnantExtendedTest`:

```
$ cd AceTimeValidation/tests/HinnantExtendedTest
$ vi Makefile
# update TZ_VERSION
$ make clean
$ make run
```

## Data Flow

Here is a partial data flow diagram:

```
AceTimeTools
   |
   v
zones.txt
   |
   |     java.time
   |        |
   |        v
   +--> compare_java/GenerateData.java ----------.
   |                                             |
   |                                             |
   |    Hinnant/date                             |
   |        |                                    |
   |        v                                    |
   +--> compare_hinnant/compare_hinnant.out ---> +
   |                                             |
   |                                             |
   |         pytz                                |
   |          |                                  |
   |          v         date_types/              |
   |   tdgenerator.py   validation_types.py      |
   |          |         /                        |
   |          v        v                         |
   +--> compare_pytz/generate_data.py ---------> +
   |                                             |
   |                                             |
   |   python-dateutil                           |
   |          |                                  |
   |          v         data_types/              |
   |   tdgenerator.py   validation_types.py      |
   |          |         /                        |
   |          v        v                         |
   +--> compare_dateutil/generate_data.py -----> +
   |        ...                                  |
   |        ...                                  |
   |        ...                                  |
   |                                             |
   |   tzdata{yyyya}.tar.gz                      |
   |     TzdbCompiler                            |
   |   Noda Time library                         |
   |          |                                  |
   |          v                                  |
   +--> compare_noda/Program.cs ---------------> +
                                                 |
                                                 v
                                        validation_data.json
                                                 |
                                                 v
        compare_*/blacklist.json ----> generate_validation.py
                                                 |
                                                 v
                                        validation_data.{h,cpp}
                                        validation_tests.cpp        AUnit
                                                 |                  /
                                                 |    .-------------
                                                 v   v
                                            EpoxyDuino
                                                 |
                                                 v
                                             make
                                             make run
```
