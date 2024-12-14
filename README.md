# AceTime Validation

This project consists of 2 sets of validation tests for the various libraries
in the AceTime family:

- [./tests/](tests) (testing BasicZoneProcessor and ExtendedZoneProcessor of
  AceTime)
- [./validation/](validation) (peer-to-peer testing)

Both of them use the scripts and binaries in `tools` directory:

- [./tools](tools)

**Version**: v1.8.0 (2024-12-13, TZDB 2024b)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

## Table of Contents

- [Overview](#overview)
    - [Tests](#tests)
    - [Validation](#validation)
    - [Tools](#tools)
- [Compiling the Tools](#compiling-the-tools)
- [Running the Tests](#running-the-tests)
- [Running the Validation](#running-the-validation)
- [Bugs and Limitations](#bugs-and-limitations)
- [License](#license)
- [Feedback and Support](#feedback-and-support)
- [Authors](#authors)

## Overview

### Tests

The primary purpose of `tests/` is to test the 2 algorithms implemented in the
[AceTime](https://github.com/bxparks/AceTime) Arduino library by the following
classes:

- `BasicZoneProcessor`
- `ExtendedZoneProcessor`

They are tested against 3 other libraries, which are assumed to be accurate:

- acetimec (C)
- acetimepy (Python)
- Hinnant date (C++)

These 3 were chosen because they have the capacity to update their zone database
directly from the TZDB, so their unit tests will likely pass when a new TZDB
release is created. The other 3rd party libraries depend on a release cycle, so
will often be pegged to an older version of TZDB, which will cause the tests to
fail.

These tests are written as [AUnit](https://github.com/bxparks/AUnit) unit tests
running on a Linux box using the
[EpoxyDuino](https://github.com/bxparks/EpoxyDuino) framework.

### Validation

The tests under `validation/` allow general peer-to-peer comparisons between any
2 timezone libraries. In practice, it's easier to select one of the libraries
as the "baseline" or "golden" data set, and compare the others against it.

The timezone libraries supported are:

- First party (libraries written by me)
    - [AceTime](https://github.com/bxparks/AceTime) for Arduino
    - [acetimec](https://github.com/bxparks/acetimec) for C
    - [acetimepy](https://github.com/bxparks/acetimepy) for Python
    - [acetimego](https://github.com/bxparks/acetimego) for Go and TinyGo
- Third party (libraries written by others)
    - [GNU libc](https://www.gnu.org/software/libc/manual/2.35/html_node/Calendar-Time.html) for C lang
    - [dateutil](https://pypi.org/project/python-dateutil/) for Python
    - [Go Lang time](https://pkg.go.dev/time) for Go lang
    - [Hinnant date](https://github.com/HowardHinnant/date) for C++
    - [Java 11 Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html) for Java
    - [Noda Time](https://nodatime.org) for C#
    - [pytz](https://pypi.org/project/pytz/) for Python
    - [zoneinfo](https://docs.python.org/3/library/zoneinfo.html) standard
      library for Python 3.9

### Tools

Each supported timezone library is represented by an associated `compare_xxx`
tool under the [./tools/](tools) directory:

- First party (libraries written by me)
    - `compare_acetime` - AceTime library
    - `compare_acetimec` - acetimec library
    - `compare_acetz` - acetimepy library
    - `compare_acetimego` - acetimego library
- Third party (libraries written by others)
    - `compare_libc` - C GNU libc library
    - `compare_dateutil` - Python `python-dateutil` library
    - `compare_gotime` - Go Lang time library
    - `compare_hinnant` - C++ Hinnant Date library
    - `compare_java` - Java JDK11 `java.time` library
    - `compare_noda` - Noda Time library
    - `compare_pytz` - Python `pytz` library
    - `compare_zoneinfo` - Python 3.9 `zoneinfo` library

The purpose of the `compare_xxx` scripts is to produce a list of all DST
transitions over the given year range, for the requested timezones. These
transitions are saved as a JSON file usually named `validation_data.json`. These
JSON files are consumed by the various scripts in `./tests` and `./validation`.

## Compiling the Tools

See [tools](tools) to compile and prepare the various scripts and binaries.
Basically:

```
$ cd tools
$ make clean
$ make
```

## Running the Tests

To run the "tests":

```
$ cd tests
$ make clean
$ make -j2 tests
$ make runtests
```

The `make tests` target runs only a subset of the integration tests which
validate against libraries that are known to work when a new TZDB version is
released:

- `AcetimecBasicTest`
- `AcetimecExtendedTest`
- `AcetzBasicTest`
- `AcetzExtendedTest`
- `HinnantBasicTest`
- `HinnantExtendedTest`

The other libraries instead rely on the TZDB version that is installed on the
underlying OS, but when a new TZDB version is released, these libraries usually
break because the OS version becomes outdated.

See additional information in [tests/README.md](tests).

## Running the Validation

To run the peer-to-peer validations:

```
$ cd validation
$ vi Makefile # update TZDB_VERSION
$ make clean
$ make -j2 validation
```

The "baseline" is selected to be the `acetime_complete.json` target which comes
from the AceTime library, from the year 1800 to 2200.

The `validation` target runs the following against the baseline:

- `diff_acetime_basic`
- `diff_acetime_extended`
- `diff_acetime_complete`
- `diff_acetimec`
- `diff_acetimego`
- `diff_acetimepy`
- `diff_hinnant`

See additional information in [validation/README.md](validation).

## License

[MIT License](https://opensource.org/licenses/MIT)

## Feedback and Support

If you have any questions, comments, or feature requests for this library,
please use the [GitHub
Discussions](https://github.com/bxparks/AceTimeValidations/discussions) for this
project. If you have bug reports, please file a ticket in [GitHub
Issues](https://github.com/bxparks/AceTimeValidations/issues). Feature requests
should go into Discussions first because they often have alternative solutions
which are useful to remain visible, instead of disappearing from the default
view of the Issue tracker after the ticket is closed.

Please refrain from emailing me directly unless the content is sensitive. The
problem with email is that I cannot reference the email conversation when other
people ask similar questions later.

## Authors

* Created by Brian T. Park (brian@xparks.net).
