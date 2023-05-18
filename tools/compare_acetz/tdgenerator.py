# Copyright 2019 Brian T. Park
#
# MIT License

"""
Implements the TestDataGenerator to generate the validation test data using
the acetime.timezone.acetz class from the acetimepy library.
"""

from typing import List
from typing import Tuple
from typing import cast
import logging
from datetime import tzinfo, datetime, timezone, timedelta
from dateutil.tz import UTC  # datetime.UTC requires Python 3.11

import acetime.version
from acetime.timezone import ZoneManager
from acetime.timezone import acetz
from acetime.common import to_unix_seconds
from acetime.typing import ZoneInfoMap
from acetime.zonedb.zone_registry import ZONE_AND_LINK_REGISTRY
from acetimetools.data_types.validation_types import (
    TestItem, TestData, ValidationData
)

# The [start, until) time interval used to search for DST transitions,
# and flag that is True if ONLY the DST changed.
TransitionTimes = Tuple[datetime, datetime, bool]


class TestDataGenerator:
    """Generate the validation test data for all  zones specified by the
    'zone_infos'. The Transitions are extracted from the ZoneProcessor and the
    UTC offsets determined by acetz.
    """

    def __init__(
        self,
        start_year: int,
        until_year: int,
        epoch_year: int,
        sampling_interval: int,
        use_internal_transitions: bool = False,
        zone_infos: ZoneInfoMap = ZONE_AND_LINK_REGISTRY,
    ):
        self.start_year = start_year
        self.until_year = until_year
        self.epoch_year = epoch_year
        self.sampling_interval = timedelta(hours=sampling_interval)
        self.use_internal_transitions = use_internal_transitions
        self.zone_infos = zone_infos

        self.zone_manager = ZoneManager(zone_infos)

        self.seconds_to_acetime_epoch_from_unix_epoch = int(
            datetime(epoch_year, 1, 1, tzinfo=timezone.utc).timestamp()
        )

    def get_validation_data(self, zones: List[str]) -> ValidationData:
        test_data = self._create_test_data(zones)
        return {
            'start_year': self.start_year,
            'until_year': self.until_year,
            'epoch_year': self.epoch_year,
            'source': 'acetimepy',
            'version': str(acetime.version.__version__),
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
            tz = self.zone_manager.gettz(zone_name)
            if not tz:
                logging.error(f"Zone '{zone_name}' not found in acetimepy")
                continue

            if self.use_internal_transitions:
                transitions = self._create_transitions_using_internal(tz)
            else:
                transitions = self._create_transitions_for_zone(tz)
            samples = self._create_samples_for_zone(tz)
            if transitions or samples:
                test_data[zone_name] = {
                    "transitions": transitions,
                    "samples": samples,
                }
            i += 1

        return test_data

    def _create_transitions_for_zone(self, tz: tzinfo) -> List[TestItem]:
        """Add DST transitions, using 'A' and 'B' designators"""
        items: List[TestItem] = []
        transitions = self._find_transitions(tz)
        for (left, right, only_dst) in transitions:
            left_item = self._create_test_item(left, 'a' if only_dst else 'A')
            items.append(left_item)

            right_item = self._create_test_item(right, 'b' if only_dst else 'B')
            items.append(right_item)
        return items

    def _find_transitions(self, tz: tzinfo) -> List[TransitionTimes]:
        """Find the DST transition using dateutil by sampling the time period
        from [start_year, until_year].
        (Copied from compare_dateutil/tdgenerator.py).
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
        """Determine if dt1 -> dt2 is a UTC offset transition. Also detect DST
        offset transition. (Copied from compare_dateutil/tdgenerator.py).
        """
        return dt1.utcoffset() != dt2.utcoffset() \
            or dt1.dst() != dt2.dst()

    def _only_dst(self, dt1: datetime, dt2: datetime) -> bool:
        """Determine if dt1 -> dt2 is only a DST transition."""
        return dt1.utcoffset() == dt2.utcoffset() and dt1.dst() != dt2.dst()

    def _binary_search_transition(
        self,
        tz: tzinfo,
        dt_left: datetime,
        dt_right: datetime,
    ) -> Tuple[datetime, datetime]:
        """Do a binary search to find the exact transition times, to within 1
        second accuracy. The dt_left and dt_right are 22 hours (79200 seconds)
        apart. So the binary search should take about 17 iterations to find the
        DST transition within one adjacent second.
        (Copied from compare_dateutil/tdgenerator.py).
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

    def _create_transitions_using_internal(self, tz: tzinfo) -> List[TestItem]:
        """Create list of DST transitions using the internal cache of
        transitions in the tz object. Must take care to eliminate phantom
        transitions (no total UTC offset or DST offset change), which are just
        artifacts of the implementation.
        """
        atz = cast(acetz, tz)
        zone_processor = atz.zp  # maybe use a copy instead?
        items: List[TestItem] = []
        for year in range(self.start_year, self.until_year):
            # Add samples just before and just after the DST transition.
            zone_processor.init_for_year(year)
            for transition in zone_processor.transitions:
                # Skip if the start year of the Transition does not match the
                # year of interest. This may happen since we use generate
                # transitions over a 14-month interval.
                start = transition.start_date_time
                transition_year = start.y
                if transition_year != year:
                    continue

                epoch_seconds = transition.start_epoch_second
                before_seconds = epoch_seconds - 1
                after_dt = datetime.fromtimestamp(
                    to_unix_seconds(epoch_seconds), tz)
                before_dt = datetime.fromtimestamp(
                    to_unix_seconds(before_seconds), tz)

                # Ignore for phantom transitions
                if (
                    before_dt.utcoffset() == after_dt.utcoffset()
                    and before_dt.dst() == after_dt.dst()
                ):
                    continue

                # Add a test data just before the transition
                tag = 'A' if before_dt.utcoffset() != after_dt.utcoffset() \
                    else 'a'
                item = self._create_test_item(before_dt, tag)
                items.append(item)

                # Add a test data at the transition itself (which will
                # normally be shifted forward or backwards).
                tag = 'B' if before_dt.utcoffset() != after_dt.utcoffset() \
                    else 'b'
                item = self._create_test_item(after_dt, tag)
                items.append(item)
        return items

    def _create_samples_for_zone(self, tz: tzinfo) -> List[TestItem]:
        """Create samples for the tz in the years [start_year, until_year).
        One test point for each month, on the *second* of the month,
        annotated by tag='S' on the first attempt, then 'T' on the second
        attempt.

        Using the *second* of each month instead of the first of the month
        prevents Jan 1, 2000 from being converted to a negative epoch seconds
        for certain timezones, which gets converted into a UTC date in 1999 when
        ExtendedZoneProcessor is used to convert the epoch seconds back to a
        ZonedDateTime. The UTC date in 1999 causes the actual max buffer size of
        ExtendedZoneProcessor to become different than the one predicted by
        BufSizeEstimator (which samples whole years from 2000 until 2050), and
        causes the AceTimeValidation/ExtendedZoneInfoTest to fail on the buffer
        size check.
        """
        items: List[TestItem] = []
        for year in range(self.start_year, self.until_year):
            for month in range(1, 13):
                # If 00:00 on the second of the month is not a "simple"
                # datetime, then try subsequent days.
                tag = 'S'
                for day in range(2, 29):
                    dt = datetime(year, month, day, 0, 0, 0, tzinfo=tz)
                    if not _in_gap(dt, tz):
                        item = self._create_test_item(dt, tag)
                        items.append(item)
                        break
                    tag = 'T'
        return items

    def _create_test_item(self, dt: datetime, tag: str) -> TestItem:
        unix_seconds = int(dt.timestamp())
        acetime_seconds = unix_seconds \
            - self.seconds_to_acetime_epoch_from_unix_epoch
        total_offset = int(dt.utcoffset().total_seconds())  # type: ignore
        dst_offset = int(dt.dst().total_seconds())  # type: ignore

        # See https://stackoverflow.com/questions/5946499 for more info on how
        # to extract the abbreviation. dt.tzinfo will never be None because the
        # timezone will always be defined.
        assert dt.tzinfo is not None
        abbrev = dt.tzinfo.tzname(dt)

        return {
            'epoch': acetime_seconds,
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


def _in_gap(dt: datetime, tz: tzinfo) -> bool:
    """Return True if `dt` in a gap. The python datetime API does
    not provide a direct way to detect this, so we have to work a little harder.
    """
    unix_seconds = int(dt.timestamp())
    et = datetime.fromtimestamp(unix_seconds, tz)
    return (
        dt.year != et.year
        or dt.month != et.month
        or dt.day != et.day
        or dt.hour != et.hour
        or dt.minute != et.minute
        or dt.second != et.second
    )
