#!/usr/bin/env python3
#
# Copyright 2020 Brian T. Park
#
# MIT License

"""
Generate the JSON validation test data on the STDOUT from the pytz (using
.tdgenerator.TestDataGenerator) given the 'zones.txt' file from the
tzcompiler.py on the STDIN.

Usage
$ ./generate_data.py [--start_year start] [--until_year until]
    [--sampling_interval hours]
    < zones.txt
    > validation_data.json
"""

import sys
import logging
import json
from argparse import ArgumentParser
from typing import List

# Can't use relative import (.tdgenerator) here because PEP 3122 got rejected
# https://mail.python.org/pipermail/python-3000/2007-April/006793.html.
from tdgenerator import TestDataGenerator  # noqa


class Generator:
    def __init__(
        self,
        invocation: str,
        start_year: int,
        until_year: int,
        sampling_interval: int,
    ):
        self.invocation = invocation
        self.start_year = start_year
        self.until_year = until_year
        self.sampling_interval = sampling_interval

    def generate(self) -> None:
        """Generate the validation_data JSON."""

        # Read the zones from the STDIN
        zones = read_zones()

        # Generate the test data set.
        test_generator = TestDataGenerator(
            start_year=self.start_year,
            until_year=self.until_year,
            sampling_interval=self.sampling_interval,
        )
        test_generator.create_test_data(zones)
        validation_data = test_generator.get_validation_data()

        # Write out the validation_data.json file.
        json.dump(validation_data, sys.stdout, indent=2)
        print()  # add terminating newline


def read_zones() -> List[str]:
    """Read the list of zone_names from the sys.stdin."""
    zones: List[str] = []
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        if line.startswith('#'):
            continue
        zones.append(line)
    return zones


def main() -> None:
    parser = ArgumentParser(description='Generate Test Data.')

    parser.add_argument(
        '--start_year',
        help='Start year of validation (default: start_year)',
        type=int,
        default=2000)
    parser.add_argument(
        '--until_year',
        help='Until year of validation (default: 2038)',
        type=int,
        default=2038)
    parser.add_argument(
        '--sampling_interval',
        type=int,
        default=22,
        help='Sampling interval in hours (default 22)',
    )

    args = parser.parse_args()

    # Configure logging
    logging.basicConfig(level=logging.INFO)

    invocation = ' '.join(sys.argv)

    generator = Generator(
        invocation=invocation,
        start_year=args.start_year,
        until_year=args.until_year,
        sampling_interval=args.sampling_interval,
    )
    generator.generate()


if __name__ == '__main__':
    main()
