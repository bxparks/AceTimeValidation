#------------------------------------------------------------------------------
# Python, mypy, and flake8 checks.
#------------------------------------------------------------------------------

# Add the following if the NodaTime compiler breaks too often
#
# if [ $$i = 'compare_noda/Makefile' ]; then \
# #	continue ; \
# fi; \
#
all:
	set -e; \
	for i in */Makefile; do \
		$(MAKE) -C $$(dirname $$i); \
	done

# Python type checking, style checking, and unit tests.
python: mypy flake8 tests

# Mypy complains if I run 'mypy --strict' on compare_xxx directories. It cannot
# find definitions for acetimepy and AceTimeTools. If I explicitly add the
# source path for those then it complains about not finding `tdgenerator.py`
# and `zptdgenerator.py` and so on. The -m flag seems to resolve the error, but
# I am not convinced that mypy actually does anything with that flag. Python
# packaging is so confusing, it's impossible to figure out how to do anything.
mypy:
	python3 -m mypy --strict \
		-m compare_acetz \
		-m compare_pytz \
		-m compare_dateutil \
		-m compare_zoneinfo \
		-m generate_validation \
		-m flatten_validation \
		-m diff_validation

tests:
	python3 -m unittest

# W503 and W504 are both enabled by default and are mutual
# contradictory, so we have to suppress one of them.
# E501 uses 79 columns by default, but 80 is the default line wrap in
# vim, so change the line-length.
flake8:
	python3 -m flake8 . \
		--count \
		--ignore W503 \
		--show-source \
		--statistics \
		--max-line-length=80

clean:
	for i in */Makefile; do \
		$(MAKE) -C $$(dirname $$i) clean; \
	done
