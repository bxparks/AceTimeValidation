# AceTimeValidation Tools

Validation test data generators for various third party libraries written
in various languages:

* `compare_pytz` - use the `pytz` library in Python
* `compare_dateutil` - use the `python-dateutil` library in Python
* `compare_acetz` - use `AceTimePython` library in Python
* `compare_java` - use `java.time` library in Java
* `compare_cpp` - use Howard Hinnant `date` library in C++
* `compare_noda` - use the Noda Time library in C#

These produce a `validation_data.json` file, which is processed by the

* `generator_validation`

script to generate the following Arduino-compatible C++ files:

* `validation_data.h`
* `validation_data.cpp`
* `validation_tests.cpp`
