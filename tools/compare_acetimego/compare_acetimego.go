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

	"github.com/bxparks/acetimego/acetime"
	"github.com/bxparks/acetimego/zonedball"
)

//-----------------------------------------------------------------------------

// Generate the validation test data for AceTime using Go Lang.
//
// Usage:
//
// $ go run compare_acetimego.go [--] [--help]
// [--start_year start]
// [--until_year until]
// [--epoch_year year]
// < zones.txt
// > validation_data.json
func main() {
	parseArgs()

	dt := acetime.LocalDateTime{int16(epochYear), 1, 1, 0, 0, 0, 0 /*Fold*/}
	epochOffset = -int64(dt.EpochSeconds())
	zoneManager = acetime.NewZoneManager(&zonedball.DataContext)

	zones := readZones()
	testData := processZones(zones)
	validationData := createValidationData(testData)
	printJson(validationData)
}

//-----------------------------------------------------------------------------

// Default values of various flags.
var (
	startYear = 2000
	untilYear = 2100
	epochYear = 2050

	// Number of seconds to add to UnixSeconds to get the offset from the
	// rendered --epoch_year (default 2050).
	epochOffset int64

	zoneManager acetime.ZoneManager
)

const (
	samplingInterval = 22 // hours
)

func usage() {
	fmt.Println(`Usage: go run compare_acetimego.go [--] [--help]
        [--start_year start] [--until_year until] [--epoch_year year]
        < zones.txt > validation_data.json`)
}

// ParseArgs() is a function that parses the command line arguments in os.Args,
// looking for optional long flags (beginning with two dashes), optional short
// flags (beginning with one dash), and positional arguments at the end. Returns
// a slice of the remaining positional arguments which maybe empty slice.
//
// There is probably a std library module that already handles command line
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
type ValidationData struct {
	StartYear         int      `json:"start_year"`
	UntilYear         int      `json:"until_year"`
	EpochYear         int      `json:"epoch_year"`
	Source            string   `json:"source"`
	Scope             string   `json:"scope"`
	Version           string   `json:"version"`
	TzVersion         string   `json:"tz_version"`
	HasValidAbbrev    bool     `json:"has_valid_abbrev"`
	HasValidDst       bool     `json:"has_valid_dst"`
	OffsetGranularity int      `json:"offset_granularity"`
	TData             TestData `json:"test_data"`
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

// ProcessZones() iterates over the list of zones and calls processZone().
func processZones(zones []string) TestData {
	testData := make(TestData)
	for i, zoneName := range zones {
		print("[")
		print(i)
		print("]")
		print(" ")
		println(zoneName)
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
func processZone(zoneName string) (TestEntry, error) {
	tz := zoneManager.TimeZoneFromName(zoneName)
	if tz.IsError() {
		return TestEntry{}, errors.New("Unable to create zoneName")
	}

	transitions := createTransitions(&tz)
	samples := createSamples(&tz)
	entry := TestEntry{transitions, samples}
	return entry, nil
}

//-----------------------------------------------------------------------------

// createTransition() finds the DST transitions of the timezone, and creates
// a testItem for the 'before' time, and a testItem for the 'after' time.
func createTransitions(tz *acetime.TimeZone) []TestItem {
	testItems := make([]TestItem, 0, 500)
	transitions := findTransitions(startYear, untilYear, samplingInterval, tz)
	for _, transition := range transitions {
		var beforeItem TestItem
		var afterItem TestItem
		if transition.result == 1 {
			beforeItem = createTestItem(transition.before, "A", tz)
			afterItem = createTestItem(transition.after, "B", tz)
		} else if transition.result == 2 {
			beforeItem = createTestItem(transition.before, "a", tz)
			afterItem = createTestItem(transition.after, "b", tz)
		}
		testItems = append(testItems, beforeItem)
		testItems = append(testItems, afterItem)
	}

	return testItems
}

// A tuple that represents the before and after time of a timezone transition.
type TransitionTimes struct {
	before acetime.Time
	after  acetime.Time
	result int8
}

// findTransitions() finds the timezone transitions and returns an array of
// tuples of (before, after).
func findTransitions(
	startYear int,
	untilYear int,
	samplingInterval int, // hours
	tz *acetime.TimeZone) []TransitionTimes {

	transitions := make([]TransitionTimes, 0, 500)

	ldt := acetime.LocalDateTime{int16(startYear), 1, 1, 0, 0, 0, 0}
	zdt := acetime.NewZonedDateTimeFromLocalDateTime(&ldt, tz)
	if zdt.IsError() {
		return transitions
	}
	t := zdt.EpochSeconds()

	samplingIntervalSeconds := acetime.Time(samplingInterval * 3600)
	for {
		tNext := t + samplingIntervalSeconds
		zdtNext := acetime.NewZonedDateTimeFromEpochSeconds(tNext, tz)
		if zdtNext.IsError() {
			continue
		}
		if zdtNext.Year >= int16(untilYear) {
			break
		}

		// Look for utc offset transition
		if isTransition(t, tNext, tz) > 0 {
			left, right, result := binarySearchTransition(t, tNext, tz)
			transitions = append(transitions, TransitionTimes{left, right, result})
		}

		t = tNext
	}

	return transitions
}

// Determine if a transition occurs by comparing the total offset in minutes.
// The result code is:
// * -1 - error
// * 0 - no transition
// * 1 - regular transition (total UTC offset is different)
// * 2 - silent transition (both STD or DST changed and canceled each other)
func isTransition(
	t1 acetime.Time, t2 acetime.Time, tz *acetime.TimeZone) int8 {

	extra1 := acetime.NewZonedExtraFromEpochSeconds(t1, tz)
	if extra1.IsError() {
		return -1
	}
	extra2 := acetime.NewZonedExtraFromEpochSeconds(t2, tz)
	if extra2.IsError() {
		return -1
	}

	if extra1.OffsetSeconds() != extra2.OffsetSeconds() {
		return 1
	} else if extra1.DstOffsetSeconds != extra2.DstOffsetSeconds {
		return 2
	}
	return 0
}

// Determine if (left, right) is a transition. The result code is:
// * -1 - error
// * 0 - no transition
// * 1 - transition (total UTC offset is different)
// * 2 - silent transition (both STD and DST changed and cancelled each other)
func binarySearchTransition(
	left acetime.Time,
	right acetime.Time,
	tz *acetime.TimeZone,
) (acetime.Time, acetime.Time, int8) {

	var result int8
	for {

		// 1-second transition resolution.
		delta := (right - left) / 2
		if delta == 0 {
			result = isTransition(left, right, tz)
			break
		}

		mid := left + delta
		result = isTransition(left, mid, tz)
		if result == -1 {
			break
		} else if result == 0 {
			left = mid
		} else {
			right = mid
		}
	}
	return left, right, result
}

//-----------------------------------------------------------------------------

// createSamples() add a testItem for the second day of each month for the given
// zone.
func createSamples(tz *acetime.TimeZone) []TestItem {
	testItems := make([]TestItem, 0, 2000)
	for year := int16(startYear); year < int16(untilYear); year++ {
		for month := uint8(1); month <= 12; month++ {
			itemType := "S"
			// If the second day of the month is in the gap, try subsequent days until
			// we find one that is not in a gap.
			for day := uint8(2); day <= 28; day++ {
				ldt := acetime.LocalDateTime{year, month, day, 0, 0, 0, 0 /*Fold*/}
				extra := acetime.NewZonedExtraFromLocalDateTime(&ldt, tz)

				if extra.FoldType == acetime.FoldTypeExact ||
					extra.FoldType == acetime.FoldTypeOverlap {

					zdt := acetime.NewZonedDateTimeFromLocalDateTime(&ldt, tz)
					sampleTestItem := createTestItem(zdt.EpochSeconds(), itemType, tz)
					testItems = append(testItems, sampleTestItem)
					break
				}
				itemType = "T" // subsequent samples marked with "T" instead of "S"
			}
		}
	}

	return testItems
}

//-----------------------------------------------------------------------------

func createTestItem(
	epochSeconds acetime.Time, itemType string, tz *acetime.TimeZone) TestItem {

	zdt := acetime.NewZonedDateTimeFromEpochSeconds(epochSeconds, tz)
	extra := acetime.NewZonedExtraFromEpochSeconds(epochSeconds, tz)

	return TestItem{
		EpochSeconds: int64(epochSeconds) + epochOffset,
		TotalOffset:  zdt.OffsetSeconds,
		DstOffset:    extra.DstOffsetSeconds,
		Year:         zdt.Year,
		Month:        zdt.Month,
		Day:          zdt.Day,
		Hour:         zdt.Hour,
		Minute:       zdt.Minute,
		Second:       zdt.Second,
		Abbrev:       extra.Abbrev,
		ItemType:     itemType,
	}
}

//-----------------------------------------------------------------------------

func createValidationData(testData TestData) ValidationData {
	return ValidationData{
		StartYear:         startYear,
		UntilYear:         untilYear,
		EpochYear:         epochYear,
		Scope:             "complete",
		Source:            "acetimego",
		Version:           "",
		TzVersion:         zonedball.TzDatabaseVersion,
		HasValidAbbrev:    true,
		HasValidDst:       true,
		OffsetGranularity: 1,
		TData:             testData,
	}
}

//-----------------------------------------------------------------------------

func printJson(validationData ValidationData) {
	jsonData, err := json.MarshalIndent(validationData, "", "  ")
	if err != nil {
		log.Println(err)
	}
	fmt.Println(string(jsonData))
}
