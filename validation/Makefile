TZDB_VERSION := 2023c
TZ_REPO := ../../tz
ACETIME_TOOLS := ../../AceTimeTools
VALIDATION_TOOLS := $(abspath ../tools)

EPOCH_YEAR := 1970
BASIC_START_YEAR := 2000
BASIC_UNTIL_YEAR := 2100
EXTENDED_START_YEAR := 2000
EXTENDED_UNTIL_YEAR := 2200
COMPLETE_START_YEAR := 1800
COMPLETE_UNTIL_YEAR := 2200

#------------------------------------------------------------------------------

# Extract the 'Zone' entries from the TZDB files.
zones.txt: tzdb
	echo '# Zones from TZDB $(TZDB_VERSION)' > $@
	grep --no-filename '^Zone' tzdb/* | awk '{print $$2}' | sort | uniq >> $@

# Copy the TZDB data files into the ./tzdb/ directory.
tzdb: $(ACETIME_TOOLS)/copytz.sh $(TZ_REPO)
	$(ACETIME_TOOLS)/copytz.sh --tag $(TZDB_VERSION) $(TZ_REPO) $@
	echo $(TZDB_VERSION) > $@/version.txt

#------------------------------------------------------------------------------

# Run the validation targets which are expected to pass after a new TZDB is
# released. This can be invoked with 'make -j4' to run the dependent targets in
# parallel because they are independent.
validation: \
diff_acetime_basic \
diff_acetime_extended \
diff_acetime_complete \
diff_acetimec \
diff_acetz \
diff_hinnant

# Additional targets which need to be recompiled, or need to wait for an OS
# update before they become comformant with the latest TZDB_VERSION.
allvalidation: \
diff_libc \
diff_noda \
diff_acetimego

#------------------------------------------------------------------------------

observed.json: zone.txt Makefile
	$(VALIDATION_TOOLS)/compare_acetz/compare_acetz.py \
		--start_year $(BASIC_START_YEAR) \
		--until_year $(BASIC_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		--use_internal_transition \
		< zone.txt \
		> $@

observed.txt: observed.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

expected.json: zone.txt Makefile
	$(VALIDATION_TOOLS)/compare_acetimec/compare_acetimec.out \
		--start_year $(BASIC_START_YEAR) \
		--until_year $(BASIC_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		< zone.txt \
		> $@

expected.txt: expected.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

diff: expected.txt observed.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed observed.json \
		--expected expected.json

#------------------------------------------------------------------------------
# Baseline (acetime_complete)
#------------------------------------------------------------------------------

baseline.json: acetime_complete.json
	cp $< $@

# The extra dependency to 'baseline.json' is needed to guarantee that
# baseline.json to exist if baseline.txt exists.
baseline.txt: acetime_complete.txt baseline.json
	cp acetime_complete.txt $@

#------------------------------------------------------------------------------
# AceTime library
#------------------------------------------------------------------------------

# Takes about 15 secs to processor 350 zones on a dual-core i5-6300U computer.
acetime_basic.json: zones.txt
	$(VALIDATION_TOOLS)/compare_acetime/compare_acetime.out \
		--start_year $(BASIC_START_YEAR) \
		--until_year $(BASIC_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		--scope basic \
		< zones.txt \
		> $@

acetime_basic.txt: acetime_basic.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

# Takes about 15 secs to processor 350 zones on a dual-core i5-6300U computer.
acetime_extended.json: zones.txt
	$(VALIDATION_TOOLS)/compare_acetime/compare_acetime.out \
		--start_year $(BASIC_START_YEAR) \
		--until_year $(BASIC_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		--scope extended \
		< zones.txt \
		> $@

acetime_extended.txt: acetime_extended.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

# Takes about 15 secs to processor 350 zones on a dual-core i5-6300U computer.
acetime_complete.json: zones.txt
	$(VALIDATION_TOOLS)/compare_acetime/compare_acetime.out \
		--start_year $(COMPLETE_START_YEAR) \
		--until_year $(COMPLETE_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		--scope complete \
		< zones.txt \
		> $@

acetime_complete.txt: acetime_complete.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@


diff_acetime_basic: acetime_basic.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed acetime_basic.json \
		--expected baseline.json

diff_acetime_extended: acetime_extended.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed acetime_extended.json \
		--expected baseline.json

diff_acetime_complete: acetime_complete.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed acetime_complete.json \
		--expected baseline.json

#------------------------------------------------------------------------------
# acetimec library
#------------------------------------------------------------------------------

# Takes about 9 secs to process 350 zones on a dual-core i5-6300U computer.
acetimec.json: zones.txt
	$(VALIDATION_TOOLS)/compare_acetimec/compare_acetimec.out \
		--start_year $(COMPLETE_START_YEAR) \
		--until_year $(COMPLETE_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		--zonedb zonedball \
		< zones.txt \
		> $@

acetimec.txt: acetimec.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

diff_acetimec: acetimec.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed acetimec.json \
		--expected baseline.json

#------------------------------------------------------------------------------
# acetimego library
#------------------------------------------------------------------------------

# Takes about 22 secs to process 350 zones on a dual-core i5-6300U computer.
acetimego.json: zones.txt
	$(VALIDATION_TOOLS)/compare_acetimego/compare_acetimego.out \
		--start_year $(COMPLETE_START_YEAR) \
		--until_year $(COMPLETE_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		< zones.txt \
		> $@

acetimego.txt: acetimego.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

diff_acetimego: acetimego_complete.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed acetimego.json \
		--expected baseline.json

#------------------------------------------------------------------------------
# acetimepy library
#------------------------------------------------------------------------------

# Takes about 40 secs to process 350 zones on a dual-core i5-6300U computer.
# The result must be identical to acetzb.json.
acetz.json: zones.txt
	$(VALIDATION_TOOLS)/compare_acetz/compare_acetz.py \
		--start_year $(COMPLETE_START_YEAR) \
		--until_year $(COMPLETE_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		--use_internal_transition \
		< zones.txt \
		> $@

acetz.txt: acetz.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

diff_acetz: acetz.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed acetz.json \
		--expected baseline.json

#------------------------------------------------------------------------------
# dateutil library
#------------------------------------------------------------------------------

# Takes about 7 mins to process 350 zones on a dual-core i5-6300U computer.
dateutil.json: zones.txt
	$(VALIDATION_TOOLS)/compare_dateutil/compare_dateutil.py \
		--start_year $(BASIC_START_YEAR) \
		--until_year $(BASIC_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		< zones.txt \
		> $@

dateutil.txt: dateutil.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

diff_dateutil: dateutil.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed dateutil.json \
		--expected baseline.json

#------------------------------------------------------------------------------
# Go time library
#------------------------------------------------------------------------------

# Takes about 8 secs to process 350 zones.
gotime.json: zones.txt
	$(VALIDATION_TOOLS)/compare_gotime/compare_gotime.out \
		--start_year $(COMPLETE_START_YEAR) \
		--until_year $(COMPLETE_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		< zones.txt \
		> $@

gotime.txt: gotime.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

diff_gotime: gotime.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed gotime.json \
		--expected baseline.json

#------------------------------------------------------------------------------
# Hinnant date library
#------------------------------------------------------------------------------

# Takes about 70 secs to processor 350 zones on a dual-core i5-6300U computer.
hinnant.json: zones.txt
	$(VALIDATION_TOOLS)/compare_hinnant/compare_hinnant.out \
		--start_year $(COMPLETE_START_YEAR) \
		--until_year $(COMPLETE_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		< zones.txt \
		> $@

hinnant.txt: hinnant.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

diff_hinnant: hinnant.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed hinnant.json \
		--expected baseline.json

#------------------------------------------------------------------------------
# Java time library
#------------------------------------------------------------------------------

# Takes about 10 secs to processor 350 zones on a dual-core i5-6300U computer.
java.json: zones.txt
	java -cp $(VALIDATION_TOOLS)/compare_java CompareJava \
		--start_year $(BASIC_START_YEAR) \
		--until_year $(BASIC_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		< zones.txt \
		> $@

java.txt: java.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

diff_java: java.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed java.json \
		--expected baseline.json

#------------------------------------------------------------------------------
# libc library
#------------------------------------------------------------------------------

# Takes about 13 secs to process 350 zones on a dual-core i5-6300U computer.
libc.json: zones.txt
	$(VALIDATION_TOOLS)/compare_libc/compare_libc.out \
		--start_year $(COMPLETE_START_YEAR) \
		--until_year $(COMPLETE_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		< zones.txt \
		> $@

libc.txt: libc.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

diff_libc: libc.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed libc.json \
		--expected baseline.json

#------------------------------------------------------------------------------
# NodaTime library
#------------------------------------------------------------------------------

# These Makefile targets copied from ../tests/NodaBasicTest and
# ../tests/NodaExtendedTest.

# Location of the https://github.com/nodatime/nodatime repo, assuming to be
# a sibling to this project.
NODA_TIME_DIR := $(abspath ../../nodatime)
NODA_COMPILER_DIR := $(NODA_TIME_DIR)/src/NodaTime.TzdbCompiler

# Takes about ~120 (!) secs to process 350 zones on a dual-core i5-6300U
# computer. The main bottleneck is the Console.WriteLine() function which seems
# to be incredibly slow on Csharp.
noda.json: \
zones.txt \
$(VALIDATION_TOOLS)/compare_noda/Program.cs \
tzdata$(TZDB_VERSION).nzd
	dotnet run \
		--framework net6.0 \
		--project $(VALIDATION_TOOLS)/compare_noda/compare_noda.csproj \
		-- \
		--start_year $(COMPLETE_START_YEAR) \
		--until_year $(COMPLETE_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		--nzd_file tzdata$(TZDB_VERSION).nzd \
		< zones.txt \
		> $@

noda.txt: noda.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

# Generate the custom tzdata{xxx}.nzd file, depending on TZDB_VERSION.
tzdata$(TZDB_VERSION).nzd: $(NODA_COMPILER_DIR)/bin/Debug/net6.0/NodaTime.TzdbCompiler
	$(NODA_COMPILER_DIR)/bin/Debug/net6.0/NodaTime.TzdbCompiler \
		-s https://data.iana.org/time-zones/releases/tzdata$(TZDB_VERSION).tar.gz \
		-w $(NODA_TIME_DIR)/data/cldr/windowsZones-42.xml \
		-o $@

# Build the TzdbCompiler inside its own directory and explicitly invoke the
# binary. If I use 'dotnet run --project xxx', it complains about not finding a
# bunch of files.
$(NODA_COMPILER_DIR)/bin/Debug/net6.0/NodaTime.TzdbCompiler:
	cd $(NODA_COMPILER_DIR); \
	dotnet build

diff_noda: noda.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed noda.json \
		--expected baseline.json

#------------------------------------------------------------------------------
# zoneinfo library
#------------------------------------------------------------------------------

# Takes about 40 secs to process 350 zones on a dual-core i5-6300U computer.
zoneinfo.json: zones.txt
	$(VALIDATION_TOOLS)/compare_zoneinfo/compare_zoneinfo.py \
		--start_year $(BASIC_START_YEAR) \
		--until_year $(BASIC_UNTIL_YEAR) \
		--epoch_year $(EPOCH_YEAR) \
		< zones.txt \
		> $@

zoneinfo.txt: zoneinfo.json
	$(VALIDATION_TOOLS)/flatten_validation/flatten.py \
		< $< > $@

diff_zoneinfo: zoneinfo.txt baseline.txt
	$(VALIDATION_TOOLS)/diff_validation/diff.py \
		--observed zoneinfo.json \
		--expected baseline.json

#------------------------------------------------------------------------------

clean:
	rm -rf tzdb
	rm -f tz*.nzd *.json *.txt
