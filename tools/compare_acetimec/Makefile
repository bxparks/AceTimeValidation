CFLAGS := -Wall -Wextra -Werror -std=gnu11 \
	-I../../../acetimec/src

compare_acetimec.out: \
	compare_acetimec.o \
	test_data.o \
	sampling.o \
	../../../acetimec/src/acetimec.a
	$(CC) -o $@ $^

compare_acetimec.o: compare_acetimec.c test_data.h sampling.h

sampling.o: sampling.c sampling.h

test_data.o: test_data.c test_data.h

validation_data.json: compare_acetimec.out small.txt Makefile
	./compare_acetimec.out \
		--start_year 2000 \
		--until_year 2100 \
		--epoch_year 2050 \
		--zonedb zonedball \
		< small.txt \
		> $@

clean:
	rm -f *.o *.out validation_data.json
