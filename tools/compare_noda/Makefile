TZDB_VERSION := 2023c
START_YEAR := 2000
UNTIL_YEAR := 2100
ACE_TIME_EPOCH_YEAR := 2050

# Location of the https://github.com/nodatime/nodatime repo, assuming to be
# a sibling to this project.
NODA_TIME_DIR := $(abspath ../../../nodatime)
NODA_COMPILER_DIR := $(NODA_TIME_DIR)/src/NodaTime.TzdbCompiler

build: Program.cs
	dotnet build --framework net6.0

validation_data.json: Program.cs small.txt tzdata$(TZDB_VERSION).nzd
	dotnet run \
		--framework net6.0 \
		-- \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		--nzd_file tzdata$(TZDB_VERSION).nzd \
		< small.txt \
		> validation_data.json

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

clean:
	rm -rf validation_data.json tzdata$(TZDB_VERSION).nzd bin obj
