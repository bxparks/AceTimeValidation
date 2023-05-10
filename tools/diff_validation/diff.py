#!/usr/bin/env python3
#
# Copyright 2023 Brian T. Park
#
# MIT License

"""
Generate diff report that compares the given validation.txt to the baseline
validation.txt.

Usage:
$ diff.py --observed file.json --expected file.json
"""

from typing import List  # , Dict
import argparse
import sys
import json

from acetimetools.data_types.validation_types import (
    TestItem,
    # TestEntry,
    ValidationData
)


def main() -> None:
    # Configure command line flags.
    parser = argparse.ArgumentParser(
        description='Diff validation data of observed against expected'
    )

    # DST blacklist JSON file.
    parser.add_argument(
        '--observed',
        type=str,
        help='Subject validation data file'
    )

    # Ignore blacklist. Useful for debugging 3rd party timezones which have
    # inconsistencies with AceTime (or Hinnant date).
    parser.add_argument(
        '--expected',
        type=str,
        help='Baseline validation data file',
    )

    # Parse the command line arguments
    args = parser.parse_args()

    print(f'Reading {args.observed}')
    with open(args.observed) as f:
        observed = json.load(f)
    print(f'Reading {args.expected}')
    with open(args.expected) as f:
        expected = json.load(f)

    differ = Differ(observed, expected)
    differ.diff()
    if not differ.valid:
        sys.exit(1)
    print('Done')


class Differ:
    def __init__(self, observed: ValidationData, expected: ValidationData):
        self.valid = True
        self.observed = observed
        self.expected = expected

        self.check_abbrev = observed['has_valid_abbrev'] and \
            expected['has_valid_abbrev']
        if not self.check_abbrev:
            print('Disabling validation for abbrev')

        self.check_dst = observed['has_valid_dst'] and expected['has_valid_dst']
        if not self.check_dst:
            print('Disabling validation for DST offset')

    def diff(self):
        self.diff_header()
        self.diff_zone_names()
        self.diff_test_data()

    def diff_header(self):
        print('Diff header')
        if self.observed['start_year'] != self.expected['start_year']:
            print('start_year different')
            self.valid = False
        if self.observed['until_year'] != self.expected['until_year']:
            print('until_year different')
            self.valid = False
        if self.observed['epoch_year'] != self.expected['epoch_year']:
            print('epoch_year different')
            self.valid = False

    def diff_zone_names(self):
        print('Diff zone_names')
        observed = set(self.observed['test_data'].keys())
        expected = set(self.expected['test_data'].keys())
        if observed != expected:
            self.valid = False
            missing = expected - observed
            if missing:
                print(f'Missing zones compared to expected: {missing}')
            extra = observed - expected
            if extra:
                print(f'Extra zones compared to expected: {extra}')

    def diff_test_data(self):
        print('Diff test_data')
        obs_test_data = self.observed['test_data']
        exp_test_data = self.expected['test_data']
        for zone, obs_entry in obs_test_data.items():
            exp_entry = exp_test_data[zone]

            # transitions
            obs_transitions = obs_entry['transitions']
            exp_transitions = exp_entry['transitions']
            # if len(obs_transitions) != len(exp_transitions):
            #     print(f'ERROR {zone}: num transitions not equal')
            #     self.valid = False
            #     continue
            self.diff_test_items(
                zone, "transitions", obs_transitions, exp_transitions)

            # samples
            obs_samples = obs_entry['samples']
            exp_samples = exp_entry['samples']
            # if len(obs_samples) != len(exp_samples):
            #     print(f'ERROR {zone}: num samples not equal')
            #     self.valid = False
            #     continue
            self.diff_test_items(
                zone, "samples", obs_samples, exp_samples)

    def diff_test_items(
        self,
        zone: str,
        label: str,
        observed: List[TestItem],
        expected: List[TestItem],
    ) -> None:
        # 2 pointers to traverse the observed and expected lists of TestItem
        io = 0
        ie = 0
        while io < len(observed) and ie < len(expected):
            obs = observed[io]
            exp = expected[ie]

            if not self.check_dst and exp['type'] in ['a', 'b']:
                ie += 1
                continue

            # Ignore 'type' specifier until the compare_xxx binaries are all
            # upgraded to support the 'a' and 'b' (silent) transitions, as well
            # as the 'A' and 'B' (normal) transitions.
            #
            # if obs['type'] != exp['type']:
            #     self.valid = False
            #     print(f"ERROR {zone} {label} type: obs[{io}] != exp[{ie}]")

            if obs['epoch'] != exp['epoch']:
                self.valid = False
                print(f"ERROR {zone} {label} 'epoch': obs[{io}] != exp[{ie}]")

            if obs['total_offset'] != exp['total_offset']:
                self.valid = False
                print(f"ERROR {zone} {label} 'total': obs[{io}] != exp[{ie}]")

            if self.check_dst and obs['dst_offset'] != exp['dst_offset']:
                self.valid = False
                print(f"ERROR {zone} {label} 'dst': obs[{io}] != exp[{ie}]")

            if obs['y'] != exp['y']:
                self.valid = False
                print(f"ERROR {zone} {label} 'y': obs[{io}] != exp[{ie}]")

            if obs['M'] != exp['M']:
                self.valid = False
                print(f"ERROR {zone} {label} 'M': obs[{io}] != exp[{ie}]")

            if obs['d'] != exp['d']:
                self.valid = False
                print(f"ERROR {zone} {label} 'd': obs[{io}] != exp[{ie}]")

            if obs['h'] != exp['h']:
                self.valid = False
                print(f"ERROR {zone} {label} 'h': obs[{io}] != exp[{ie}]")

            if obs['m'] != exp['m']:
                self.valid = False
                print(f"ERROR {zone} {label} 'm': obs[{io}] != exp[{ie}]")

            if obs['s'] != exp['s']:
                self.valid = False
                print(f"ERROR {zone} {label} 's': obs[{io}] != exp[{ie}]")

            if self.check_abbrev and obs['abbrev'] != exp['abbrev']:
                self.valid = False
                print(f"ERROR {zone} {label} 'abbrev': obs[{io}] != exp[{ie}]")

            io += 1
            ie += 1


if __name__ == '__main__':
    main()
