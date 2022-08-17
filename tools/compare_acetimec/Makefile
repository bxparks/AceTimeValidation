CFLAGS := -Wall -Wextra -Werror -ansi -pedantic -std=c11 \
	-I../../../AceTimeC/src

generate_data.out: \
	generate_data.o \
	test_data.o \
	val_data.o \
	../../../AceTimeC/src/acetimec.a
	$(CC) -o $@ $^

generate_data.o: generate_data.c test_data.h val_data.h

test_data.o: test_data.c test_data.h

val_data.o: val_data.c val_data.h

validation_data.json: generate_data.out small.txt
	./generate_data.out \
		--start_year 2000 \
		--until_year 2050 \
		< small.txt \
		> $@

clean:
	rm -f generate_data.out validation_data.json *.o
