GENERATED := validation_data.json

all:
	@echo 'No compilation necessary'

validation_data.json: small.txt compare_dateutil.py tdgenerator.py
	./compare_dateutil.py < small.txt > $@

clean:
	rm -f $(GENERATED)
