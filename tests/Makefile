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

# Build the deterministic validation tests. These can be trusted to pass no
# matter which OS version is running in the docker image on GitHub. Validation
# only ExtendedZoneProcessor of AceTime, to reduce the time taken by this
# validation test.
tests:
	set -e; \
	for i in HinnantExtendedTest/Makefile AcetzExtendedTest/Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i); \
	done

# Run the deterministic validation tests. These are used in the GitHub CI
# workflow.
runtests:
	set -e; \
	for i in HinnantExtendedTest/Makefile AcetzExtendedTest/Makefile; do \
		echo '==== Running:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) run; \
	done

# Build *all* validation tests in parallel for reduced waiting time. Sometimes,
# this fails for some race condition which I have not spent time figuring out.
# When that happens, use the 'alltests-serial' instead.
alltests:
	(set -e; \
	trap 'kill 0' SIGHUP SIGINT SIGQUIT SIGKILL SIGTERM; \
	for i in */Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) & \
	done; \
	wait)

# Same as 'make alltests' but in series not in parallel. Useful when
# 'alltests` fails due to some sporadic race condition.
alltests-serial:
	set -e; \
	for i in */Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i); \
	done

# Run *all* validation tests.
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
