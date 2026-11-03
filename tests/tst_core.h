#pragma once

#include <QObject>

class tst_core : public QObject
{
  Q_OBJECT
public:
  tst_core();
  ~tst_core() override;


private slots:
  void test_fakeClock();
  void test_systemClock();
  void test_schedulerClock();

  void test_timeAxis();
  void test_visibleTimeRangeChanged();
  void test_timeRangeController();

};