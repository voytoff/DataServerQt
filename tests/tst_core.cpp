#include "tst_core.h"
#include "fakeclock.h"
#include <qtestcase.h>
#include <qtestsupport_core.h>
#include "systemclock.h"

tst_core::tst_core() { }
tst_core::~tst_core() = default;

void tst_core::test_fakeClock()
{
  using namespace qds;

  FakeClock clock;

  QCOMPARE(clock.now(), 0ull);

  clock.set(1000);
  QCOMPARE(clock.now(), 1000ull);

  clock.advance(250);
  QCOMPARE(clock.now(), 1250ull);
}

void tst_core::test_systemClock()
{
  using namespace qds;

  SystemClock clock;

  auto t1 = clock.now();

  QTest::qWait(20);

  auto t2 = clock.now();

  QVERIFY(t2 > t1);
}
