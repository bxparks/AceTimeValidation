# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

DB_NAMESPACE := zonedbnoda
GENERATED := validation_data.cpp \
	validation_data.h \
	validation_tests.cpp \
	validation_data.json \
	$(DB_NAMESPACE)/zone_infos.h \
	$(DB_NAMESPACE)/zone_infos.cpp \
	$(DB_NAMESPACE)/zone_policies.h \
	$(DB_NAMESPACE)/zone_policies.cpp \
	$(DB_NAMESPACE)/zone_registry.h \
	$(DB_NAMESPACE)/zone_registry.cpp \
	$(DB_NAMESPACE)/zones.txt \
	tzdata$(TZ_VERSION).nzd
OBJS := validation_data.o \
	validation_tests.o \
	$(DB_NAMESPACE)/zone_infos.o \
	$(DB_NAMESPACE)/zone_policies.o \
	$(DB_NAMESPACE)/zone_registry.o
APP_NAME := BasicNodaTest
ARDUINO_LIBS := AUnit AceCommon AceTime AceSorting
MORE_CLEAN := more_clean
include ../../EpoxyDuino/EpoxyDuino.mk

# Define the scope of the test data.
SCOPE := basic
TZ_VERSION := 2022g
ACE_TIME_EPOCH_YEAR := 2050
DB_START_YEAR := 2000
DB_UNTIL_YEAR := 10000
VAL_START_YEAR := 2000
VAL_UNTIL_YEAR := 2100

# Location of TZDB and various tools.
TZ_REPO := $(abspath ../../tz)
VALIDATION_TOOLS := $(abspath ../tools)
ACE_TIME_TOOLS := $(abspath ../../AceTimeTools)

# Location of the https://github.com/nodatime/nodatime repo. Assumed to be
# sibling to the AceTimeValiation project.
NODA_TIME_DIR := $(abspath ../../nodatime)
NODA_COMPILER_DIR := $(NODA_TIME_DIR)/src/NodaTime.TzdbCompiler

runtests:
	./$(APP_NAME).out

validation_data.cpp: validation_data.h

validation_tests.cpp: validation_data.h

validation_data.h: validation_data.json
	$(VALIDATION_TOOLS)/generate_validation/generate_validation.py \
		--tz_version $(TZ_VERSION) \
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		< validation_data.json

validation_data.json: $(DB_NAMESPACE)/zones.txt \
		$(VALIDATION_TOOLS)/compare_noda/Program.cs \
		tzdata$(TZ_VERSION).nzd
	dotnet run \
		--project $(VALIDATION_TOOLS)/compare_noda/compare_noda.csproj \
		-- \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		--nzd_file tzdata$(TZ_VERSION).nzd \
		< $(DB_NAMESPACE)/zones.txt \
		> $@

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

$(DB_NAMESPACE)/%.o: $(DB_NAMESPACE)/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(DB_NAMESPACE)/zone_infos.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_policies.h: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_policies.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_registry.h: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_registry.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zones.txt: $(DB_NAMESPACE)/zone_infos.h

# Generate the zones.txt and zonedb files under the $(DB_NAMESPACE) directory.
$(DB_NAMESPACE)/zone_infos.h:
	mkdir -p $(DB_NAMESPACE); \
	cd $(DB_NAMESPACE); \
	$(ACE_TIME_TOOLS)/tzcompiler.sh \
		--tzrepo $(TZ_REPO) \
		--tag $(TZ_VERSION) \
		--language arduino,zonelist \
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		--generate_int16_year \
		--start_year $(DB_START_YEAR) \
		--until_year $(DB_UNTIL_YEAR)

more_clean:
	rm -rf $(DB_NAMESPACE) tzdata$(TZ_VERSION).nzd
