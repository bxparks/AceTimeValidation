package main

import (
	"fmt"
	"strconv"
	"time"
)

// A tuple that represents the before and after time of a timezone transition.
type TransitionTimes struct {
	before time.Time
	after  time.Time
}

// findTransitions() finds the timezone transitions and returns an array of
// tuples of (before, after).
func findTransitions(
	startYear int,
	untilYear int,
	samplingInterval int, // hours
	tz *time.Location) []TransitionTimes {

	dt := time.Date(startYear, 1, 1, 0, 0, 0, 0, time.UTC)
	dtLocal := dt.In(tz)

	transitions := make([]TransitionTimes, 0, 500)
	samplingIntervalNanos := samplingInterval * 3600 * 1000000000
	intervalDuration := time.Duration(samplingIntervalNanos)
	for {
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

// Determine if a transition occurs by comparing the total offset in minutes.
// The go.time library does not expose the DST offset at a given time, so we
// cannot determine if there was a silent transition (i.e. a transition whose
// total offset remained the same, but the DST offset and STD offset changed and
// cancelled each other, sometimes resulting in an abbreviation change.
func isTransition(before time.Time, after time.Time) bool {
	return utcOffsetMinutes(before) != utcOffsetMinutes(after)
}

func binarySearchTransition(
	tz *time.Location,
	dtLeft time.Time,
	dtRight time.Time) (time.Time, time.Time) {

	dtLeftLocal := dtLeft.In(tz)
	for {

		// 1-second transition resolution.
		duration := dtRight.Sub(dtLeft)
		durationSeconds := int(duration.Seconds())
		deltaSeconds := durationSeconds / 2
		if deltaSeconds == 0 {
			break
		}

		dtMid := dtLeft.Add(time.Duration(deltaSeconds * int(time.Second)))
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

// utcOffsetMinutes() returns the UTC offset of the given time as minutes.
func utcOffsetMinutes(t time.Time) int {
	utcOffsetString := t.Format("-07:00")
	length := len(utcOffsetString)
	if length != 6 {
		panic(fmt.Sprintf("len(\"%v\")=%v; should be 6", utcOffsetString, length))
	}
	return convertOffsetStringToMinutes(utcOffsetString)
}

func convertOffsetStringToMinutes(offset string) int {
	signString := offset[0:1]
	var sign int
	if signString == "-" {
		sign = -1
	} else {
		sign = 1
	}

	hourString := offset[1:3]
	minuteString := offset[4:6]
	hour, _ := strconv.Atoi(hourString)
	minute, _ := strconv.Atoi(minuteString)

	minutes := sign * (hour*60 + minute)
	return minutes
}
