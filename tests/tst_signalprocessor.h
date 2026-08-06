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
  void test_calculation_line();
  void test_calculation_branching();
  void test_calculation_independent();
  void test_calculation_cycle();
  void test_calculation_selfReference();
  void test_calculation_bigGraph();

};

#endif // TST_SIGNALPROCESSOR_H
