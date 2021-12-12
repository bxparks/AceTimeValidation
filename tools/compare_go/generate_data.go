package main

import (
	"bufio"
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
	"time"
)

const (
	// Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
	// (2000-01-01 00:00:00)
	secondsSinceUnixEpoch = 946684800
)

//-----------------------------------------------------------------------------

// Generate the validation test data for AceTime using Go Lang.
//
// Usage:
// $ go run generate_data.go [--] [--help]
//		[--start_year start]
//  	[--until_year until]
//		< zones.txt
//		> validation_data.json
func main() {
	_, err := parseArgs()
	if err != nil {
		fmt.Println("Error:", err)
		usage()
		return
	}
	if help {
		usage()
		return
	}

	zones := readZones()
	testData := processZones(zones)
	validationData := createValidationData(testData)
	printJson(validationData)
}

//-----------------------------------------------------------------------------

// Default values of various flags.
var (
	startYear        = 2000
	untilYear        = 2050
	help             = false
	samplingInterval = 22 // hours
)

func usage() {
	fmt.Println("Usage: go run generate_data.go [--] [--help]")
	fmt.Println("  [--start_year start] [--until_year until]")
	fmt.Println("  [--sampling_interval hours]")
	fmt.Println("  < zones.txt > validation_data.json")
}

// ParseArgs() is a function that parses the command line arguments in os.Args,
// looking for optional long flags (beginning with two dashes), optional short
// flags (beginning with one dash), and positional arguments at the end. Returns
// a slice of the remaining positional arguments (which may be an empty slice)
// and an err object.
func parseArgs() ([]string, error) {
	args := os.Args
	argc := len(args)

	// Loop through each command line argument, looking for optional flags,
	// and terminating if a positional argument is found.
	i := 1
	var err error
	for ; i < argc; i++ {
		if args[i] == "--start_year" {
			i++
			if i >= argc {
				return args[i:], errors.New("--start_year has no value")
			}
			startYear, err = strconv.Atoi(args[i])
			if err != nil {
				return args[i:], errors.New("--start_year have integer value")
			}
		} else if args[i] == "--until_year" {
			i++
			if i >= argc {
				return args[i:], errors.New("--start_year has no value")
			}
			untilYear, err = strconv.Atoi(args[i])
			if err != nil {
				return args[i:], errors.New("--until_year have integer value")
			}
		} else if args[i] == "--sampling_interval" {
			i++
			if i >= argc {
				return args[i:], errors.New("--sampling_interval has no value")
			}
			untilYear, err = strconv.Atoi(args[i])
			if err != nil {
				return args[i:], errors.New("--until_year have integer value")
			}
		} else if args[i] == "--help" {
			help = true
			return args[i:], nil
		} else if args[i] == "--" {
			i++
			break
		} else if len(args[i]) > 0 && args[i][0] == '-' {
			return args[i:], errors.New(fmt.Sprintf("Invalid flag '%s'", args[i]))
		}
		fmt.Printf("%d: %s\n", i, args[i])
	}

	return args[i:], nil
}

//-----------------------------------------------------------------------------

// The collection of all zones and their test items.
type ValidationDataType struct {
	StartYear      int          `json:"start_year"`
	UntilYear      int          `json:"until_year"`
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
	return testItems, nil
}

// AddTransition() finds the DST transitions of the timezone, and creates
// a testItem for the 'before' time, and a testItem for the 'after' time.
func addTransitions(
	testItems []TestItemType,
	tz *time.Location,
) []TestItemType {

	transitions := findTransitions(tz)
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
	epochSeconds := int(unixSeconds - secondsSinceUnixEpoch)
	// seconds
	totalOffset := UtcOffsetMinutes(t) * 60
	// Go lang time package does not provide the DST offset.
	dstOffset := 0
	abbrev := Abbreviation(t)

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

// A tuple that represents the before and after time of a timezone transition.
type TransitionTimes struct {
	before time.Time
	after  time.Time
}

// findTransitions() finds the timezone transitions and returns an array of
// tuples of (before, after).
func findTransitions(tz *time.Location) []TransitionTimes {
	dt := time.Date(startYear, 1, 1, 0, 0, 0, 0, time.UTC)
	dtLocal := dt.In(tz)

	transitions := make([]TransitionTimes, 0, 500)
	for {
		samplingIntervalNanos := samplingInterval * 3600 * 1000000000
		intervalDuration := time.Duration(samplingIntervalNanos)
		dtNext := dt.Add(intervalDuration)
		dtNextLocal := dtNext.In(tz)
		if dtNextLocal.Year() >= untilYear {
			break
		}

		// Look for utc offset transition
		if isTransition(dtLocal, dtNextLocal) {
			dtLeft, dtRight := binarySearchTransition(tz, dt, dtNext)
			dtLeftLocal := dtLeft.In(tz)
			dtRightLocal := dtRight.In(tz)
			transitions = append(
				transitions, TransitionTimes{dtLeftLocal, dtRightLocal})
		}

		dt = dtNext
		dtLocal = dtNextLocal
	}

	return transitions
}

func isTransition(before time.Time, after time.Time) bool {
	return UtcOffsetMinutes(before) != UtcOffsetMinutes(after)
}

func binarySearchTransition(
	tz *time.Location,
	dtLeft time.Time,
	dtRight time.Time) (time.Time, time.Time) {

	dtLeftLocal := dtLeft.In(tz)
	for {
		duration := dtRight.Sub(dtLeft)
		durationMinutes := int(duration.Minutes())
		deltaMinutes := durationMinutes / 2
		if deltaMinutes == 0 {
			break
		}

		dtMid := dtLeft.Add(time.Duration(deltaMinutes * 60 * 1000000000))
		dtMidLocal := dtMid.In(tz)
		if isTransition(dtLeftLocal, dtMidLocal) {
			dtRight = dtMid
		} else {
			dtLeft = dtMid
			dtLeftLocal = dtMidLocal
		}
	}
	return dtLeft, dtRight
}

//-----------------------------------------------------------------------------

// UtcOffsetMinutes() returns the UTC offset of the given time as minutes.
func UtcOffsetMinutes(t time.Time) int {
	utcOffsetString := t.Format("-07:00")
	return convertOffsetStringToMinutes(utcOffsetString)
}

func convertOffsetStringToMinutes(offset string) int {
	hourString := offset[0:3]
	minuteString := offset[4:6]
	hour, _ := strconv.Atoi(hourString)
	minute, _ := strconv.Atoi(minuteString)
	return convertHourMinuteToMinutes(hour, minute)
}

func convertHourMinuteToMinutes(hour int, minute int) int {
	sign := 1
	if hour < 0 {
		sign = -1
		hour = -hour
	}
	minutes := hour*60 + minute
	return sign * minutes
}

func Abbreviation(t time.Time) string {
	abbrev := t.Format("MST")
	return abbrev
}

//-----------------------------------------------------------------------------

func createValidationData(testData TestDataType) ValidationDataType {
	return ValidationDataType{
		StartYear:      startYear,
		UntilYear:      untilYear,
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
