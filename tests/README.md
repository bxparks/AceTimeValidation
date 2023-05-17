# Validation Tests

This directory contains a number of [AUnit](https://github.com/bxparks/AUnit)
tests, each of which is composed of:

* static files checked into git
    * `XxxTest.ino`
* generate files, not checked into git, but programmatically generated
    * `validation_data.json`
    * `validation_data.h`
    * `validation_data.cpp`
    * `validation_tests.cpp`

The `validation_data.json` file is produced by one of the `compare_xxx` scripts
or binaries in the [tools](../tools) directory. It contains the list of DST
transitions for each timezone as determined by the library invoked by the
`compare_xxx` script.

The resulting `*.h` and `*.cpp` file form a self-contained unit test against the
AceTime library. The unit test can be compiled and executed in a Linux
environment using the [EpoxyDuino](https://github.com/bxparks/EpoxyDuino)
adapter layer.

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
