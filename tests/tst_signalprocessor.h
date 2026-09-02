#pragma once

#include <QObject>

class tst_signalprocessor : public QObject
{
  Q_OBJECT
public:
  tst_signalprocessor();
  ~tst_signalprocessor() override;

private:

private slots:
  /*
  void test_calculation_plan();
  void test_calculation_line();
  void test_calculation_branching();
  void test_calculation_independent();
  void test_calculation_cycle();
  void test_calculation_selfReference();
  void test_calculation_bigGraph();
  void test_calculation_calculationProcessor();

  void test_formulas_FormulaRepository();
  void test_formulas_CalculationPlan();
  void test_calculationCompiler_failFormula();
  void test_calculationCompiler_unknownSignalDependency();
  void test_calculationProcessor_failingFormula();
  void test_calculationProcessor_emptyCalculationPlan();
  void test_calculationProcessor_formulaContext();
  void test_calculationCompiler_unknownSignalOutput();
*/
  void test_calculationPlan_base();
  void test_calculationPlan_failAst();
  void test_calculationPlan_cycle();
  void test_calculationPlan_rebuildInvalid();

  void test_signalProcessor_calculate();

  void test_signalProcessor_failOnceDataSource();
  void test_signalProcessor_failFormula();
  void test_signalProcessor_cycle();

};
