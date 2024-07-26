# Peer-to-Peer Validation

This directory allows different timezone libraries to be validated against each
other in pairs. This slightly different than the [../tests](../tests) directory
whose primary purpose is to validate the
[AceTime](https://github.com/bxparks/AceTime) Arduino library against various
third party timezone libraries.

Although the underlying code is able to validate in pairs, for simplicity,
the golden baseline dataset is computed using the `AceTime` library using the
`zonedbc` (complete) database.

## How to Run

- Update the `zonedb` versions of various dependent libraries:
    - AceTime
    - acetimec
    - acetimego
    - acetimepy
- Recompile the `tools` binaries to pick up the latest TZDB version.
    - `$ cd ../tools`
    - `$ make clean`
    - `$ make`
- Run the diffs
    - `$ cd ../validation`
    - `$ vi Makefile`
        - Update the `TZDB_VERSION` parameter in the `Makefile`
    - `$ make clean`
    - `$ make -j2 validation`

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
`baseline.json` is copied from `acetime_complete.json` which is provided by the
`AceTime` library.

The `Makefile` provides targets of the form `diff_xxx` which compare those
validation files against the `baseline.json` file. For example, the `diff_libc`
target compares the `libc.json` file produced by the C libc library against the
baseline.

### Timezone List

The list of timezones is provided by:

* `zones.txt` (full list, about 350 timezones)
* `zone.txt` (small subset for debugging)

### Helper Scripts

A number of helper scripts and binaries exist under the [tools](../tools)
directory to implement this function of this directory:

* `tools/compare_xxx/`
    * Each library `xxx` has a native binary (e.g. `compare_libc.out`), or a
      script (e.g. `compare_zoneutil.py`), or an intermediate object file (e.g.
      `compare_java.class`).
    * The job of those binaries or scripts is to read the list of timezones on
      the *stdin* (usually provided by `zones.txt`), and print out the
      validation data in JSON format on the *stdout* (which is redirected to the
      `xxx.json` file).
* `tools/flatten_validation/flatten.py`
    * Convert a `xxx.json` file into a more human-readable and easier to debug
      `xxx.txt` file.
* `tools/diff_validation/diff.py`
    * Compare the `--observed xxx.json` file with the `--expected yyy.json`
      file.
    * Exits with status 0 upon detecting no diff.
    * Exits with status 1 upon detecting a diff.

## Supported Libraries

The following libraries are supported:

**AceTime** family of libraries

* AceTime library
    * files: `acetime.json`, `acetime.txt`
    * target: `make diff_acetime`
    * script: `tools/compare_acetime`
* acetimec library (**baseline**)
    * files: `acetimec.json`, `acetimec.txt`
    * target: `make diff_acetimec`
    * script: `tools/compare_acetimec`
* acetimepy library
    * files: `acetz.json`, `acetz.txt`
    * target: `make diff_acetz`
    * script: `tools/compare_acetz`
* acetimego
    * files: `acetimego.json`, `acetimego.txt`
    * target: `make diff_acetimego`
    * script: `tools/compare_acetimego`

**Third Party**

* Hinnant date C++ library
    * files: `hinnant.json`, `hinnant.txt`
    * diff target: `make diff_hinnant`
    * script: `tools/compare_hinnant`
* libc C library
    * files: `libc.json`, `libc.txt`
    * target: `make diff_libc`
    * script: `tools/compare_libc`
* dateutil Python library
    * files: `dateutil.json`, `dateutil.txt`
    * target: `make diff_dateutil`
    * script: `tools/compare_dateutil`
* go.time Go library
    * files: `go.json`, `go.txt`
    * target: `make diff_go`
    * script: `tools/compare_gotime`
* java.time Java library
    * files: `java.json`, `java.txt`
    * target: `make diff_java`
    * script: `tools/compare_java`
* NodaTime C# library
    * files: `noda.json`, `noda.txt`
    * target: `make diff_noda`
    * script: `tools/compare_noda`
* pytz Python library
    * files: `pytz.json`, `pytz.txt`
    * target: `make diff_pytz`
    * script: `tools/compare_pytz`
* zoneinfo Python library
    * files: `zoneinfo.json`, `zoneinfo.txt`
    * target: `make diff_zoneinfo`
    * script: `tools/compare_zoneinfo`

## Diff Results

Here are the diff result for the various targets which validate against the
`baseline.json` (which is currently the `acetimec.json` file from acetimec):

```
+-----------------------+--------+--------------------------------+
| target                | status | comment                        |
+-----------------------+--------+--------------------------------+
| diff_acetime_basic    | OK     |                                |
| diff_acetime_complete | OK     | comparison with self           |
| diff_acetime_extended | OK     |                                |
| diff_acetimec         | OK     |                                |
| diff_acetimego        | OK     |                                |
| diff_acetimepy        | OK     |                                |
| diff_dateutil         | errors | supports only y <= 2038        |
| diff_go               | OK     |                                |
| diff_hinnant          | OK     |                                |
| diff_java             | errors | old TZDB, invalid DST          |
| diff_libc             | OK     |                                |
| diff_noda             | OK     |                                |
| diff_pytz             | errors | supports only y <= 2038        |
| diff_zoneinfo         | errors | incorrect DST offsets          |
+-------------------    +--------+--------------------------------+
```

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
