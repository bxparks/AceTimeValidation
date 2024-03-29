# Compare with acetimec

Compare the [AceTime](https://github.com/bxparks/AceTime) library with the 
[acetimec](https://github.com/bxparks/acetimec) library.
Generates the `validation_data.json` file needed by:

* `AcetimecBasicTest/`
* `AcetimecExtendedTest/`

The program reads the `zones.txt` file (generated by the `tzcompiler.py`
program) which contains the list of zones supported by zoneinfo files in
`src/ace_time/zonedb/` and `src/ace_time/zonedbx`. The program then generates a
test data set of all DST transitions, the epoch seconds, and the expected
date/time components using the [acetimec](https://github.com/bxparks/acetimec)
library.

## Compiling

I have tested this Ubuntu 20.04. Install the following:

* [AceTimeValidation](https://github.com/bxparks/AceTimeValidation)
* [acetimec](https://github.com/bxparks/acetimec) as a sibling to
  AceTimeValidation

Then use GNU Make to generate `compare_acetimec.out`:

```
$ cd src/acetimec
$ make
$ cd src/AceTimeValidation/tools/compare_acetimec
$ make
$ ./compare_acetimec < zones.txt > validation_data.json
```
