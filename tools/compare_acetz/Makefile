GENERATED := validation_data.json

all:
	@echo 'No compilation necessary'

validation_data.json: compare_acetz.py
	./compare_acetz.py < small.txt > $@

clean:
	rm -f $(GENERATED)
