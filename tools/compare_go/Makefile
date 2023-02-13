compare_go.out: compare_go.go transitions.go
	go build -o $@

validation_data.json: small.txt compare_go.out
	./compare_go.out < small.txt > $@

clean:
	rm -f compare_go.out validation_data.json
