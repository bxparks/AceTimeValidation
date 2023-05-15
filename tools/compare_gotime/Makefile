compare_gotime.out: compare_gotime.go transitions.go
	go build -o $@

validation_data.json: small.txt compare_gotime.out
	./compare_gotime.out < small.txt > $@

clean:
	rm -f compare_gotime.out validation_data.json
