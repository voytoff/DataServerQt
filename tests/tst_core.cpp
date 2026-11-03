#include <QCoreApplication>
#include <qsignalspy.h>
#include <qtestcase.h>
#include <qtestsupport_core.h>
#include "tst_core.h"
#include "fakeclock.h"
#include "schedulerclock.h"
#include "systemclock.h"
#include "timeaxis.h"
#include "timechartview.h"
#include "timerangecontroller.h"


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

void tst_core::test_timeAxis()
{
  using namespace qds;

  TimeAxis axis;

  axis.setOriginTimestamp(Timestamp{1'000'000});
  axis.setZeroTimestamp(Timestamp{1'000'000});

  const Timestamp point{6'000'000};

  QCOMPARE(axis.chartSeconds(point), 5.0);
  QCOMPARE(axis.relativeSeconds(point), 5.0);

  axis.setZeroTimestamp(Timestamp{8'000'000});

  QCOMPARE(axis.chartSeconds(point), 5.0);
  QCOMPARE(axis.relativeSeconds(point), -2.0);

  axis.setMode(TimeAxisMode::LocalTime);

  QCOMPARE(axis.chartSeconds(point), 5.0);

  // Проверка форматирования относительного времени.
  axis.setMode(TimeAxisMode::RelativeSeconds);

  QCOMPARE(
    axis.format(point, WallClockTime{}),
    QString("-2.000"));

  // Проверка точности для больших Timestamp.
  axis.setOriginTimestamp(
    Timestamp{10'000'000'000'000'000ULL});

  QCOMPARE(
    axis.chartSeconds(
      Timestamp{10'000'000'000'000'001ULL}),
    0.000001);
}

void tst_core::test_visibleTimeRangeChanged()
{
  using namespace qds;
/*
  int arg = 0;
  QCoreApplication app(arg, nullptr);

  TimeChartView view;
  qds::TimeAxis axis;

  view.setTimeAxis(&axis);

  QSignalSpy spy(
    &view,
    &TimeChartView::visibleTimeRangeChanged);

  view.setVisibleTimeRange(2.0, 8.0);

  QCOMPARE(spy.count(), 1);

  QCOMPARE(view.visibleTimeRange().first, 2.0);
  QCOMPARE(view.visibleTimeRange().second, 8.0);

  axis.setZeroTimestamp(Timestamp{5'000'000});

  QCOMPARE(spy.count(), 1);
*/
}

void tst_core::test_timeRangeController()
{
  using namespace qds;

  TimeRangeController controller;

  QCOMPARE(controller.minimum(), 0.0);
  QCOMPARE(controller.maximum(), 10.0);

  QSignalSpy spy(
    &controller,
    &TimeRangeController::rangeChanged);

  controller.setRange(2.0, 8.0);

  QCOMPARE(controller.minimum(), 2.0);
  QCOMPARE(controller.maximum(), 8.0);
  QCOMPARE(spy.count(), 1);

  // Повторная установка того же диапазона.
  controller.setRange(2.0, 8.0);
  QCOMPARE(spy.count(), 1);

  // Некорректный диапазон.
  controller.setRange(10.0, 5.0);
  QCOMPARE(spy.count(), 1);

  // Нулевой диапазон.
  controller.setRange(5.0, 5.0);
  QCOMPARE(spy.count(), 1);

  // Проверка бесконечных значений.
  controller.setRange(
    0.0,
    std::numeric_limits<double>::infinity());

  QCOMPARE(spy.count(), 1);

  // Корректное изменение.
  controller.setRange(-5.0, 15.0);

  QCOMPARE(controller.minimum(), -5.0);
  QCOMPARE(controller.maximum(), 15.0);
  QCOMPARE(spy.count(), 2);
}