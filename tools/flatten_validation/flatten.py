#!/usr/bin/env python3
#
# Copyright 2023 Brian T. Park
#
# MIT License

"""
Flatten the `validation_data.json` file on the STDIN to a more compact
TXT file format on the STDOUT. The TXT file can be more than 10X smaller than
the JSON file.

Usage:
$ flatten.py < validation_data.json > validation_data.txt
"""

from typing import List, Dict
import argparse
import logging
import sys
import json

from acetimetools.data_types.validation_types import (
    TestItem,
    TestEntry,
    ValidationData
)


def main() -> None:
    # Configure command line flags.
    parser = argparse.ArgumentParser(
        description='Flatten validation_data.json'
    )

    # DST blacklist JSON file.
    parser.add_argument(
        '--blacklist',
        type=str,
        help='JSON file containing DST blacklist',
    )

    # Ignore blacklist. Useful for debugging 3rd party timezones which have
    # inconsistencies with AceTime (or Hinnant date).
    parser.add_argument(
        '--ignore_blacklist',
        action='store_true',
        help='Ignore the --blacklist flag, useful for debugging',
    )

    # Parse the command line arguments
    args = parser.parse_args()

    # Configure logging. This should normally be executed after the
    # parser.parse_args() because it allows us set the logging.level using a
    # flag.
    logging.basicConfig(level=logging.INFO)

    # Read the JSON on the STDIN
    validation_data = json.load(sys.stdin)

    # Read the DST blacklist file if given.
    if args.blacklist and not args.ignore_blacklist:
        with open(args.blacklist) as f:
            blacklist = json.load(f)
    else:
        blacklist = {}

    # Generate the validation_*.{h, cpp} files
    flattener = Flattener(blacklist=blacklist)
    flattener.flatten(validation_data)


class Flattener:
    """Convert JSON into flat file of the form:
    HEADER
    start_year: {int}
    until_year: {int}
    epoch_year: {int}
    ...

    ZONE {zone}
    TRANSITIONS {len}
    {# epoch offset dst y M d h m s abbrev type}
    ...
    SAMPLES {len}
    {epoch offset dst y M d h m s abbrev type}
    ...

    ZONE {zone}
    ...
    """

    def __init__(self, blacklist: Dict[str, str]):
        self.blacklist = blacklist

    def flatten(self, validation_data: ValidationData):
        self.print_header(validation_data)
        test_data = validation_data['test_data']
        for zone, test_entry in test_data.items():
            self.print_zone_data(zone, test_entry)

    def print_header(self, validation_data: ValidationData) -> None:
        print('HEADER')
        print('start_year', validation_data['start_year'])
        print('until_year', validation_data['until_year'])
        print('epoch_year', validation_data['epoch_year'])
        print('has_valid_abbrev', validation_data['has_valid_abbrev'])
        print('has_valid_dst', validation_data['has_valid_dst'])
        print()

    def print_zone_data(self, zone: str, entry: TestEntry) -> None:
        print(f'ZONE {zone}')

        transitions = entry['transitions']
        print(f'TRANSITIONS {len(transitions)}')
        self.print_test_items(transitions)

        samples = entry['samples']
        print(f'SAMPLES {len(samples)}')
        self.print_test_items(samples)
        print()

    def print_test_items(self, items: List[TestItem]) -> None:
        if len(items):
            print("# line       epoch    utc    dst    y  m  d  h  m  s  \
abbrev type")
        line_number = 0
        for item in items:
            epoch_seconds = item['epoch']
            total_offset = item['total_offset']
            delta_offset = item['dst_offset']
            year = item['y']
            month = item['M']
            day = item['d']
            hour = item['h']
            minute = item['m']
            second = item['s']
            abbrev_value = item['abbrev']
            abbrev = f'{abbrev_value}' if abbrev_value else '-'
            type = item['type']

            print(f"{line_number:>6} \
{epoch_seconds:11} {total_offset:6} {delta_offset:6} \
{year:4} {month:2} {day:2} {hour:2} {minute:2} {second:2} {abbrev:>7} \
{type:>4}")
            line_number += 1


if __name__ == '__main__':
    main()
