GENERATED := validation_data.json

all:
	@echo 'No compilation necessary'

validation_data.json:
	./compare_pytz.py < small.txt > $@

clean:
	rm -f $(GENERATED)
