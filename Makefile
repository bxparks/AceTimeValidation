# Bash shell is required to allow using the `wait` command when invoking the
# parallel build target 'allvalidations'
SHELL=bash

# There are 2 sets of validation test targets in this Makefile.
#
# 1) The validation tests against libraries which can be configured to pass
# deterministically because they allow the TZ version to be specified, instead
# of picking up whatever random version that the underlying OS has installed.
# That means that they can be used in the GitHub CI workflow. These
# deterministic validation tests are:
#
#	* BasicHinnantDateTest
#	* ExtendedHinnantDateTest
#	* BasicAcetzTest
#	* ExtendedAcetzTest
#
# They are invoked using:
#
# $ make validations
# $ make runvalidations
#
# 2) The full validation tests (Basic*Test and Extended*Test). Some of these
# will be broken when a new TZ DB is released because the dependent library has
# not released a new version of the library yet. The full set of validation
# tests are invoked using:
#
# $ make allvalidations # compiles in parallel
# $ make runallvalidations
#
# Sometimes `make allvalidations` fails because it tries to build the targets
# in parallel. In that case, we can use:
#
# $ make allvalidations-serial

# Build the deterministic validation tests. These can be trusted to pass no
# matter which OS version is running in the docker image on GitHub.
validations:
	set -e; \
	for i in *HinnantDateTest/Makefile \
			*AcetzTest/Makefile \
			*AceTimeCTest/Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i); \
	done

# Run the deterministic validation tests. These are used in the GitHub CI
# workflow.
runvalidations:
	set -e; \
	for i in *HinnantDateTest/Makefile \
			*AcetzTest/Makefile \
			*AceTimeCTest/Makefile; do \
		echo '==== Running:' $$(dirname $$i); \
		$$(dirname $$i)/$$(dirname $$i).out; \
	done

# Build *all* validation tests in parallel for reduced waiting time. Sometimes,
# this fails for some race condition which I have not spent time figuring out.
# When that happens, use the 'allvalidations-serial' instead.
allvalidations:
	(set -e; \
	trap 'kill 0' SIGHUP SIGINT SIGQUIT SIGKILL SIGTERM; \
	for i in */Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) & \
	done; \
	wait)

# Same as 'make allvalidations' but in series not in parallel. Useful when
# 'allvalidations` fails due to some sporadic race condition.
allvalidations-serial:
	set -e; \
	for i in */Makefile; do \
		echo '==== Making:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i); \
	done

# Run *all* validation tests.
runallvalidations:
	set -e; \
	for i in */Makefile; do \
		echo '==== Running:' $$(dirname $$i); \
		$$(dirname $$i)/$$(dirname $$i).out; \
	done

# Clean all validation tests.
clean:
	set -e; \
	for i in */Makefile; do \
		echo '==== Cleaning:' $$(dirname $$i); \
		$(MAKE) -C $$(dirname $$i) clean; \
	done
