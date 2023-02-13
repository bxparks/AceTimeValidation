GENERATED := validation_data.json

all:
	@echo 'No compilation necessary'

validation_data.json: small.txt compare_zoneinfo.py tdgenerator.py
	./compare_zoneinfo.py < small.txt > $@

clean:
	rm -f $(GENERATED)
