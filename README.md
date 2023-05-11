# AceTime Validation

These are integration tests to validate the algorithms in the
[AceTime](https://github.com/bxparks/AceTime) library which are mostly
implemented by the `ZonedDateTime` and `ZoneProcessor` classes. The AceTime
algorithms are compared to the following third party timezone libraries:

* [Python pytz](https://pypi.org/project/pytz/) library
* [Python dateutil](https://pypi.org/project/python-dateutil/) library
* [Python zoneinfo](https://docs.python.org/3/library/zoneinfo.html) library
* [AceTimePython](https://github.com/bxparks/AceTimePython) library
* [Java 11 Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html) library.
* [Hinnant date](https://github.com/HowardHinnant/date) C++ library
* [Noda Time](https://nodatime.org) C# library
* [Go Lang time](https://pkg.go.dev/time) package
* [AceTimeC](https://github.com/bxparks/AceTimeC) library
* [GNU libc](https://www.gnu.org/software/libc/manual/2.35/html_node/Calendar-Time.html) C library

These integration tests require a desktop-class machine running Linux or MacOS.
They are too big to run on any Arduino microcontroller that I know of. They use
the [EpoxyDuino](https://github.com/bxparks/EpoxyDuino) emulation layer to run
these programs on the desktop machine. They also use various files (e.g.
`validation_data.h`, `validation_data.cpp`, `validation_tests.cpp`) which are
*generated* dynamically by the various `Makefile` files. (These files used to be
manually generated, then checked into source control. But after it was clear
that no Arduino microcontroller would be able to run these tests, it did not
seem worth checking in the generated code.)

Writing tests for this library was challenging, probably taking up 2-3X more
effort than the core of the library. I think the reason is that the number of
input variables into the library and the number of output variables are
substantially large, making it difficult to write isolated unit tests. Secondly,
the TZ Database zone files are deceptively easy to read by humans, but contain
many implicit rules that are difficult to translate into computer algorithms,
creating a large number of code paths to test.

Testing against third party libraries seem the only reasonable way to validate
the complex algorithms implemented by the `BasicZoneProcessor` and
`ExtendedZoneProcessor`) classes in AceTime. It is impractical to manually
create the inputs and expected outputs. The calculation of one data point can
take several minutes manually. Initially, I wrote the 2 implementations of
`ZoneProcessor` thinking that I could validate one against the other. (I think I
wrote 4-5 different versions altogether, of which only 2 made it into this
library). However, it turned out that the number of timezones supported by the
`ExtendedZoneProcessor` was much larger than the ones supported by
`BasicZoneProcessor` so it became infeasible to test the non-overlapping
timezones.

**Version**: v1.5.2 (2023-04-01, TZDB 2023c)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

## Table of Contents

* [Overview](#Overview)
* [Dependencies](#Dependencies)
* [Compiling and Running](#CompilingAndRunning)
    * [Prerequisites](#Prerequisites)
    * [Running CI Validations](#RunningCIValidations)
    * [Running All Validations](#RunningAllValidations)
* [Validation Tests](#ValidationTests)
    * [Python pytz](#TestPythonPytz)
    * [Python dateutil](#TestPythonDateUtil)
    * [AceTimePython](#TestAceTimePython)
    * [Java java.time](#TestJavaTime)
    * [C++ Hinnant Date](#TestHinnantDate)
    * [Noda Time](#TestNodaTime)
    * [Go Lang Time](#TestGoLangTime)
    * [AceTimeC](#TestAceTimeC)
* [License](#License)
* [Feedback and Support](#FeedbackAndSupport)
* [Authors](#Authors)

<a name="Overview"></a>
## Overview

These scripts live under the `AceTimeValidation/tools/` directory:

* `compare_acetimec` - AceTimeC library
* `compare_acetz` - AceTimePython library
* `compare_libc` - C GNU libc library
* `compare_hinnant` - C++ Hinnant Date library
* `compare_dateutil` - Python `python-dateutil` library
* `compare_go` - Go Lang time library
* `compare_java` - Java JDK11 `java.time` library
* `compare_noda` - Noda Time library
* `compare_pytz` - Python `pytz` library
* `compare_zoneinfo` - Python 3.9 `zoneinfo` library

Each script reads the `zones.txt` file from the stdin. This file was generated
from `zonelistgenerator.py` through the `tzcompiler.py` script in the
AceTimeTools project. The `zones.txt` file contains the time zones which should
be processed by the `compare_xxx` scripts.

The output of each of these scripts is a `validation_data.json` file.

The `validation_data.json` contains a list of `TestItem` records, each
containing the `epoch_seconds` and the corresponding date/time component as
calculated from various 3rd party date/time libraries.

The `generate_validation.py` script reads the `validation_data.json` and
produces a set of Arduino-compatible C++ files:

* `validation_data.cpp`
* `validation_data.h`
* `validation_tests.cpp`

Executing this code-generated program under EpoxyDuino verifies if the date-time
components produced by the third party library matches the components calculated
by the AceTime algorithms.

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
   |                                             |
   |                                             |
   |   tzdata{yyyya}.tar.gz                      |
   |          |                                  |
   |          v                                  |
   |     TzdbCompiler                            |
   |          |                                  |
   |          v                                  |
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
                                        validation_tests.cpp
```

The `AceTimeValidation` project contains a set of Arduino-compatible projects
which can be compiled using EpoxyDuino, to validate the various third party
libraries.

<a name="Dependencies"></a>
## Dependencies

* Ubuntu 22.04 or MacOS 11.6.8 (Big Sur)
* Python 3.7 or higher
* [AceTime](https://github.com/bxparks/AceTime)
    * As sibling project to AceTimeValidation
* [AceTimeTools](https://github.com/bxparks/AceTimeTools)
    * As sibling project to AceTimeValidation
    * This has not yet been uploaded to PyPI, so the installation process is
      manual.
    * `$ git clone https://github.com/bxparks/AceTimeTools
    * `$ cd AceTimeTools`
    * `$ pip3 install --user -e .`
* [AceTimePython](https://github.com/bxparks/AceTimePython) library
    * This has not yet been uploaded to PyPI, so the installation process is
      manual.
    * `$ git clone https://github.com/bxparks/AceTimePython`
    * `$ cd AceTimePython`
    * `$ pip3 install --user -e .`
* [EpoxyDuino](https://github.com/bxparks/EpoxyDuino)
    * As sibling project to AceTimeValidation
    * The AceTime library and the unit tests generated by the
      AceTimeValidation project are Arduino programs. EpoxyDuino allows these to
      be compiled and executed on a Linux (or MacOS) environment.
* [IANA TZDB](https://github.com/eggert/tz)
    * As sibling project to AceTimeValidation
    * `$ git clone https://github.com/eggert/tz`
* [Python pytz](https://pypi.org/project/pytz/) library
    * `$ pip3 install --user pytz`
* [Python dateutil](https://pypi.org/project/python-dateutil/) library
    * `$ pip3 install --user python-dateutil`
* [Java 11 Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html) library.
    * `$ sudo apt install openjdk-11-jdk`
* [Hinnant date](https://github.com/HowardHinnant/date) C++ library
    * `git clone https://github.com/HowardHinnant/date`
* libcurl library
    * `$ sudo apt install libcurl4-openssl-dev`
    * Required by the Hinnant date library
* [Noda Time](https://nodatime.org) C# library
    * Requires the .Net 6.0 framework
        * https://docs.microsoft.com/en-us/dotnet/core/install/linux-ubuntu
* [Go Lang 1.17 or higher](https://go.dev/doc/install)
* [AceTimeC](https://github.com/bxparks/AceTimeC)
    * `$ git clone https://github.com/bxparks/AceTimeC`

<a name="CompilingAndRunning"></a>
## Compiling and Running

<a name="Prerequisites"></a>
### Prerequisites

The required Python, Java and C++ tools and libraries are explained in:

* [compare_acetimec](tools/compare_acetimec)
* [compare_acetz](tools/compare_acetz)
* [compare_dateutil](tools/compare_dateutil)
* [compare_go](tools/compare_go)
* [compare_hinnant](tools/compare_hinnant)
* [compare_java](tools/compare_java)
* [compare_libc](tools/compare_libc)
* [compare_noda](tools/compare_noda)
* [compare_pytz](tools/compare_pytz)
* [compare_zoneinfo](tools/compare_zoneinfo)

The various `Makefile` files under the subdirectories here will run `make -C` in
those directories to build the Java and C++ binaries as necessary. Here is a
(potentially out of date) summary of the 3rd party prerequisites:

1. Install EpoxyDuino as a sibling project to `AceTime`:
    * `$ git clone https://github.com/bxparks/EpoxyDuino`
1. Clone the IANA TZ database as a sibling project to `AceTime`:
    * `$ git clone https://github.com/eggert/tz`
1. Install the Python `pytz` and `dateutil` libraries:
    * `$ pip3 install --user pytz python-dateutil`
1. Install the Java 11 JDK:
    * `$ sudo apt install openjdk-11-jdk` (Ubuntu 22.04)
1. Clone the Hinnant date library as a sibling to the `AceTime` directory, and
   install the `libcurl4` library that it requires to download the tzfiles:
    * `$ git clone https://github.com/HowardHinnant/date`
    * `$ sudo apt install libcurl4-openssl-dev`
1. Install Microsoft .NET 6.0 on Linux:
    * See https://docs.microsoft.com/en-us/dotnet/core/install/linux
    * The `compare_noda.csproj` file already contains the NuGet dependency
      to Noda Time and will be automatically retrieved by the `dotnet build`
      command.

<a name="RunningCIValidations"></a>
### Running CI Validations

The `make validations` target runs only a subset of the integration tests which
validate against libraries that are known to work when a new TZDB version is
released:

* AcetzBasicTest
* AcetzExtendedTest
* HinnantBasicTest
* HinnantExtendedTest

That's because these libraries allow the IANA TZDB version to be explicitly
specified. The other libraries instead rely on the TZDB version that is
installed on the underlying OS, but when a new TZDB version is released, these
libraries usually break because the OS version becomes outdated.

```
$ make clean
$ make validations
$ make runvalidations
```

<a name="RunningAllValidations"></a>
### Running All Validations

You can run all validation tests in this project by running the following
commands:

```
$ make clean
$ make allvalidations
$ make runallvalidations
```

Some of them will often fail because of bugs in the 3rd party library, or
because the library has not upgraded its version of the IANA TZ database, or
because the library relies on the outdated version of the TZDB installed on the
host OS.

<a name="ValidationTests"></a>
## Validation Tests

<a name="TestPythonPytz"></a>
### Python pytz

The Python pytz library was a natural choice since the `tzcompiler.py` was
already written in Python. I created:

* [PytzBasicTest](tests/PytzBasicTest/)
* [PytzExtendedTest](tests/PytzExtendedTest/)

The `pytz` library is used to generate various C++ source code
(`validation_data.cpp`, `validation_data.h`, `validation_tests.cpp`) which
contain a list of epochSeconds, the UTC offset, the DST offset, at DST
transition points, for all timezones. The integration test then compiles in the
`ZonedDateTime` and verifies that the expected DST transitions and date
components are identical.

The resulting data test set contains between 150k to 220k data points, and can
no longer fit in any Arduino microcontroller that I am aware of. They can be
executed only on desktop-class Linux or MacOS machines through the use of the
[EpoxyDuino](https://github.com/bxparks/EpoxyDuino) emulation framework.

The `pytz` library supports [dates only until
2038](https://answers.launchpad.net/pytz/+question/262216). It is also tricky to
match the `pytz` version to the TZ Database version used by AceTime. The
following versions of `pytz` have been tested:

* pytz 2019.1, containing TZ Datbase 2019a
* pytz 2019.2, containing TZ Datbase 2019b
* pytz 2019.3, containing TZ Datbase 2019c

A number of zones did not match between pytz and AceTime. Those
have been listed in the `compare_pytz/blacklist.json` file.

<a name="TestPythonDateUtil"></a>
### Python dateutil

Validation against the Python dateutil library is similar to pytz. I created:

* [DateUtilBasicTest](tests/DateUtilBasicTest/)
* [DateUtilExtendedTest](tests/DateUtilExtendedTest/)

Similar to the `pytz` library, the `dateutil` library supports [dates only until
2038](https://github.com/dateutil/dateutil/issues/462). The
following versions of `dateutil` have been tested:

* dateutil 2.8.1, containing TZ Datbase 2019c

A number of zones did not match between dateutil and AceTime. Those
have been listed in the `compare_dateutil/blacklist.json` file.

<a name="TestAceTimePython"></a>
### AceTimePython

These validate against the `acetz` class of the
[AceTimePython](https://github.com/bxparks/AceTimePython) library whose
`zone_processor.py` module implements the exact same algorithm as the
`ExtendedZoneProcessor.h` file in the
[AceTime](https://github.com/bxparks/AceTime) library. The `zone_processor.py`
algorithm is exposed through the `acetz` subclass of the `tzinfo` class of the
standard `datetime` library.

* [AcetzBasicTest](tests/AcetzBasicTest/)
* [AcetzExtendedTest](tests/AcetzExtendedTest/)

If the zonedb files in the `src/acetime/zonedb/` directory of the
`AceTimePython` library is generated from the same version of the TZDB as the
Makefiles in these two tests, then these validation tests should always pass.

<a name="TestJavaTime"></a>
### Java java.time

The Java 11 `java.time` library is not limited to 2038 but supports years
through the [year 1,000,000,000
(billion)](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/class-use/Instant.html).
I wrote the
[compare_java](tools/compare_java)
program to generate a `validation_data.cpp` file in exactly the same format as
the `tzcompiler.py` program, and produced data points from year 2000 to year
2050, which is the exact range of years supported by the `zonedb::` and
`zonedbx::` zoneinfo files.

The result is 2 validation programs:

* [JavaBasicTest](tests/JavaBasicTest/)
* [JavaExtendedTest](tests/JavaExtendedTest/)

The most difficult part of using Java is figuring out how to install it
and figuring out which of the many variants of the JDK to use. On Ubuntu 18.04,
I used `openjdk 11.0.4 2019-07-16` which seems to use TZ Database 2018g. I have
no recollection how I installed it, I think it was something like `$ sudo apt
install openjdk-11-jdk:amd64`.

The underlying timezone database used by the `java.time` package seems to be
locked to the release version of the JDK. I have not been able to figure out a
way to upgrade the timezone database independently (it's something to do with
the
[TZUpdater](https://www.oracle.com/technetwork/java/javase/documentation/tzupdater-readme-136440.html)
but I haven't figured it out.)

A number of zones did not match between java.time and AceTime. Those
have been listed in the `compare_java/blacklist.json` file.

<a name="TestHinnantDate"></a>
### C++ Hinnant Date

I looked for a timezone library that allowed me to control the specific
version of the TZ Database. This led me to the C++11/14/17 [Hinnant
date](https://github.com/HowardHinnant/date) library, which has apparently been
accepted into the C++20 standard. This date and timezone library is incredible
powerful, complex and difficult to use. I managed to incorporate it into 2 more
validation tests, and verified that the AceTime library matches the Hinnant date
library for all timezones from 2000 to 2049 (inclusive):

* [HinnantBasicTest](tests/HinnantBasicTest/)
* [HinnantExtendedTest](tests/HinnantExtendedTest/)

AceTime matches Hinnant Date on all data points from the year 2000 to 2050. No
`blacklist.json` file was needed.

<a name="TestNodaTime"></a>
### Noda Time

I wrote the test data generator [compare_noda](tools/compare_noda) in C# to
generate a `validation_data.json` file using the
[Noda Time](https://nodatime.org) library. The result is 2 validation programs:

* [NodaBasicTest](tests/NodaBasicTest/)
* [NodaExtendedTest](tests/NodaExtendedTest/)

AceTime matches Noda Time on all data points from the year 2000 to 2050. No
`blacklist.json` file was needed.

<a name="TestGoLangTime"></a>
### Go Lang Time

The [compare_go](tools/compare_go) tool generates a `validation_data.json`
file using the `time` package (https://pkg.go.dev/time) in the Golang standard
library. I believe the `time` package uses the underlying TZDB installed on the
host operating system. There are 2 validation programs: that consume this test

* [GoBasicTest](tests/GoBasicTest/)
* [GoExtendedTest](tests/GoExtendedTest/)

AceTime matches the Go lang `time` package on all data points from the year 2000
to 2050. No `blacklist.json` file was needed. The `time` package does not
calculate the DST offset, so no validation for that field was performed.

<a name="TestAceTimeC"></a>
### AceTimeC

I wrote the test data generator [compare_acetimec](tools/compare_acetimec) in
the C language to generate a `validation_data.json` file using the
[AceTimeC](https://github.com/bxparks/AceTimeC) library. The result is 2
validation programs:

* [AceTimeCBasicTest](tests/AceTimeCBasicTest/)
* [AceTimeCExtendedTest](tests/AceTimeCExtendedTest/)

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
