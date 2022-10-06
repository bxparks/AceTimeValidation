# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

DB_NAMESPACE := zonedbdateutil
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
	$(DB_NAMESPACE)/zones.txt
OBJS := validation_data.o \
	validation_tests.o \
	$(DB_NAMESPACE)/zone_infos.o \
	$(DB_NAMESPACE)/zone_policies.o \
	$(DB_NAMESPACE)/zone_registry.o
APP_NAME := BasicDateUtilTest
ARDUINO_LIBS := AUnit AceCommon AceTime AceSorting
MORE_CLEAN := more_clean
include ../../EpoxyDuino/EpoxyDuino.mk

#.PHONY: $(GENERATED)

# Define the scope of the test data.
#
# dateutil (2.8.1) intially ships with 2019c, but when the 'tzdata' on
# Ubuntu 20.04 is updated to 2020a, it uses the new version instead of the
# internal version. I wish there was a way to peg the tzdata version
# used by dateutil to make unit testing more predictable.
#
# dateutil works until 2037, then fails the DST offsets in 2037 for the
# following zones:
#	Australia_Adelaide failed.
#	Australia_Broken_Hill failed.
#	Australia_Currie failed.
#	Australia_Hobart failed.
#	Australia_Melbourne failed.
#	Australia_Sydney failed.
#	Europe_Dublin failed.
#	Pacific_Auckland failed.
SCOPE := basic
TZ_VERSION := 2022d
START_YEAR := 2000
UNTIL_YEAR := 2037

# Location of various tools.
VALIDATION_TOOLS := $(abspath ../tools)
ACE_TIME_TOOLS := $(abspath ../../AceTimeTools)

runtests:
	./$(APP_NAME).out

validation_data.cpp: validation_data.h

validation_tests.cpp: validation_data.h

validation_data.h: validation_data.json
	$(VALIDATION_TOOLS)/generate_validation/generate_validation.py \
		--tz_version $(TZ_VERSION) \
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		--blacklist $(VALIDATION_TOOLS)/compare_dateutil/blacklist.json \
		< validation_data.json

validation_data.json: $(DB_NAMESPACE)/zones.txt
	$(VALIDATION_TOOLS)/compare_dateutil/generate_data.py \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR) \
		< $(DB_NAMESPACE)/zones.txt \
		> $@

$(DB_NAMESPACE)/%.o: $(DB_NAMESPACE)/%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(DB_NAMESPACE)/zone_infos.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_policies.h: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_policies.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_registry.h: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zone_registry.cpp: $(DB_NAMESPACE)/zone_infos.h

$(DB_NAMESPACE)/zones.txt: $(DB_NAMESPACE)/zone_infos.h

# Generate the zones.txt and zonedb files under the $(DB_NAMESPACE) directory.
$(DB_NAMESPACE)/zone_infos.h: tzfiles
	mkdir -p $(DB_NAMESPACE); \
	cd $(DB_NAMESPACE); \
	$(ACE_TIME_TOOLS)/src/acetimetools/tzcompiler.py \
		--input_dir ../tzfiles \
		--tz_version $(TZ_VERSION) \
		--language arduino,zonelist \
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		--start_year $(START_YEAR) \
		--until_year $(UNTIL_YEAR)

# Extract and copy the tzfiles into a local directory to allow parallel
# execution of validation/*Test makefiles.
tzfiles:
	$(ACE_TIME_TOOLS)/copytz.sh --tag $(TZ_VERSION) ../../tz tzfiles

more_clean:
	rm -rf $(DB_NAMESPACE) tzfiles
