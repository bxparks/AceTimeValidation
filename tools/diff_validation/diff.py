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

from acetimetools.datatypes.valtyping import (
    TestItem,
    TestEntry,
    ValidationData
)


class DiffFailed(Exception):
    pass


def main() -> None:
    # Configure command line flags.
    parser = argparse.ArgumentParser(
        description='Diff validation data of observed against expected'
    )

    parser.add_argument(
        '--observed',
        type=str,
        help='Subject validation data JSON file'
    )

    parser.add_argument(
        '--expected',
        type=str,
        help='Baseline validation data JSON file',
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
    print('Done')
    if not differ.valid:
        sys.exit(1)


class Differ:
    def __init__(self, observed: ValidationData, expected: ValidationData):
        self.valid = True
        self.observed = observed
        self.expected = expected

        if self.expected['scope'] != 'complete':
            self.fail("expected['scope'] must be 'complete'")

        self.is_subset = (self.observed['scope'] != 'complete')
        if self.is_subset:
            print('Observed is a subset of Expected')

        self.check_abbrev = observed['has_valid_abbrev'] and \
            expected['has_valid_abbrev']
        if not self.check_abbrev:
            print('Disabling validation for abbrev')

        self.check_dst = observed['has_valid_dst'] and expected['has_valid_dst']
        if not self.check_dst:
            print('Disabling validation for DST offset')

        self.start_year = observed['start_year']
        self.until_year = observed['until_year']
        self.offset_granularity = observed['offset_granularity']

    def fail(self, s: str) -> None:
        self.valid = False
        print(s)
        raise DiffFailed()

    def diff(self):
        try:
            self.diff_header()
            self.diff_zone_names()
            self.diff_test_data()
        except DiffFailed:
            pass

    def diff_header(self):
        print('Diff header')

        if self.is_subset:
            if self.observed['start_year'] < self.expected['start_year']:
                self.fail("observed[start_year] < expected[start_year]")
            if self.observed['until_year'] > self.expected['until_year']:
                self.fail("observed[until_year] > expected[until_year]")
        else:
            if self.observed['start_year'] != self.expected['start_year']:
                self.fail("start_year different")
            if self.observed['until_year'] != self.expected['until_year']:
                self.fail('until_year different')
        if self.observed['epoch_year'] != self.expected['epoch_year']:
            self.fail('epoch_year different')

    def diff_zone_names(self):
        print('Diff zone_names')
        observed = set(self.observed['test_data'].keys())
        expected = set(self.expected['test_data'].keys())

        if not self.is_subset:
            missing = expected - observed
            if missing:
                self.fail(f'Missing zones compared to expected: {missing}')

        extra = observed - expected
        if extra:
            self.fail(f'Extra zones compared to expected: {extra}')

    def diff_test_data(self):
        print('Diff test_data')
        obs_test_data = self.observed['test_data']
        exp_test_data = self.expected['test_data']

        skipped_list: List[str] = []
        failed_list: List[str] = []
        # Loop over observed zones, and validate against the expected zones.
        for zone, obs_entry in obs_test_data.items():
            exp_entry = exp_test_data[zone]
            try:
                skipped = self.diff_zone_entry(zone, obs_entry, exp_entry)
                if skipped:
                    skipped_list.append(zone)
            except DiffFailed:
                failed_list.append(zone)

        if skipped_list:
            print(f'Skipped {len(skipped_list)} zones')
        if failed_list:
            print(f'Failed {len(failed_list)} zones')

    def diff_zone_entry(
        self,
        zone: str,
        obs_entry: TestEntry,
        exp_entry: TestEntry,
    ) -> bool:
        """Return True if zone is skipped."""

        # If transitions and samples are empty in observed, this indicates
        # that the zone is not supported by the observed. Note this, but
        # don't terminate.
        obs_transitions = obs_entry['transitions']
        obs_samples = obs_entry['samples']
        if len(obs_transitions) == 0 and len(obs_samples) == 0:
            return True

        # transitions
        exp_transitions = exp_entry['transitions']
        # if len(obs_transitions) != len(exp_transitions):
        #     self.fail(f'ERROR {zone}: num transitions not equal')
        #     continue
        self.diff_test_items(
            zone, "transitions", obs_transitions, exp_transitions)

        # samples
        exp_samples = exp_entry['samples']
        # if len(obs_samples) != len(exp_samples):
        #     self.fail(f'ERROR {zone}: num samples not equal')
        #     continue
        self.diff_test_items(
            zone, "samples", obs_samples, exp_samples)

        return False

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

        # Synchronize the year if observed is a subset of expected
        if self.is_subset and len(observed) != 0:
            obs = observed[io]
            year = obs['y']
            while ie < len(expected):
                exp = expected[ie]
                if exp['y'] >= year:
                    break
                ie += 1

        # Validate each observed TestItem against the expected TestItem.
        while io < len(observed) and ie < len(expected):
            obs = observed[io]
            exp = expected[ie]

            # Skip silent transitions if not supported by observed dataset.
            if not self.check_dst:
                if exp['type'] in ['a', 'b']:
                    ie += 1
                    continue
                if obs['type'] in ['a', 'b']:
                    io += 1
                    continue

            # Ignore 'type' specifier until the compare_xxx binaries are all
            # upgraded to support the 'a' and 'b' (silent) transitions, as well
            # as the 'A' and 'B' (normal) transitions.
            #
            # if obs['type'] != exp['type']:
            #     self.fail(
            #         f"ERROR {zone} {label} type: obs[{io}] != exp[{ie}]")

            if obs['epoch'] != exp['epoch']:
                self.fail(
                    f"ERROR {zone} {label} 'epoch': obs[{io}] != exp[{ie}]")

            obs_offset = truncate(obs['total_offset'], self.offset_granularity)
            exp_offset = truncate(exp['total_offset'], self.offset_granularity)
            if obs_offset != exp_offset:
                self.fail(
                    f"ERROR {zone} {label} 'total': obs[{io}] != exp[{ie}]")

            if self.check_dst and obs['dst_offset'] != exp['dst_offset']:
                self.fail(f"ERROR {zone} {label} 'dst': obs[{io}] != exp[{ie}]")

            if obs['y'] != exp['y']:
                self.fail(f"ERROR {zone} {label} 'y': obs[{io}] != exp[{ie}]")

            if obs['M'] != exp['M']:
                self.fail(f"ERROR {zone} {label} 'M': obs[{io}] != exp[{ie}]")

            if obs['d'] != exp['d']:
                self.fail(f"ERROR {zone} {label} 'd': obs[{io}] != exp[{ie}]")

            if obs['h'] != exp['h']:
                self.fail(f"ERROR {zone} {label} 'h': obs[{io}] != exp[{ie}]")

            if obs['m'] != exp['m']:
                self.fail(f"ERROR {zone} {label} 'm': obs[{io}] != exp[{ie}]")

            if obs['s'] != exp['s']:
                self.fail(f"ERROR {zone} {label} 's': obs[{io}] != exp[{ie}]")

            if self.check_abbrev and obs['abbrev'] != exp['abbrev']:
                self.fail(
                    f"ERROR {zone} {label} 'abbrev': obs[{io}] != exp[{ie}]")

            io += 1
            ie += 1

        # Verify number of test items ignoring silent transitions of type 'a'
        # and 'b'.
        len_observed = len([
            item
            for item in observed
            if item['type'] in ['A', 'B', 'S', 'T']
        ])
        len_expected = len([
            item
            for item in expected
            if (
                item['type'] in ['A', 'B', 'S', 'T']
                and item['y'] >= self.start_year
                and item['y'] < self.until_year
            )
        ])
        if self.is_subset:
            if len_observed > len_expected:
                self.fail(
                    f"ERROR {zone} {label}: len(observed) ({len_observed}) > "
                    f"len(expected) ({len_expected})")
        else:
            if len_observed != len_expected:
                self.fail(
                    f"ERROR {zone} {label}: len(observed) ({len_observed}) != "
                    f"len(expected) ({len_expected})")


def div_to_zero(a: int, b: int) -> int:
    """Integer division (a/b) that truncates towards 0, instead of -infinity as
    is default for Python. Assumes b is positive, but a can be negative or
    positive.
    """
    return a // b if a >= 0 else (a - 1) // b + 1


def truncate(a: int, b: int) -> int:
    """Truncate a to multiples of b, rounding down towards 0."""
    return div_to_zero(a, b) * b


if __name__ == '__main__':
    main()
