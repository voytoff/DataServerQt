#include <qtestcase.h>
#include "tst_scheduler.h"
#include "buffermanager.h"
#include "emptysignalprocessor.h"
#include "fakedatasource.h"
#include "scheduler.h"
#include "schedulerclock.h"
#include "systemclock.h"
#include "testsrv.h"

tst_scheduler::tst_scheduler() { }
tst_scheduler::~tst_scheduler() = default;

void tst_scheduler::test_scheduler_base()
{
  using namespace qds;

  SystemConfiguration cfg = createTestConfig00();
  SignalMemoryLayout layout;
  layout.build(cfg);

  BufferManager buffers;
  buffers.initialize(layout);

  EmptySignalProcessor processor;
  SystemClock iclock;
  SchedulerClock clock(iclock);
  Scheduler scheduler(buffers, processor, clock);

  FakeDataSource fds;

  scheduler.addDataSource(fds);

  QVERIFY(scheduler.tick());

  QVERIFY(buffers.ready());

  auto &frame = buffers.readFrame();
  QCOMPARE(frame.raw.value(0), 123.);

  QCOMPARE(frame.number.value, 1);
  //QCOMPARE(frame.timestamp.value, 1);

  QCOMPARE(frame.calculated.value(0), 0.0);
  QCOMPARE(frame.calculated.value(1), 0.0);

  QVERIFY(scheduler.tick());

  auto& frame2 = buffers.readFrame();

  QCOMPARE(frame2.number.value, 2);
  //QCOMPARE(frame2.timestamp.value, 2);
  QCOMPARE(frame2.raw.value(0), 123.);
}
