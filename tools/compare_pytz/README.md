# Compare with Python Pytz

Compare the [AceTime](https://github.com/bxparks/AceTime) library with  the
Python `pytz` package.
Generates the `validation_data.json` file needed by:

* `BasicPytzTest/`
* `ExtendedPytzTest/`

## Blacklist

With `detect_dst_transition = True`, 6 zones listed in `blacklist.json` show
incorrect DST offsets compared to AceTime and Hinannt libraries.
