CFLAGS := -Wall -Wextra -std=c11

generate_data.out: generate_data.c sampling.c test_data.c
	$(CC) -o $@ $^

validation_data.json: generate_data.out small.txt
	./generate_data.out \
		--start_year 2000 \
		--until_year 2100 \
		--epoch_year 2050 \
		< small.txt \
		> $@

clean:
	rm -f generate_data.out validation_data.json
