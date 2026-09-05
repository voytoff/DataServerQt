#include "tst_hardware.h"
#include "fakelcardmodule.h"
#include "signalmemory.h"
#include <qtestcase.h>

tst_hardware::tst_hardware() { }
tst_hardware::~tst_hardware() = default;

void tst_hardware::test_fakeLCardModule_base()
{
  using namespace qds;

  FakeLCardModule module;

  RawMemory raw;
  raw.initialize(3);

  QVERIFY(!module.read(raw.values()));

  QCOMPARE(module.readCalls, 0);

  QCOMPARE(raw.values()[0], 0.0);
  QCOMPARE(raw.values()[1], 0.0);
  QCOMPARE(raw.values()[2], 0.0);

  QVERIFY(module.start());
  QVERIFY(!module.start());

  QVERIFY(module.read(raw.values()));
  QCOMPARE(module.readCalls, 1);

  QCOMPARE(raw.values()[0], 0.0);
  QCOMPARE(raw.values()[1], 1.0);
  QCOMPARE(raw.values()[2], 2.0);

  QVERIFY(module.read(raw.values()));
  QCOMPARE(module.readCalls, 2);

  QCOMPARE(raw.values()[0], 3.0);
  QCOMPARE(raw.values()[1], 4.0);
  QCOMPARE(raw.values()[2], 5.0);

  module.stop();
  QCOMPARE(module.stopCalls, 1);

  QVERIFY(!module.read(raw.values()));
  QCOMPARE(module.readCalls, 2);

  QCOMPARE(raw.values()[0], 3.0);
  QCOMPARE(raw.values()[1], 4.0);
  QCOMPARE(raw.values()[2], 5.0);
}

