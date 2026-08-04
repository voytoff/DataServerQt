#include <qtestcase.h>
#include <qtestsupport_core.h>
#include "tst_core.h"
#include "fakeclock.h"
#include "schedulerclock.h"
#include "systemclock.h"

tst_core::tst_core() { }
tst_core::~tst_core() = default;

void tst_core::test_fakeClock()
{
  using namespace qds;

  FakeClock clock;

  QCOMPARE(clock.timestamp().value, 0ull);

  clock.setTimestamp(1000);
  QCOMPARE(clock.timestamp().value, 1000ull);

  clock.advance(250);
  QCOMPARE(clock.timestamp().value, 1250ull);
}

void tst_core::test_systemClock()
{
  using namespace qds;

  SystemClock clock;

  auto t1 = clock.timestamp();

  QTest::qWait(20);

  auto t2 = clock.timestamp();

  QVERIFY(t2.value >= t1.value);
}

void tst_core::test_schedulerClock()
{
  using namespace qds;

  FakeClock fake;

  fake.setTimestamp(1000);
  fake.setWallClockTime(1754300000000000);


  SchedulerClock clock(fake);


  clock.nextTick();


  QCOMPARE(clock.frameNumber().value, 1);

  QCOMPARE(
    clock.timestamp().value,
    1000);

  QCOMPARE(
    clock.wallClockTime().unixMicroseconds,
    1754300000000000);


  fake.advance(100);


  clock.nextTick();


  QCOMPARE(clock.frameNumber().value, 2);
  QCOMPARE(clock.timestamp().value, 1100);
}