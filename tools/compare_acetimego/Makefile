compare_acetimego.out: compare_acetimego.go transitions.go
	go build -o $@

validation_data.json: small.txt compare_acetimego.out
	./compare_acetimego.out < small.txt > $@

clean:
	rm -f compare_acetimego.out validation_data.json
