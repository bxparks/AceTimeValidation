package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"sort"
	"strconv"
	"strings"
	"time"
)

//-----------------------------------------------------------------------------

// Generate the validation test data for AceTime using Go Lang.
//
// Usage:
//
// $ go run generate_data.go [--] [--help]
// [--start_year start]
// [--until_year until]
// [--epoch_year year]
// < zones.txt
// > validation_data.json
func main() {
	parseArgs()
	dt := time.Date(epochYear, 1, 1, 0, 0, 0, 0, time.UTC)
	secondsToAceTimeEpochFromUnixEpoch = dt.Unix()

	zones := readZones()
	testData := processZones(zones)
	validationData := createValidationData(testData)
	printJson(validationData)
}

//-----------------------------------------------------------------------------

// Default values of various flags.
var (
	startYear        = 2000
	untilYear        = 2100
	epochYear        = 2050
	samplingInterval = 22 // hours

	// Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
	// (2050-01-01 00:00:00), configurable using --epoch_year.
	secondsToAceTimeEpochFromUnixEpoch int64
)

func usage() {
	fmt.Println(`Usage: go run generate_data.go [--] [--help]
        [--sampling_interval hours] [--start_year start] [--until_year until]
				[--epoch_year year]
        < zones.txt > validation_data.json`)
}

// ParseArgs() is a function that parses the command line arguments in os.Args,
// looking for optional long flags (beginning with two dashes), optional short
// flags (beginning with one dash), and positional arguments at the end. Returns
// a slice of the remaining positional arguments which maybe empty slice.
//
// There is probably std library module that already handles command line
// argument parsing. I rolled my own to help me learn the Go lang.
func parseArgs() []string {
	// Shift out the name of the command in Args[0]
	args := os.Args[1:]

	var err error

	// Loop through each command line argument, looking for optional flags,
	// and terminating if a positional argument is found.
	for len(args) > 0 {
		s := args[0]
		if s == "--start_year" {
			args = args[1:]
			if len(args) == 0 {
				fmt.Println("--start_year must have an argument")
				os.Exit(1)
			}
			s = args[0]
			startYear, err = strconv.Atoi(s)
			if err != nil {
				fmt.Println("--start_year must have an integer value")
				os.Exit(1)
			}
		} else if s == "--until_year" {
			args = args[1:]
			if len(args) == 0 {
				fmt.Println("--until_year must have an argument")
				os.Exit(1)
			}
			s = args[0]
			untilYear, err = strconv.Atoi(s)
			if err != nil {
				fmt.Println("--until_year must have an integer value")
				os.Exit(1)
			}
		} else if s == "--epoch_year" {
			args = args[1:]
			if len(args) == 0 {
				fmt.Println("--epoch_year must have an argument")
				os.Exit(1)
			}
			s = args[0]
			epochYear, err = strconv.Atoi(s)
			if err != nil {
				fmt.Println("--epoch_year must have an integer value")
				os.Exit(1)
			}
		} else if s == "--sampling_interval" {
			args = args[1:]
			if len(args) == 0 {
				fmt.Println("--sampling_interval must have an argument")
				os.Exit(1)
			}
			s = args[0]
			samplingInterval, err = strconv.Atoi(s)
			if err != nil {
				fmt.Println("--sampling_interval must have an integer value")
				os.Exit(1)
			}
		} else if s == "--help" {
			usage()
			os.Exit(0)
		} else if s == "--" {
			args = args[1:]
			break
		} else if len(s) > 0 && s[0] == '-' {
			fmt.Printf("Invalid flag '%s'\n", s)
			os.Exit(1)
		} else {
			break
		}

		args = args[1:]
	}

	return args
}

//-----------------------------------------------------------------------------

// The collection of all zones and their test items.
type ValidationDataType struct {
	StartYear      int          `json:"start_year"`
	UntilYear      int          `json:"until_year"`
	EpochYear      int          `json:"epoch_year"`
	Source         string       `json:"source"`
	Version        string       `json:"version"`
	TzVersion      string       `json:"tz_version"`
	HasValidAbbrev bool         `json:"has_valid_abbrev"`
	HasValidDst    bool         `json:"has_valid_dst"`
	TestData       TestDataType `json:"test_data"`
}

// A map of zoneName to an array of TestItems.
type TestDataType map[string][]TestItemType

// A test item struct contains the epochSeconds with its local date time
// components.
type TestItemType struct {
	EpochSeconds int    `json:"epoch"`
	TotalOffset  int    `json:"total_offset"`
	DstOffset    int    `json:"dst_offset"`
	Year         int    `json:"y"`
	Month        int    `json:"M"`
	Day          int    `json:"d"`
	Hour         int    `json:"h"`
	Minute       int    `json:"m"`
	Second       int    `json:"s"`
	Abbrev       string `json:"abbrev"`
	ItemType     string `json:"type"` // "A", "B", "S", "T" or "Y"
}

//-----------------------------------------------------------------------------

// ReadZones reads the list of zones from the stdin and returns them in an array
// of strings.
func readZones() []string {
	zones := make([]string, 0, 600)

	reader := bufio.NewReader(os.Stdin)
	for {
		message, err := reader.ReadString('\n')
		if err != nil {
			break
		}

		// Skip comments or blank lines.
		trimmed := strings.Trim(message, " \t\n")
		size := len(trimmed)
		if size == 0 || trimmed[0] == '#' {
			continue
		}

		// Extract zone name.
		message = strings.TrimRight(message, "\n")
		zones = append(zones, message)
	}

	return zones
}

//-----------------------------------------------------------------------------

// ProcessZones() iterates over the list of zones and calls processZone().
func processZones(zones []string) TestDataType {
	testData := make(TestDataType)
	for _, zoneName := range zones {
		testItems, err := processZone(zoneName)
		if err != nil {
			fmt.Printf("Unable to process zone '%s'\n", zoneName)
			continue
		}

		testData[zoneName] = testItems
	}

	return testData
}

// ProcessZone() adds the DST transitions and sample data for each month.
func processZone(zoneName string) ([]TestItemType, error) {
	tz, err := time.LoadLocation(zoneName)
	if err != nil {
		return nil, err
	}

	testItems := make([]TestItemType, 0, 500)
	testItems = addTransitions(testItems, tz)
	testItems = addSamples(testItems, tz)
	sortSamples(testItems)
	return testItems, nil
}

func sortSamples(testItems []TestItemType) {
	sort.Slice(testItems, func(i, j int) bool {
		return testItems[i].EpochSeconds < testItems[j].EpochSeconds
	})
}

// AddTransition() finds the DST transitions of the timezone, and creates
// a testItem for the 'before' time, and a testItem for the 'after' time.
func addTransitions(
	testItems []TestItemType,
	tz *time.Location,
) []TestItemType {

	transitions := findTransitions(startYear, untilYear, samplingInterval, tz)
	for _, transition := range transitions {
		beforeTestItem := createTestItem(transition.before, "A")
		afterTestItem := createTestItem(transition.after, "B")
		testItems = append(testItems, beforeTestItem)
		testItems = append(testItems, afterTestItem)
	}

	return testItems
}

// AddSamples() add a testItem for each month for the given zone.
func addSamples(
	testItems []TestItemType,
	tz *time.Location,
) []TestItemType {
	for year := startYear; year < untilYear; year++ {
		for month := 1; month <= 12; month++ {
			dtSample := time.Date(year, time.Month(month), 2, 0, 0, 0, 0, tz)
			sampleTestItem := createTestItem(dtSample, "S")
			testItems = append(testItems, sampleTestItem)
		}
	}

	return testItems
}

func createTestItem(t time.Time, itemType string) TestItemType {
	unixSeconds := t.Unix()
	epochSeconds := int(unixSeconds - secondsToAceTimeEpochFromUnixEpoch)

	// seconds
	totalOffset := utcOffsetMinutes(t) * 60
	// Go lang time package does not provide the DST offset.
	dstOffset := 0
	abbrev := abbreviation(t)

	return TestItemType{
		epochSeconds,
		totalOffset,
		dstOffset,
		t.Year(),
		int(t.Month()),
		t.Day(),
		t.Hour(),
		t.Minute(),
		t.Second(),
		abbrev,
		itemType,
	}
}

func abbreviation(t time.Time) string {
	abbrev := t.Format("MST")
	return abbrev
}

//-----------------------------------------------------------------------------

func createValidationData(testData TestDataType) ValidationDataType {
	return ValidationDataType{
		StartYear:      startYear,
		UntilYear:      untilYear,
		EpochYear:      epochYear,
		Source:         "go",
		Version:        "",
		TzVersion:      "",
		HasValidAbbrev: true,
		HasValidDst:    false,
		TestData:       testData,
	}
}

//-----------------------------------------------------------------------------

func printJson(validationData ValidationDataType) {
	jsonData, err := json.MarshalIndent(validationData, "", "  ")
	if err != nil {
		log.Println(err)
	}
	fmt.Println(string(jsonData))
}
