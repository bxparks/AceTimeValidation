# Location of https://github.com/HowardHinnant/date, assumed to be sibling to
# this project.
HINNANT_DATE_DIR := $(abspath ../../../date)

compare_cpp.out: compare_cpp.cpp
	$(CXX) -DHAS_REMOTE_API=1 -DAUTO_DOWNLOAD=0 \
		-I $(HINNANT_DATE_DIR)/include -Wall -std=gnu++11 -o $@ $^ \
		$(HINNANT_DATE_DIR)/src/*.cpp -lcurl

validation_data.json: compare_cpp.out small.txt
	./compare_cpp.out \
		--start_year 2000 \
		--until_year 2100 \
		--epoch_year 2050 \
		< small.txt \
		> $@

clean:
	rm -f compare_cpp.out validation_data.json
