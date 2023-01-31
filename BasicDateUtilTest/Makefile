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
TZ_VERSION := 2022f
ACE_TIME_EPOCH_YEAR := 2050
DB_START_YEAR := 2000
DB_UNTIL_YEAR := 10000
VAL_START_YEAR := 2000
VAL_UNTIL_YEAR := 2037

# Location of TZDB and various tools.
TZ_REPO := $(abspath ../../tz)
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
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
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
	rm -rf $(DB_NAMESPACE)
