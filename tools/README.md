# AceTimeValidation Tools

The purpose of these scripts and binaries is to produce a `validation.json` file
that contains a list of DST transitions and selected samples for all timezones
given on the `stdin`. Usually, the list of timezones is contained in a
`zones.txt` file.

* `compare_acetime` - AceTime library in Arduino C++
* `compare_acetimec` - `AceTimeC` library in C
* `compare_acetz` - `acetimepy` library in Python
* `compare_dateutil` - `python-dateutil` library in Python
* `compare_gotime` - standard library `time` package in Go
* `compare_hinnant` - Howard Hinnant `date` library in C++
* `compare_java` - `java.time` library in Java
* `compare_libc` - the standard libc library in C
* `compare_noda` - the Noda Time library in C#
* `compare_pytz` - the `pytz` library in Python
* `compare_zoneinfo` - the `zoneinfo` library in Python 3.9

For the validation tests under by [tests](../tests), the
[generate_validation](generate_validation) script consumes the
`validation_data.json` file to generate the following Arduino-compatible C++
files:

## Dependencies and Prerequisites

* Ubuntu 20.04, 22.04 or MacOS 11.6.8 (Big Sur) or higher
* Python 3.7 or higher
* Python 3.9 or higher for the `zoneinfo` library
* Prerequisite tools
    * [AceTimeTools](https://github.com/bxparks/AceTimeTools) as a sibling
      project
        * `$ git clone https://github.com/bxparks/AceTimeTools`
    * [AUnit](https://github.com/bxparks/AUnit) as a sibling project
        * `$ git clone https://github.com/bxparks/AUnit`
    * [EpoxyDuino](https://github.com/bxparks/EpoxyDuino) as sibling project
        * `$ git clone https://github.com/bxparks/EpoxyDuino`
    * `libcurl4` library needed by Hinnant date:
        * `$ sudo apt install libcurl4-openssl-dev`
    * Microsoft .NET 6.0 on Linux:
        * See https://docs.microsoft.com/en-us/dotnet/core/install/linux
        * The `compare_noda.csproj` file already contains the NuGet dependency
        to Noda Time and will be automatically retrieved by the `dotnet build`
        command.
    * [IANA TZDB](https://github.com/eggert/tz) as a sibling project
        * `$ git clone https://github.com/eggert/tz`
* first party libraries
    * [AceTime](https://github.com/bxparks/AceTime) as a sibling project
        * `$ git clone https://github.com/bxparks/AceTime`
    * [AceTimeC](https://github.com/bxparks/AceTimeC) as a sibling project
        * `$ git clone https://github.com/bxparks/AceTimeC`
    * [acetimepy](https://github.com/bxparks/acetimepy) library
        * Not yet available on PyPI, so the installation process is manual.
        * `$ git clone https://github.com/bxparks/acetimepy`
        * `$ cd acetimepy`
        * `$ pip3 install --user -e .`
* third party libraries
    * [Hinnant date](https://github.com/HowardHinnant/date) as a sibling project
        * `$ git clone https://github.com/HowardHinnant/date`
    * [Python pytz](https://pypi.org/project/pytz/) library
        * `$ pip3 install --user pytz`
        * supports [dates only until 2038](https://answers.launchpad.net/pytz/+question/262216).
        * contains bugs for some zones, as listed in `compare_pytz/blacklist.json`
    * [Python dateutil](https://pypi.org/project/python-dateutil/) library
        * `$ pip3 install --user python-dateutil`
        * similar to `pytz` , `dateutil` supports
        [dates only until 2038](https://github.com/dateutil/dateutil/issues/462)
        * contains bugs for some zones, as listed in
        `compare_dateutil/blacklist.json`
    * [Java 11 Time](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/package-summary.html) library.
        * `$ sudo apt install openjdk-11-jdk`
        * supports years through the
        [year 1,000,000,000 (billion)](https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/time/class-use/Instant.html)
        * bugs with this library are listed in `compare_java/blacklist.json`
    * [Noda Time](https://nodatime.org) C# library
        * Should be automatically downloaded by `nuget`.
    * Install the latest Go language compiler.
        * See https://go.dev/doc/install

## Compiling

```
$ make clean
$ make
```
