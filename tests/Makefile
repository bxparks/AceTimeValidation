# Bash shell is required to allow using the `wait` command when invoking the
# parallel build target 'alltests'
SHELL=bash

# There are 2 sets of validation test targets in this Makefile.
#
# 1) The validation tests against libraries which can be configured to pass
# deterministically because they allow the TZ version to be specified, instead
# of picking up whatever random version that the underlying OS has installed.
# That means that they can be used in the GitHub CI workflow. These
# deterministic validation tests are:
#
#	* HinnantExtendedDateTest
#	* AcetzExtendedTest
#
# They are invoked using:
#
# $ make tests
# $ make runtests
#
# 2) The full validation tests (Basic*Test and Extended*Test). Some of these
# will be broken when a new TZ DB is released because the dependent library has
# not released a new version of the library yet. The full set of validation
# tests are invoked using:
#
# $ make alltests # compiles in parallel
# $ make runalltests
#
# Sometimes `make alltests` fails because it tries to build the targets
# in parallel. In that case, we can use:
#
# $ make alltests-serial

# Build the deterministic validation tests which can be trusted to always pass
# regardless of the OS version of the docker image on GitHub. Validate only
# ExtendedZoneProcessor of AceTime, to reduce the run time. Use something like
# 'make -j2' to run in parallel.
tests: \
HinnantExtendedTest/all \
AcetzExtendedTest/all \
AcetimecExtendedTest/all

HinnantExtendedTest/all:
	$(MAKE) -C HinnantExtendedTest

AcetzExtendedTest/all:
	$(MAKE) -C AcetzExtendedTest

AcetimecExtendedTest/all:
	$(MAKE) -C AcetimecExtendedTest

# Run the deterministic validation tests. These are used in the GitHub CI
# workflow.
runtests:
	$(MAKE) -C HinnantExtendedTest run
	$(MAKE) -C AcetzExtendedTest run
	$(MAKE) -C AcetimecExtendedTest run

# Build all validation tests.
alltests-serial:
	set -e; \
	for i in */Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i); \
	done

# Run all validation tests.
runalltests:
	set -e; \
	for i in */Makefile; do \
		echo '==== Running:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) run; \
	done

# Clean all validation tests.
clean:
	set -e; \
	for i in */Makefile; do \
		echo '==== Cleaning:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) clean; \
	done
