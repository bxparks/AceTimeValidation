# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

DB_NAMESPACE := zonedbxhd
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
APP_NAME := HinnantExtendedTest
ARDUINO_LIBS := AUnit AceCommon AceTime AceSorting
MORE_CLEAN := more_clean
include ../../../EpoxyDuino/EpoxyDuino.mk

# Configuration parameters of the AceTime zonedb.
SCOPE := extended
TZ_VERSION := 2023d
ACE_TIME_EPOCH_YEAR := 2050
DB_START_YEAR := 2000
DB_UNTIL_YEAR := 2200
VAL_START_YEAR := 2000
VAL_UNTIL_YEAR := 2100

# Location of TZDB and various tools.
TZ_REPO := $(abspath ../../../tz)
VALIDATION_TOOLS := $(abspath ../../tools)
ACE_TIME_TOOLS := $(abspath ../../../AceTimeTools)

validation_data.cpp: validation_data.h

validation_tests.cpp: validation_data.h

validation_data.h: validation_data.json
	$(VALIDATION_TOOLS)/generate_validation/generate_validation.py \
		--tz_version $(TZ_VERSION) \
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		< validation_data.json

validation_data.json: $(DB_NAMESPACE)/zones.txt \
$(VALIDATION_TOOLS)/compare_hinnant/compare_hinnant.out
	$(VALIDATION_TOOLS)/compare_hinnant/compare_hinnant.out \
		--tz_version $(TZ_VERSION) \
		--start_year $(VAL_START_YEAR) \
		--until_year $(VAL_UNTIL_YEAR) \
		--epoch_year $(ACE_TIME_EPOCH_YEAR) \
		< $(DB_NAMESPACE)/zones.txt \
		> $@

$(VALIDATION_TOOLS)/compare_hinnant/compare_hinnant.out:
	$(MAKE) -C $(VALIDATION_TOOLS)/compare_hinnant/

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
		--actions zonedb,zonelist \
		--languages arduino \
		--scope $(SCOPE) \
		--db_namespace $(DB_NAMESPACE) \
		--start_year $(DB_START_YEAR) \
		--until_year $(DB_UNTIL_YEAR)

more_clean:
	rm -rf $(DB_NAMESPACE)
