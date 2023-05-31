# Compare with acetimego Library

Generate the DST transitions and monthly samples using the
[acetimego](https://github.com/bxparks/acetimego) library as a JSON object on
the standard output. This will often be redirected into a file named
`validation_data.json`.

This allows comparison with the [AceTime](https://github.com/bxparks/AceTime)
library through the following AUnit tests under `tests/`:

* `AcetimegoBasicTest/`
* `AcetimegoExtendedTest/`

and the following `make` target under `validation/`:

* `acetimego.json`
* `acetimego.txt`
* `diff_acetimego`
