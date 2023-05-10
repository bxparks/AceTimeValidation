# Peer-to-Peer Validation

This directory allows different timezone libraries to be validated against each
other. This slightly different than the [../tests](../tests) directory which
converts the `validation_data.json` files from each library into an
[AUnit](https://github.com/bxparks/AUnit) unit test that is executed under the
[EpoxyDuino](https://github.com/bxparks/EpoxyDuino) environment on a Unix
machine.

## Files

### Makefile

The workflows are captured in the [Makefile](Makefile) that generates the
`xxx.json` files using various timezone libraries, where `xxx` is the
human-readable name for the timezone library. It also contains rules to generate
the `xxx.txt` file from the corresponding `xxx.json` file. The `.txt` file is
3-4X more compact than the JSON file to allow for easier debugging.

In theory, any `xxx.json` file can be compared to any other `yyy.json` file
using the [diff.py](../tools/diff_validation/diff.py) tool. But for simplicity,
it is convenient to compare each library against a baseline. Currently, the
`baseline.json` is copied from `acetimec.json` which is provided by the AceTimeC
library.

The `Makefile` provides targets of the form `diff_xxx` which compare those
validation files against the `baseline.json` file. For example, the `diff_ctime`
target compares the `ctime.json` file produced by the C libc library against the
baseline.

### Timezone List

The list of timezones is provided by:

* `zones.txt` (full list, about 350 timezones)
* `zone.txt` (small subset for debugging)

### Helper Scripts

A number of helper scripts and binaries exist under the [tools](../tools)
directory to implement this function of this directory:

* `tools/compare_xxx/`
    * Each library `xxx` has a native binary (e.g. `compare_ctime.out`), or a
      script (e.g. `compare_zoneutil.py`), or an intermediate object file (e.g.
      `compare_java.class`).
    * The job of those binaries or scripts is to read the list of timezones on
      the *stdin* (usually provided by `zones.txt`), and print out the
      validation data in JSON format on the *stdout* (which is redirected to the
      `xxx.json` file).
* `tools/flatten_validation/flatten.py`
    * Convert a `xxx.json` file into a `xxx.txt` file.
* `tools/diff_validation/diff.py`
    * Compare the `--observed xxx.json` file with the `--expected yyy.json`
      file.
    * Exits with status 0 upon detecting no diff.
    * Exits with status 1 upon detecting a diff.

## Supported Libraries

The following libraries are supported:

**AceTime** family of libraries

* AceTimeC library (**baseline**)
    * files: `acetimec.json`, `acetimec.txt`
    * target: `make diff_acetimec`
    * script: `tools/compare_acetimec`
* AceTimePython library
    * files: `acetz.json`, `acetz.txt`
    * target: `make diff_acetz`
    * script: `tools/compare_acetz`
* AceTime (TBD)
    * files: `acetime.json`, `acetime.txt`
    * target: `make diff_acetime`
    * script: `tools/compare_acetime`
* AceTimeGo (TBD)
    * files: `gotz.json`, `gotz.txt`
    * target: `make diff_gotz`
    * script: `tools/compare_gotz`

**Third Party**

* Hinnant date library
    * files: `hinnant.json`, `hinnant.txt`
    * diff target: `make diff_hinnant`
    * script: `tools/compare_cpp`
* libc C library
    * files: `ctime.json`, `ctime.txt`
    * target: `make diff_ctime`
    * script: `tools/compare_c`
* Python dateutil C library
    * files: `dateutil.json`, `dateutil.txt`
    * target: `make diff_dateutil`
    * script: `tools/compare_dateutil`
* Python zoneinfo C library
    * files: `zoneinfo.json`, `zoneinfo.txt`
    * target: `make diff_zoneinfo`
    * script: `tools/compare_zoneinfo`
* Go time library
    * files: `go.json`, `go.txt`
    * target: `make diff_go`
    * script: `tools/compare_go`
* Java java.time library (TBD)
    * files: `java.json`, `java.txt`
    * target: `make diff_java`
    * script: `tools/compare_java`
* C# NodaTime library (TBD)
    * files: `noda.json`, `noda.txt`
    * target: `make diff_noda`
    * script: `tools/compare_noda`

## Diff Status

Here are the diff result for the following targets which validate against the
`baseline.json` (which is currently the `acetimec.json` file from AceTimeC):

```
+-------------------+---------------+
| target            | status        |
+-------------------+---------------+
| diff_acetimec     | OK            |
| diff_acetz        | OK            |
| diff_ctime        | OK            |
| diff_go           | OK            |
| diff_hinnant      | errors        |
| diff_zoneinfo     | errors        |
+-------------------+---------------+
```

(Note: This may become a matrix in the future)

## Debugging

The special `make diff` target compares the `observed.json` file with the
`expected.json` file, using the `zone.txt` file for the list of timezones which
is intended to be small to help debugging.

The `Makefile` targets for `observed.json` and `expected.json` are intended to
be modified to point to the pair of libraries being compared. Then the following
commands will compare the two libraries and print the results:

```
$ vi Makefile
$ rm -f observed.json expected.json
$ make observed.json expected.json
$ make diff
```
