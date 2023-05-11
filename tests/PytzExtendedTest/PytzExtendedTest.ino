#line 2 "PytzExtendedTest.ino"

/*
 * This unit test depends on 'validation_data.cpp' which is so large that
 * it will likely not compile on an Arduino environment. It can however be run
 * on a Linux or MacOS environment using the provided Makefile.
 */

#include <AUnitVerbose.h>

using aunit::TestRunner;

//---------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // wait to prevent garbage on SERIAL_PORT_MONITOR
#endif
  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Leonardo/Micro
#if defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.setLineModeUnix();
#endif
}

void loop() {
  TestRunner::run();
}
