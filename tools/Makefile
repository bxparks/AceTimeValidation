#------------------------------------------------------------------------------
# Python, mypy, and flake8 checks.
#------------------------------------------------------------------------------

all: mypy flake8 tests

# Mypy complains if I run 'mypy --strict' on compare_xxx directories. It cannot
# find definitions for AceTimePython and AceTimeTools. If I explicitly add the
# source path for those then it complains about not finding `tdgenerator.py`
# and `zptdgenerator.py` and so on. The -m flag seems to resolve the error, but
# I am not convinced that mypy actually does anything with that flag. Python
# packaging is so confusing, it's impossible to figure out how to do anything.
mypy:
	mypy --strict \
		-m compare_acetz \
		-m compare_pytz \
		-m compare_dateutil \
		-m compare_zoneinfo

tests:
	python3 -m unittest

# W503 and W504 are both enabled by default and are mutual
# contradictory, so we have to suppress one of them.
# E501 uses 79 columns by default, but 80 is the default line wrap in
# vim, so change the line-length.
flake8:
	flake8 . \
		--count \
		--ignore W503 \
		--show-source \
		--statistics \
		--max-line-length=80


