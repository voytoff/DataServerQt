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

};

