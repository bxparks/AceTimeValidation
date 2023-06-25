CFLAGS := -Wall -Wextra -Werror -std=gnu11

compare_libc.out: compare_libc.o sampling.o test_data.o
	$(CC) -o $@ $^

compare_libc.o: compare_libc.c sampling.h test_data.h

sampling.o: sampling.c sampling.h

test_data.o: test_data.c test_data.h

validation_data.json: compare_libc.out small.txt
	./compare_libc.out \
		--start_year 2000 \
		--until_year 2100 \
		--epoch_year 2050 \
		< small.txt \
		> $@

clean:
	rm -f *.o *.out validation_data.json
