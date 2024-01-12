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
AcetimecBasicTest/all \
AcetimecExtendedTest/all \
AcetzBasicTest/all \
AcetzExtendedTest/all \
HinnantBasicTest/all \
HinnantExtendedTest/all

AcetimecBasicTest/all:
	$(MAKE) -C AcetimecBasicTest

AcetimecExtendedTest/all:
	$(MAKE) -C AcetimecExtendedTest

AcetzBasicTest/all:
	$(MAKE) -C AcetzBasicTest

AcetzExtendedTest/all:
	$(MAKE) -C AcetzExtendedTest

HinnantBasicTest/all:
	$(MAKE) -C HinnantBasicTest

HinnantExtendedTest/all:
	$(MAKE) -C HinnantExtendedTest

# Run the deterministic validation tests. These are used in the GitHub CI
# workflow.
runtests:
	$(MAKE) -C AcetimecBasicTest run
	$(MAKE) -C AcetimecExtendedTest run
	$(MAKE) -C AcetzBasicTest run
	$(MAKE) -C AcetzExtendedTest run
	$(MAKE) -C HinnantBasicTest run
	$(MAKE) -C HinnantExtendedTest run

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
