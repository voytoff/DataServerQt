#ifndef TST_SIGNALPROCESSOR_H
#define TST_SIGNALPROCESSOR_H

#include <QObject>

class tst_signalprocessor : public QObject
{
  Q_OBJECT
public:
  tst_signalprocessor();
  ~tst_signalprocessor() override;

private slots:
  void test_calculation_plan();
  void test_calculation_order();

};

#endif // TST_SIGNALPROCESSOR_H
