CFLAGS := -Wall -Wextra -Werror -std=c11 \
	-D _GNU_SOURCE

compare_c.out: compare_c.o sampling.o test_data.o
	$(CC) -o $@ $^

compare_c.o: compare_c.c sampling.h test_data.h

sampling.o: sampling.c sampling.h

test_data.o: test_data.c test_data.h

validation_data.json: compare_c.out small.txt
	./compare_c.out \
		--start_year 2000 \
		--until_year 2100 \
		--epoch_year 2050 \
		< small.txt \
		> $@

clean:
	rm -f *.o *.out validation_data.json
