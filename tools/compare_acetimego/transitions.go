package main

import (
	"github.com/bxparks/acetimego/acetime"
)

// A tuple that represents the before and after time of a timezone transition.
type TransitionTimes struct {
	before acetime.ATime
	after  acetime.ATime
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

	samplingIntervalSeconds := acetime.ATime(samplingInterval * 3600)
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
	t1 acetime.ATime, t2 acetime.ATime, tz *acetime.TimeZone) int8 {

	ze1 := acetime.NewZonedExtraFromEpochSeconds(t1, tz)
	if ze1.Zetype == acetime.ZonedExtraErr {
		return -1
	}
	ze2 := acetime.NewZonedExtraFromEpochSeconds(t2, tz)
	if ze2.Zetype == acetime.ZonedExtraErr {
		return -1
	}

	offset1 := ze1.StdOffsetSeconds + ze1.DstOffsetSeconds
	offset2 := ze2.StdOffsetSeconds + ze2.DstOffsetSeconds
	if offset1 != offset2 {
		return 1
	} else if ze1.DstOffsetSeconds != ze2.DstOffsetSeconds {
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
	left acetime.ATime,
	right acetime.ATime,
	tz *acetime.TimeZone,
) (acetime.ATime, acetime.ATime, int8) {

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
