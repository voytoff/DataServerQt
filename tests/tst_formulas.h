#pragma once

#include <QObject>

class tst_formulas : public QObject
{
  Q_OBJECT
public:
  tst_formulas();
  ~tst_formulas() override;

private slots:
  void test_formuls_lexer_base();
  void test_formulas_lexer_numbers();
  void test_formulas_lexer_identifiers();
  void test_formulas_lexer_invalid();
  void test_formulas_lexer_operators();

  void test_formulas_parser_number();
  void test_formulas_parser_identifier();
  void test_formulas_parser_add();
  void test_formulas_parser_tree_base();

  void test_formulas_parser_parentheses();
  void test_formulas_parser_unaryMinus();
  void test_formulas_parser_unaryMinusWithIdentifer();
  void test_formulas_parser_unaryMinusWithOperator();

  void test_formulas_parser_errorPluseEnds();
  void test_formulas_parser_errorMultiplyEnds();
  void test_formulas_parser_errorOperatorEnds();
  void test_formulas_parser_errorBktEnds();
  void test_formulas_parser_errorEqualsEnds();
  void test_formulas_parser_errorOperatorEqualsEnds();

  void test_formulas_parser_operatorPrecedence();
  void test_formulas_parser_leftAssociative();

  void test_formulas_parser_functionCall();
  void test_formulas_parser_nestedFunctionCall();
  void test_formulas_parser_functionCall_multipleArguments();

  void test_formulas_parser_functionCall_errorBktEnds();
  void test_formulas_parser_functionCall_errorArgumentEnds();
  void test_formulas_parser_functionCall_errorComaArgumentsEnds();
  void test_formulas_parser_functionCall_errorComaArgumentsStart();
  void test_formulas_parser_functionCall_errorMissingComaArguments();
  void test_formulas_parser_functionCall_errorDuplicateBkts();

  void test_formulas_parser_functionCall_SomeArguments();
  void test_formulas_parser_nestedFunctionCallFuncArguments();
  void test_formulas_parser_functionCallAndArguments();

  void test_formulas_identifierResolver();
  void test_formulas_identifierResolver_unknown();
  void test_formulas_identifierResolverCalculated();

  void test_formulas_evaluator_number();
  void test_formulas_evaluator_raw0();
  void test_formulas_evaluator_raw0AndRaw1();
  void test_formulaEvaluator_base();
  void test_formulas_evaluator_divideByZero();
  void test_formulas_evaluator_failSignalReference();
  void test_formulas_evaluator_subtract();
  void test_formulas_evaluator_multiply();
  void test_formulas_evaluator_negate();

  void test_formulas_evaluator_formulaFunctionRepository();
  void test_formulas_formulaFunctionRepository_uniqueNames();
  void test_formulas_formulaFunctionRepository_find();
  void test_formulas_formulaFunctionRepository_clear();

  void test_formulas_functionCall_unknown();
  void test_formulas_functionCall_missingParams();
  void test_formulas_functionCall_failCountParams();
  void test_formulas_functionCall_failValueParams();
  void test_formulas_functionRepository_invalidAdd();

  void test_formulas_functionCall_sqrtSignal();

  void test_formulas_functionRepository_abs_negate();
  void test_formulas_functionRepository_max();
  void test_formulas_functionRepository_min();

  void test_formulas_functionRepository_max_missingParams();
  void test_formulas_functionRepository_max_failParams();
  void test_formulas_functionRepository_min_missingParams();
  void test_formulas_functionRepository_min_failParams();

  void test_formulas_functionRepository_nesteFunctions();

  void test_calculationOrder_base();
  void test_calculationOrder_cba();

  void test_calculationOrder_cycle();
  void test_calculationOrder_selfReference();

  void test_calculationOrder_chain();
  void test_calculationOrder_serious();

  void test_formulaCalculator_base();

  void test_formulaCalculator_calculate();

  /*
  void test_formulaCalculator_initialize_success();
  void test_formulaCalculator_initialize_missingAst();
  void test_formulaCalculator_initialize_unknownSignalAst();
  void test_formulaCalculator_initialize_cycle();
  void test_formulaCalculator_initialize_reinitializeError();
  void test_formulaCalculator_initialize_reinitializeSuccess();

  void test_formulaCalculator_calculateRepeatedly();
  void test_formulaCalculator_calculate_unknownSignalAst();
  void test_formulaCalculator_calculate_divideByZero();

  void test_formulaCalculator_calculate_partialResult();

  void test_calculationPlan_base();
  void test_calculationPlan_clear();
*/
};

