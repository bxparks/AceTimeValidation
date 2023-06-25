# Copyright 2020 Brian T. Park
#
# MIT License

"""
Generate validation TestData using 'dateutil' package. This version was derived
from the 'validation.tdgenerator' module with the critical difference that it
does not pull in the ZoneProcessor to determine the DST transition times. This
means that this module also avoids pulling in the ZoneInfo, ZonePolicy and other
related classes from 'extractor' and 'transformer' processing pipeline.
Therefore, this module is truly dependent on only the 'dateutil' package.
"""

import logging
from datetime import datetime, timedelta, timezone
import dateutil
from dateutil.tz import gettz, resolve_imaginary, UTC
from typing import Any, Tuple, List

from acetimetools.datatypes.valtyping import (
    TestItem, TestData, ValidationData
)

# The [start, until) time interval used to search for DST transitions,
# and flag that is True if ONLY the DST changed.
TransitionTimes = Tuple[datetime, datetime, bool]


class TestDataGenerator():
    # Number of seconds from Unix Epoch (1970-01-01 00:00:00) to AceTime Epoch
    # (2000-01-01 00:00:00)
    seconds_to_ace_time_epoch_from_unix_epoch = 946684800

    def __init__(
        self,
        start_year: int,
        until_year: int,
        epoch_year: int,
        sampling_interval: int,
        detect_dst_transition: bool = True,
    ):
        """If detect_dst_transition is set to True, changes in the DST offset
        will be considered to be a time offset transition. Enabling this will
        cause additional test data points to be generated, but often they will
        conflict with the DST offsets calculated by AceTime or HinnantDate
        library. In other words, I think dateutil is incorrect for those DST
        transitions.
        """
        self.start_year = start_year
        self.until_year = until_year
        self.epoch_year = epoch_year
        self.sampling_interval = timedelta(hours=sampling_interval)
        self.detect_dst_transition = detect_dst_transition

        dt = datetime(epoch_year, 1, 1, 0, 0, 0, tzinfo=timezone.utc)
        TestDataGenerator.seconds_to_ace_time_epoch_from_unix_epoch = \
            int(dt.timestamp())

    def get_validation_data(self, zones: List[str]) -> ValidationData:
        test_data = self._create_test_data(zones)
        return {
            'start_year': self.start_year,
            'until_year': self.until_year,
            'epoch_year': self.epoch_year,
            'scope': 'complete',
            'source': 'dateutil',
            'version': str(dateutil.__version__),  # type: ignore
            'tz_version': 'unknown',
            'has_valid_abbrev': True,
            'has_valid_dst': True,
            'test_data': test_data,
        }

    def _create_test_data(self, zones: List[str]) -> TestData:
        """Create both transitions and samples test data.
        For [2000, 2038], this generates about 100,000 data points.
        """
        test_data: TestData = {}
        i = 0
        for zone_name in zones:
            logging.info(f"[{i}] {zone_name}")
            tz = gettz(zone_name)
            if not tz:
                logging.error(f"Zone '{zone_name}' not found in dateutil")
                continue

            transitions = self._create_transitions_for_zone(tz)
            samples = self._create_samples_for_zone(tz)
            if transitions or samples:
                test_data[zone_name] = {
                    "transitions": transitions,
                    "samples": samples,
                }
            i += 1

        return test_data

    def _create_samples_for_zone(self, tz: Any) -> List[TestItem]:
        """Add a TestItem for each month from start_year to until_year."""
        items: List[TestItem] = []
        for year in range(self.start_year, self.until_year):
            for month in range(1, 13):
                # Add a sample test point on the *second* of each month instead
                # of the first of the month. This prevents Jan 1, 2000 from
                # being converted to a negative epoch seconds for certain
                # timezones, which gets converted into a UTC date in 1999 when
                # ExtendedZoneProcessor is used to convert the epoch seconds
                # back to a ZonedDateTime. The UTC date in 1999 causes the
                # actual max buffer size of ExtendedZoneProcessor to become
                # different than the one predicted by BufSizeEstimator (which
                # samples whole years from 2000 until 2050), and causes the
                # AceTimeValidation/ExtendedDateUtilTest to fail on the buffer
                # size check.
                dt = resolve_imaginary(
                    datetime(year, month, 2, 0, 0, 0, tzinfo=tz)
                )
                item = self._create_test_item(dt, 'S')
                items.append(item)
        return items

    def _create_transitions_for_zone(self, tz: Any) -> List[TestItem]:
        """Add DST transitions, using 'A' and 'B' designators"""
        items: List[TestItem] = []
        transitions = self._find_transitions(tz)
        for (left, right, only_dst) in transitions:
            left_item = self._create_test_item(
                left, 'a' if only_dst else 'A')
            items.append(left_item)

            right_item = self._create_test_item(
                right, 'b' if only_dst else 'B')
            items.append(right_item)
        return items

    def _find_transitions(self, tz: Any) -> List[TransitionTimes]:
        """Find the DST transition using dateutil by sampling the time period
        from [start_year, until_year].
        """
        # TODO: Do I need to start 1 day before Jan 1 UTC, in case the
        # local time is ahead of UTC?
        dt = datetime(self.start_year, 1, 1, 0, 0, 0, tzinfo=UTC)
        dt_local = dt.astimezone(tz)

        # Check every 'sampling_interval' hours for a transition
        transitions: List[TransitionTimes] = []
        while True:
            next_dt = dt + self.sampling_interval
            next_dt_local = next_dt.astimezone(tz)
            if next_dt.year >= self.until_year:
                break

            # Look for a UTC or DST transition.
            if self._is_transition(dt_local, next_dt_local):
                # print(f'Transition between {dt_local} and {next_dt_local}')
                dt_left, dt_right = self._binary_search_transition(
                    tz, dt, next_dt)
                dt_left_local = dt_left.astimezone(tz)
                dt_right_local = dt_right.astimezone(tz)
                only_dst = self._only_dst(dt_left_local, dt_right_local)
                transitions.append((dt_left_local, dt_right_local, only_dst))

            dt = next_dt
            dt_local = next_dt_local

        return transitions

    def _is_transition(self, dt1: datetime, dt2: datetime) -> bool:
        """Determine if dt1 -> dt2 is a UTC offset transition. If
        detect_dst_transition is True, then also detect DST offset transition.
        """
        if dt1.utcoffset() != dt2.utcoffset():
            return True
        if self.detect_dst_transition:
            return dt1.dst() != dt2.dst()
        return False

    def _only_dst(self, dt1: datetime, dt2: datetime) -> bool:
        """Determine if dt1 -> dt2 is only a DST transition."""
        if not self.detect_dst_transition:
            return False
        return dt1.utcoffset() == dt2.utcoffset() and dt1.dst() != dt2.dst()

    def _binary_search_transition(
        self,
        tz: Any,
        dt_left: datetime,
        dt_right: datetime,
    ) -> Tuple[datetime, datetime]:
        """Do a binary search to find the exact transition times, to within 1
        second accuracy. The dt_left and dt_right are 22 hours (79200 seconds)
        apart. So the binary search should take a maximum about 17 iterations to
        find the DST transition within one adjacent second.
        """
        dt_left_local = dt_left.astimezone(tz)
        while True:
            delta_seconds = int((dt_right - dt_left) / timedelta(seconds=1))
            delta_seconds //= 2
            if delta_seconds == 0:
                break

            dt_mid = dt_left + timedelta(seconds=delta_seconds)
            mid_dt_local = dt_mid.astimezone(tz)
            if self._is_transition(dt_left_local, mid_dt_local):
                dt_right = dt_mid
            else:
                dt_left = dt_mid
                dt_left_local = mid_dt_local

        return dt_left, dt_right

    @staticmethod
    def _create_test_item(dt: datetime, tag: str) -> TestItem:
        """Create a TestItem from a datetime."""
        unix_seconds = int(dt.timestamp())
        epoch_seconds = (
            unix_seconds
            - TestDataGenerator.seconds_to_ace_time_epoch_from_unix_epoch
        )
        total_offset = int(dt.utcoffset().total_seconds())  # type: ignore
        dst_offset = int(dt.dst().total_seconds())  # type: ignore

        # See https://stackoverflow.com/questions/5946499 for more info on how
        # to extract the abbreviation. dt.tzinfo will never be None because the
        # timezone will always be defined.
        assert dt.tzinfo is not None
        abbrev = dt.tzinfo.tzname(dt)

        return {
            'epoch': epoch_seconds,
            'total_offset': total_offset,
            'dst_offset': dst_offset,
            'y': dt.year,
            'M': dt.month,
            'd': dt.day,
            'h': dt.hour,
            'm': dt.minute,
            's': dt.second,
            'abbrev': abbrev,
            'type': tag,
        }
