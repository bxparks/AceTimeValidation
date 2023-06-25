# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

OBJS := sampling.o test_data.o
APP_NAME := compare_acetime
ARDUINO_LIBS := AceCommon AceTime AceSorting
MORE_CLEAN := more_clean
include ../../../EpoxyDuino/EpoxyDuino.mk

sampling.o: sampling.cpp sampling.h

test_data.o: test_data.cpp test_data.h

validation_basic.json: compare_acetime.out small.txt
	./compare_acetime.out \
		--start_year 2000 \
		--until_year 2100 \
		--epoch_year 2050 \
		--scope basic \
		< small.txt \
		> $@

validation_extended.json: compare_acetime.out small.txt
	./compare_acetime.out \
		--start_year 2000 \
		--until_year 2100 \
		--epoch_year 2050 \
		--scope extended \
		< small.txt \
		> $@

validation_complete.json: compare_acetime.out small.txt
	./compare_acetime.out \
		--start_year 2000 \
		--until_year 2100 \
		--epoch_year 2050 \
		--scope complete \
		< small.txt \
		> $@

more_clean:
	rm -f validation_basic.json validation_extended.json
