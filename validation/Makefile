# Define the scope of the test data.
SCOPE := basic
TZ_VERSION := 2023c
ACE_TIME_EPOCH_YEAR := 2050
DB_START_YEAR := 2000
DB_UNTIL_YEAR := 10000
VAL_START_YEAR := 2000
VAL_UNTIL_YEAR := 2100

# Location of TZDB and various tools.
VALIDATION_TOOLS := $(abspath ../tools)

#------------------------------------------------------------------------------

observed.json: zone.txt Makefile
	$(VALIDATION_TOOLS)/compare_acetz/compare_acetz.py \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< zone.txt \
		> $@

observed.txt: observed.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

expected.json: zone.txt Makefile
	$(VALIDATION_TOOLS)/compare_acetimec/compare_acetimec.out \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< zone.txt \
		> $@

expected.txt: expected.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

#------------------------------------------------------------------------------

# Use acetimec as the baseline. Previouslky used Hinnant date until I
# discovered bugs with its detection of date-time in overlap or gap.
baseline.json: acetimec.json
	cp -f $< $@

baseline.txt: acetimec.txt
	cp -f $< $@

#------------------------------------------------------------------------------

# Takes about 75 secs to processor 350 zones on a dual-core i5-6300U computer.
hinnant.json: zones.txt
	$(VALIDATION_TOOLS)/compare_cpp/compare_cpp.out \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< zones.txt \
		> $@

hinnant.txt: hinnant.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

#------------------------------------------------------------------------------

# Takes about 7 mins to process 350 zones on a dual-core i5-6300U computer.
acetz.json: zones.txt
	$(VALIDATION_TOOLS)/compare_acetz/compare_acetz.py \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< zones.txt \
		> $@

acetz.txt: acetz.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

#------------------------------------------------------------------------------

# Takes about 7 mins to process 350 zones on a dual-core i5-6300U computer.
dateutil.json: zones.txt
	$(VALIDATION_TOOLS)/compare_dateutil/compare_dateutil.py \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< zones.txt \
		> $@

dateutil.txt: dateutil.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

#------------------------------------------------------------------------------

# Takes about 40 secs to process 350 zones on a dual-core i5-6300U computer.
zoneinfo.json: zones.txt
	$(VALIDATION_TOOLS)/compare_zoneinfo/compare_zoneinfo.py \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< zones.txt \
		> $@

zoneinfo.txt: zoneinfo.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

#------------------------------------------------------------------------------

# Takes about 55 secs to process 350 zones on a dual-core i5-6300U computer.
acetimec.json: zones.txt
	$(VALIDATION_TOOLS)/compare_acetimec/compare_acetimec.out \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< zones.txt \
		> $@

acetimec.txt: acetimec.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

#------------------------------------------------------------------------------

# Takes about 13 secs to process 350 zones on a dual-core i5-6300U computer.
libc.json: zones.txt
	$(VALIDATION_TOOLS)/compare_libc/compare_libc.out \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< zones.txt \
		> $@

libc.txt: libc.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

#------------------------------------------------------------------------------

# Takes about 8 secs to process 350 zones.
go.json: zones.txt
	$(VALIDATION_TOOLS)/compare_go/compare_go.out \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< zones.txt \
		> $@

go.txt: go.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

#------------------------------------------------------------------------------

diff: expected.txt observed.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed observed.json \
		--expected expected.json

diff_hinnant: hinnant.json baseline.json
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed hinnant.json \
		--expected baseline.json

diff_acetz: acetz.json baseline.json
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed acetz.json \
		--expected baseline.json

diff_acetimec: acetimec.json baseline.json
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed acetimec.json \
		--expected baseline.json

diff_zoneinfo: zoneinfo.json baseline.json
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed zoneinfo.json \
		--expected baseline.json

diff_libc: libc.json baseline.json
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed libc.json \
		--expected baseline.json

diff_go: go.json baseline.json
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed go.json \
		--expected baseline.json
