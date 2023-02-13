CFLAGS := -Wall -Wextra -Werror -std=c11 \
	-I../../../AceTimeC/src

generate_data.out: \
	generate_data.o \
	test_data.o \
	sampling.o \
	../../../AceTimeC/src/acetimec.a
	$(CC) -o $@ $^

generate_data.o: generate_data.c test_data.h sampling.h

sampling.o: sampling.c sampling.h

test_data.o: test_data.c test_data.h

validation_data.json: generate_data.out small.txt
	./generate_data.out \
		--start_year 2000 \
		--until_year 2100 \
		< small.txt \
		> $@

clean:
	rm -f *.o *.out validation_data.json
