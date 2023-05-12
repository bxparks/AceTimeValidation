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

# Takes about 15 secs to processor 350 zones on a dual-core i5-6300U computer.
acetime.json: zones.txt
	$(VALIDATION_TOOLS)/compare_acetime/compare_acetime.out \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< zones.txt \
		> $@

acetime.txt: acetime.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

#------------------------------------------------------------------------------

# Takes about 9 secs to process 350 zones on a dual-core i5-6300U computer.
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

# Takes about 70 secs to processor 350 zones on a dual-core i5-6300U computer.
hinnant.json: zones.txt
	$(VALIDATION_TOOLS)/compare_hinnant/compare_hinnant.out \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< zones.txt \
		> $@

hinnant.txt: hinnant.json
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

# These Makefile targets copied from ../tests/NodaBasicTest and
# ../tests/NodaExtendedTest.

# Location of the https://github.com/nodatime/nodatime repo, assuming to be
# a sibling to this project.
NODA_TIME_DIR := $(abspath ../../nodatime)
NODA_COMPILER_DIR := $(NODA_TIME_DIR)/src/NodaTime.TzdbCompiler

# Takes about ?? secs to process 350 zones on a dual-core i5-6300U computer.
noda.json: \
zones.txt \
$(VALIDATION_TOOLS)/compare_noda/Program.cs \
tzdata$(TZ_VERSION).nzd
	dotnet run \
		--framework net6.0 \
		--project $(VALIDATION_TOOLS)/compare_noda/compare_noda.csproj \
		-- \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		--nzd_file tzdata$(TZ_VERSION).nzd \
		< zones.txt \
		> $@

noda.txt: noda.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

# Generate the custom tzdata{xxx}.nzd file, depending on TZ_VERSION.
tzdata$(TZ_VERSION).nzd: $(NODA_COMPILER_DIR)/bin/Debug/net6.0/NodaTime.TzdbCompiler
	$(NODA_COMPILER_DIR)/bin/Debug/net6.0/NodaTime.TzdbCompiler \
		-s https://data.iana.org/time-zones/releases/tzdata$(TZ_VERSION).tar.gz \
		-w $(NODA_TIME_DIR)/data/cldr/windowsZones-42.xml \
		-o $@

# Build the TzdbCompiler inside its own directory and explicitly invoke the
# binary. If I use 'dotnet run --project xxx', it complains about not finding a
# bunch of files.
$(NODA_COMPILER_DIR)/bin/Debug/net6.0/NodaTime.TzdbCompiler:
	cd $(NODA_COMPILER_DIR); \
	dotnet build

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

diff: expected.txt observed.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed observed.json \
		--expected expected.json

diff_acetime: acetime.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed acetime.json \
		--expected baseline.json

diff_acetimec: acetimec.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed acetimec.json \
		--expected baseline.json

diff_acetz: acetz.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed acetz.json \
		--expected baseline.json

diff_dateutil: dateutil.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed dateutil.json \
		--expected baseline.json

diff_go: go.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed go.json \
		--expected baseline.json

diff_hinnant: hinnant.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed hinnant.json \
		--expected baseline.json

diff_libc: libc.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed libc.json \
		--expected baseline.json

diff_noda: noda.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed noda.json \
		--expected baseline.json

diff_zoneinfo: zoneinfo.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed zoneinfo.json \
		--expected baseline.json
