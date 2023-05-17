# AceTime Validation

This project consists of 2 sets of validation tests:

* [./tests/](tests)
* [./validation/](validation)

Both of them use the scripts and binaries in `tools` directory:

* [./tools](tools)

The primary purpose of `tests/` is to test the 2 algorithms
(`BasicZoneProcessor` and `ExtendedZoneProcessor`) implemented by the
[AceTime](https://github.com/bxparks/AceTime) Arduino library. Those algorithms
are tested against other third party timezone libraries. These tests are written
as [AUnit](https://github.com/bxparks/AUnit) unit tests running on a Linux box
using the [EpoxyDuino](https://github.com/bxparks/EpoxyDuino) framework.

The AceTime library inspired additional first-party timezone libraries for
different languages:

* [AceTimePython](https://github.com/bxparks/AceTimePython) for Python
* [AceTimeC](https://github.com/bxparks/AceTimeC) for C
* [AceTimeGo](https://github.com/bxparks/AceTimeGo) for Go and TinyGo

These additional libraries required more general validation tests which are
placed in the `validations/` subdirectory. The purpose that infrastructure is to
allow easier peer-to-peer testing of any timezone library against any other
timezone library.

The timezone libraries supported by this project are:

* First party (libraries written by me)
    * [AceTime](https://github.com/bxparks/AceTime) for Arduino
    * [AceTimeC](https://github.com/bxparks/AceTimeC) for C
    * [AceTimePython](https://github.com/bxparks/AceTimePython) for Python
    * [AceTimeGo](https://github.com/bxparks/AceTimeGo) for Go and TinyGo
* Third party (libraries written by others)
    * [GNU libc](https://www.gnu.org/software/libc/manual/2.35/html_node/Calendar-Time.html) for C lang
    * [dateutil](https://pypi.org/project/python-dateutil/) for Python
    * [Go Lang time](https://pkg.go.dev/time) for Go lang
    * [Hinnant date](https://github.com/HowardHinnant/date) for C++
    * [Java 11 Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html) for Java
    * [Noda Time](https://nodatime.org) for C#
    * [pytz](https://pypi.org/project/pytz/) for Python
    * [zoneinfo](https://docs.python.org/3/library/zoneinfo.html) for Python

Each supported timezone library is represented by an associated `compare_xxx`
tool under the [./tools/](tools) directeory:

* First party (libraries written by me)
    * `compare_acetime` - AceTime library
    * `compare_acetimec` - AceTimeC library
    * `compare_acetz` - AceTimePython library
    * `compare_gotz` - (TBD) AceTimeGo library
* Third party (libraries written by others)
    * `compare_libc` - C GNU libc library
    * `compare_dateutil` - Python `python-dateutil` library
    * `compare_gotime` - Go Lang time library
    * `compare_hinnant` - C++ Hinnant Date library
    * `compare_java` - Java JDK11 `java.time` library
    * `compare_noda` - Noda Time library
    * `compare_pytz` - Python `pytz` library
    * `compare_zoneinfo` - Python 3.9 `zoneinfo` library

The purpose of the `compare_xxx` scripts is to produce a list of all DST
transitions over the given year range, for the requested timezones. These
transitions are saved as a JSON file usually named `validation_data.json`. These
JSON files are consumed by:

* the `./tests` directory to programmatically generate the AUnit unit tests, and
* the `./validation` directory to generate difference reports between pairs of
  timezone libraries.

**Version**: v1.5.2 (2023-04-01, TZDB 2023c)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

## Table of Contents

* [Overview](#Overview)
* [Running the Tests](#RunningTests)
* [Running the Validation](#RunningValidation)
* [Bugs and Limitations](#BugsAndLimitations)
* [License](#License)
* [Feedback and Support](#FeedbackAndSupport)
* [Authors](#Authors)

<a name="CompilingTools"></a>
## Compiling the Tools

See [tools](tools) to compile and prepare the various scripts and binaries.
Basically:

```
$ cd tools
$ make
```

<a name="RunningTests"></a>
## Running the Tests

The `make tests` target runs only a subset of the integration tests which
validate against libraries that are known to work when a new TZDB version is
released:

* `AcetzExtendedTest`
* `HinnantExtendedTest`

(The `AcetzBasicTest` and `HinnantBasicTest` are no longer run to reduce the
execution time of the `make tests` target.)

The other libraries instead rely on the TZDB version that is installed on the
underlying OS, but when a new TZDB version is released, these libraries usually
break because the OS version becomes outdated.

Assuming that all the tools are built, validation tests can be run by:

```
$ cd tests
$ make clean
$ make tests
$ make runtests
```

See additional information in [tests/README.md](tests).

<a name="RunningValidation"></a>
## Running the Validation

```
$ cd validation
$ make diff_acetime
$ make diff_acetz
$ make diff_dateutil
$ make diff_gotime
$ make diff_hinnant
$ make diff_java
$ make diff_libc
$ make diff_libc
$ make diff_noda
$ make diff_pytz
$ make diff_zoneinfo
```

See additional information in [validation/README.md](validation).

<a name="BugsAndLimitations"></a>
## Bugs and Limitations

* Writing these validation tests was challenging, probably taking up 2-3X more
  effort than the core of the library.
    * I think the reason is that the number of input variables into the library
      and the number of output variables are substantially large, making it
      difficult to write isolated unit tests.
    * Secondly, the TZ Database zone files are deceptively easy to read by
      humans, but contain many implicit rules that are difficult to translate
      into computer algorithms, creating a large number of code paths to test.
* The `./validation` suite scans only the 100 years over `[2000, 2100)`
  interval.
    * Additional tests should be written to validate the entire range supported
      by the IANA TZDB: [1844, 2087]

<a name="License"></a>
## License

[MIT License](https://opensource.org/licenses/MIT)

<a name="FeedbackAndSupport"></a>
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

<a name="Authors"></a>
## Authors

* Created by Brian T. Park (brian@xparks.net).
