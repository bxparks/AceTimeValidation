generate_data.out: generate_data.go
	go build -o $@ $<

validation_data.json: small.txt generate_data.out
	./generate_data.out < small.txt > $@

fmt:
	go fmt generate_data.go

clean:
	rm -f generate_data.out validation_data.json
