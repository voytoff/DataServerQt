#ifndef TST_CORE_H
#define TST_CORE_H

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

};

#endif // TST_CORE_H
