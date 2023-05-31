// Copyright 2023 Brian T. Park
//
// MIT License
//
// Read the validation JSON data on the stdin, and print out a flatten version
// that is easier to read by humans for debugging purposes.
//
// Usage:
// $ go run flatten.go < data.json > data.txt
//
// Performance:
// The Go version is actually 40-50% slower than the Python version. This is
// probably because the Python json package is implemented in C.

package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
)

//-----------------------------------------------------------------------------

// The collection of all zones and their test items.
type ValidationData struct {
	StartYear      int      `json:"start_year"`
	UntilYear      int      `json:"until_year"`
	EpochYear      int      `json:"epoch_year"`
	Source         string   `json:"source"`
	Version        string   `json:"version"`
	TzVersion      string   `json:"tz_version"`
	HasValidAbbrev bool     `json:"has_valid_abbrev"`
	HasValidDst    bool     `json:"has_valid_dst"`
	TData          TestData `json:"test_data"`
}

// A map of zoneName to an array of TestItems.
type TestData map[string]TestEntry

// A pair of transitions and samples.
type TestEntry struct {
	Transitions []TestItem `json:"transitions"`
	Samples     []TestItem `json:"samples"`
}

// A test item struct contains the epochSeconds with its local date time
// components.
type TestItem struct {
	EpochSeconds int64  `json:"epoch"`
	TotalOffset  int32  `json:"total_offset"`
	DstOffset    int32  `json:"dst_offset"`
	Year         int16  `json:"y"`
	Month        uint8  `json:"M"`
	Day          uint8  `json:"d"`
	Hour         uint8  `json:"h"`
	Minute       uint8  `json:"m"`
	Second       uint8  `json:"s"`
	Abbrev       string `json:"abbrev"`
	ItemType     string `json:"type"` // "A", "B", "a", "b", "S", "T"
}

//-----------------------------------------------------------------------------

func main() {
	data := readData()
	writeData(data)
}

func readData() *ValidationData {
	var data ValidationData
	err := json.NewDecoder(os.Stdin).Decode(&data)
	if err != nil {
		log.Fatal(err)
	}
	return &data
}

func writeData(data *ValidationData) {
	fmt.Println("HEADER")
	fmt.Println("start_year", data.StartYear)
	fmt.Println("until_year", data.UntilYear)
	fmt.Println("epoch_year", data.EpochYear)
	fmt.Println("has_valid_abbrev", data.HasValidAbbrev)
	fmt.Println("has_valid_dst", data.HasValidDst)
	fmt.Println()

	testData := data.TData
	for zone, entry := range testData {
		writeTestEntry(zone, &entry)
	}
}

func writeTestEntry(zone string, entry *TestEntry) {
	fmt.Println("ZONE", zone)

	transitions := entry.Transitions
	fmt.Println("TRANSITIONS", len(transitions))
	writeTestItems(transitions)

	samples := entry.Samples
	fmt.Println("SAMPLES", len(samples))
	writeTestItems(samples)
	fmt.Println()
}

func writeTestItems(items []TestItem) {
	if len(items) != 0 {
		fmt.Println(
			"# line       epoch    utc    dst    y  m  d  h  m  s  abbrev type")
	}

	for i := range items {
		item := &items[i]

		var abbrev string
		if item.Abbrev == "" {
			abbrev = "-"
		} else {
			abbrev = item.Abbrev
		}

		fmt.Printf(
			"%6d %11d %6d %6d %4d %2d %2d %2d %2d %2d %7s %4s\n",
			i,
			item.EpochSeconds,
			item.TotalOffset,
			item.DstOffset,
			item.Year,
			item.Month,
			item.Day,
			item.Hour,
			item.Minute,
			item.Second,
			abbrev,
			item.ItemType,
		)
	}
}
