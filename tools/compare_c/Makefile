CFLAGS := -Wall -Wextra -Werror -std=c11 \
	-D _GNU_SOURCE

generate_data.out: generate_data.o sampling.o test_data.o
	$(CC) -o $@ $^

generate_data.o: generate_data.c sampling.h test_data.h

sampling.o: sampling.c sampling.h

test_data.o: test_data.c test_data.h

validation_data.json: generate_data.out small.txt
	./generate_data.out \
		--start_year 2000 \
		--until_year 2100 \
		--epoch_year 2050 \
		< small.txt \
		> $@

clean:
	rm -f *.o *.out validation_data.json
