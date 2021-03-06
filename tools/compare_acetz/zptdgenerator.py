# Copyright 2019 Brian T. Park
#
# MIT License

"""
Implements the TestDataGenerator to generate the validation test data using
acetz, which uses Python ZoneProcessor class. Pulling in ZoneProcessor also
means that it pulls in the data structures defined by zonedb.
"""

from typing import Dict
from typing import List
from typing import Optional
from typing import cast
import logging
from datetime import tzinfo, datetime, timezone, timedelta

import acetime.version
from acetime.acetz import ZoneManager
from acetime.zone_processor import ZoneProcessor
from acetime.zone_processor import DateTuple
from acetime.zonedb_types import ZoneInfoMap
from acetime.zonedb.zone_registry import ZONE_REGISTRY
from acetimetools.data_types.at_types import SECONDS_SINCE_UNIX_EPOCH
from acetimetools.data_types.validation_types import (
    TestItem, TestData, ValidationData
)


class TestDataGenerator:
    """Generate the validation test data for all  zones specified by the
    'zone_infos'. The Transitions are extracted from the ZoneProcessor and the
    UTC offsets determined by acetz.
    """

    def __init__(
        self,
        start_year: int,
        until_year: int,
        sampling_interval: int,
        zone_infos: ZoneInfoMap = cast(ZoneInfoMap, ZONE_REGISTRY),
    ):
        self.start_year = start_year
        self.until_year = until_year
        self.sampling_interval = timedelta(hours=sampling_interval)
        self.zone_infos = zone_infos
        self.zone_manager = ZoneManager(zone_infos)

    def create_test_data(self, zones: List[str]) -> None:
        test_data: TestData = {}
        for zone_name in zones:
            test_items = self._create_test_data_for_zone(zone_name)
            if test_items:
                test_data[zone_name] = test_items
        self.test_data = test_data

    def get_validation_data(self) -> ValidationData:
        return {
            'start_year': self.start_year,
            'until_year': self.until_year,
            'source': 'acetz',
            'version': str(acetime.version.__version__),
            'tz_version': 'unknown',
            'has_valid_abbrev': True,
            'has_valid_dst': True,
            'test_data': self.test_data,
        }

    def _create_test_data_for_zone(
        self,
        zone_name: str,
    ) -> Optional[List[TestItem]]:
        """Create the TestItems for a specific zone.
        """
        logging.info(f"_create_test_items(): {zone_name}")
        zone_info = self.zone_infos.get(zone_name)
        if not zone_info:
            logging.error(f"Zone '{zone_name}' not found in acetz package")
            return None

        tz = self.zone_manager.gettz(zone_name)
        zone_processor = ZoneProcessor(zone_info)
        return self._create_transition_test_items(
            zone_name, tz, zone_processor)

    def _create_transition_test_items(
        self,
        zone_name: str,
        tz: tzinfo,
        zone_processor: ZoneProcessor
    ) -> List[TestItem]:
        """Create a TestItem for the tz for each zone, for each year from
        start_year to until_year, exclusive. The 'zone_processor' object is used
        as a shortcut to generate the list of transitions, so it needs to be a
        different object than the zone processor embedded inside the 'tz'
        object.

        The following test samples are created:

        * One test point for each month, on the first of the month.
        * One test point for Dec 31, 23:00 for each year.
        * A test point at the transition from DST to Standard, or vise versa.
        * A test point one second before the transition.

        Each TestData is annotated as:
        * 'A', 'a': pre-transition
        * 'B', 'b': post-transition
        * 'S': a monthly test sample
        * 'Y': end of year test sample

        For [2000, 2038], this generates about 100,000 data points.
        """
        items_map: Dict[int, TestItem] = {}
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

                # Skip if the UTC year bleed under or over the boundaries.
                if transition.transition_time_u.y < self.start_year:
                    continue
                if transition.transition_time_u.y >= self.until_year:
                    continue

                epoch_seconds = transition.start_epoch_second

                # Add a test data just before the transition
                test_item = self._create_test_item_from_epoch_seconds(
                    tz, epoch_seconds - 1, 'A')
                self._add_test_item(items_map, test_item)

                # Add a test data at the transition itself (which will
                # normally be shifted forward or backwards).
                test_item = self._create_test_item_from_epoch_seconds(
                    tz, epoch_seconds, 'B')
                self._add_test_item(items_map, test_item)

            # Add a sample test point on the *second* of each month instead of
            # the first of the month. This prevents Jan 1, 2000 from being
            # converted to a negative epoch seconds for certain timezones, which
            # gets converted into a UTC date in 1999 when ExtendedZoneProcessor
            # is used to convert the epoch seconds back to a ZonedDateTime. The
            # UTC date in 1999 causes the actual max buffer size of
            # ExtendedZoneProcessor to become different than the one predicted
            # by BufSizeEstimator (which samples whole years from 2000 until
            # 2050), and can cause the AceTimeValidation/ExtendedAcetzTest to
            # fail on the buffer size check.
            for month in range(1, 13):
                tt = DateTuple(y=year, M=month, d=2, ss=0, f='w')
                test_item = self._create_test_item_from_datetime(
                    tz, tt, type='S')
                self._add_test_item(items_map, test_item)

            # Add a sample test point at the end of the year.
            tt = DateTuple(y=year, M=12, d=31, ss=23 * 3600, f='w')
            test_item = self._create_test_item_from_datetime(
                tz, tt, type='Y')
            self._add_test_item(items_map, test_item)

        # Return the TestItems ordered by epoch
        return [items_map[x] for x in sorted(items_map)]

    def _create_test_item_from_datetime(
        self,
        tz: tzinfo,
        tt: DateTuple,
        type: str,
    ) -> TestItem:
        """Create a TestItem for the given DateTuple in the local time zone.
        """
        # TODO(bpark): It is not clear that this produces the desired
        # datetime for the given tzinfo if tz is an acetz. But I hope it
        # gives a datetime that's roughtly around that time, which is good
        # enough for unit testing.
        dt = datetime(tt.y, tt.M, tt.d, tt.ss // 3600, tzinfo=tz)
        unix_seconds = int(dt.timestamp())
        epoch_seconds = unix_seconds - SECONDS_SINCE_UNIX_EPOCH
        return self._create_test_item_from_epoch_seconds(
            tz, epoch_seconds, type)

    def _create_test_item_from_epoch_seconds(
        self,
        tz: tzinfo,
        epoch_seconds: int,
        type: str,
    ) -> TestItem:
        """Determine the expected date and time components for the given
        'epoch_seconds' for the given 'tz'. The 'epoch_seconds' is the
        transition time calculated by the ZoneProcessor class.

        Return the TestItem with the following fields:
            epoch: epoch seconds from AceTime epoch (2000-01-01T00:00:00Z)
            total_offset: the expected total UTC offset at epoch_seconds
            dst_offset: the expected DST offset at epoch_seconds
            y, M, d, h, m, s: expected date&time components at epoch_seconds
            type: 'a', 'b', 'A', 'B', 'S', 'Y'
        """

        # Convert AceTime epoch_seconds to Unix epoch_seconds.
        unix_seconds = epoch_seconds + SECONDS_SINCE_UNIX_EPOCH

        # Get the transition time, then feed that into acetz to get the
        # total offset and DST shift
        utc_dt = datetime.fromtimestamp(unix_seconds, tz=timezone.utc)
        dt = utc_dt.astimezone(tz)
        total_offset = int(dt.utcoffset().total_seconds())  # type: ignore
        dst_offset = int(dt.dst().total_seconds())  # type: ignore
        assert dt.tzinfo
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
            'type': type,
        }

    @staticmethod
    def _add_test_item(items_map: Dict[int, TestItem], item: TestItem) -> None:
        current = items_map.get(item['epoch'])
        if current:
            # If a duplicate TestItem exists for epoch, then check that the
            # data is exactly the same.
            if (
                current['total_offset'] != item['total_offset']
                or current['dst_offset'] != item['dst_offset']
                or current['y'] != item['y'] or current['M'] != item['M']
                or current['d'] != item['d'] or current['h'] != item['h']
                or current['m'] != item['m'] or current['s'] != item['s']
            ):
                raise Exception(f'Item {current} does not match item {item}')
            # 'A' and 'B' takes precedence over 'S' or 'Y'
            if item['type'] in ['A', 'B']:
                items_map[item['epoch']] = item
        else:
            items_map[item['epoch']] = item
