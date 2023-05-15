# Compare with Go Time Library

Generate the DST transitions and monthly samples using the Go standard library
`time` package in the form of a `validation_data.json` file on the standard
output. This allows comparison with the
[AceTime](https://github.com/bxparks/AceTime) library through the following
AUnit tests:

* `GoTimeBasicTest/`
* `GoTimeExtendedTest/`
