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

};

