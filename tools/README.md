# AceTimeValidation Tools

Validation data generators for various third party libraries written
in various languages:

* `compare_acetimec` - use `AceTimeC` library in C
* `compare_acetz` - use `AceTimePython` library in Python
* `compare_cpp` - use Howard Hinnant `date` library in C++
* `compare_dateutil` - use the `python-dateutil` library in Python
* `compare_go` - use the `time` library in Go
* `compare_java` - use `java.time` library in Java
* `compare_noda` - use the Noda Time library in C#
* `compare_pytz` - use the `pytz` library in Python
* `compare_zoneinfo` - use the `zoneinfo` library in Python

These produce a `validation_data.json` file, which is processed by the
[generate_validation](generate_validation) script to generate the following
Arduino-compatible C++ files:

* `validation_data.h`
* `validation_data.cpp`
* `validation_tests.cpp`

These are then compiled as an [AUnit](https://github.com/bxparks/AUnit) unit
test program using the [EpoxyDuino](https://github.com/bxparks/EpoxyDuino)
adapter layer, into a binary that can executed on a Linux machine. The binary
performs that validation test.