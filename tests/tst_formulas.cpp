#include "tst_formulas.h"
#include "parser/formulalexer.h"
#include <QtTest/qtestcase.h>

tst_formulas::tst_formulas() { }
tst_formulas::~tst_formulas() = default;

void tst_formulas::test_formuls_lexer_base()
{
  using namespace qds;

  FormulaLexer lexer("Raw0 + Raw1 * 2");

  std::vector<FormulaToken> tokens;

  for (;;)
  {
    FormulaToken token = lexer.next();

    if (token.type == FormulaTokenType::End)
      break;

    tokens.push_back(std::move(token));
  }

  QCOMPARE(tokens.size(), 5);

  QCOMPARE(tokens[0].text, "Raw0");
  QCOMPARE(tokens[0].type, FormulaTokenType::Identifier);
  QCOMPARE(tokens[0].number, 0.);

  QCOMPARE(tokens[1].text, "+");
  QCOMPARE(tokens[1].type, FormulaTokenType::Plus);
  QCOMPARE(tokens[1].number, 0.);

  QCOMPARE(tokens[2].text, "Raw1");
  QCOMPARE(tokens[2].type, FormulaTokenType::Identifier);
  QCOMPARE(tokens[2].number, 0.);

  QCOMPARE(tokens[3].text, "*");
  QCOMPARE(tokens[3].type, FormulaTokenType::Multiply);
  QCOMPARE(tokens[3].number, 0.);

  QCOMPARE(tokens[4].text, "2");
  QCOMPARE(tokens[4].type, FormulaTokenType::Number);
  QCOMPARE(tokens[4].number, 2.);
}

void tst_formulas::test_formulas_lexer_numbers()
{
  using namespace qds;

  FormulaLexer lexer("12 12.5 0.25 .5 5.");

  std::vector<FormulaToken> tokens;

  for (;;)
  {
    auto token = lexer.next();

    if (token.type == FormulaTokenType::End)
      break;

    tokens.push_back(std::move(token));
  }

  QCOMPARE(tokens.size(), 5);

  QCOMPARE(tokens[0].type, FormulaTokenType::Number);
  QCOMPARE(tokens[0].text, "12");
  QCOMPARE(tokens[0].number, 12.0);

  QCOMPARE(tokens[1].type, FormulaTokenType::Number);
  QCOMPARE(tokens[1].text, "12.5");
  QCOMPARE(tokens[1].number, 12.5);

  QCOMPARE(tokens[2].type, FormulaTokenType::Number);
  QCOMPARE(tokens[2].text, "0.25");
  QCOMPARE(tokens[2].number, 0.25);

  QCOMPARE(tokens[3].type, FormulaTokenType::Number);
  QCOMPARE(tokens[3].text, ".5");
  QCOMPARE(tokens[3].number, 0.5);

  QCOMPARE(tokens[4].type, FormulaTokenType::Number);
  QCOMPARE(tokens[4].text, "5.");
  QCOMPARE(tokens[4].number, 5.0);
}

void tst_formulas::test_formulas_lexer_identifiers()
{
  using namespace qds;

  FormulaLexer lexer("A Raw123 signal_1 _raw");

  std::vector<FormulaToken> tokens;

  for (;;)
  {
    auto token = lexer.next();

    if (token.type == FormulaTokenType::End)
      break;

    tokens.push_back(std::move(token));
  }

  QCOMPARE(tokens.size(), 4);

  QCOMPARE(tokens[0].type, FormulaTokenType::Identifier);
  QCOMPARE(tokens[0].text, "A");
  QCOMPARE(tokens[0].number, 0.0);

  QCOMPARE(tokens[1].type, FormulaTokenType::Identifier);
  QCOMPARE(tokens[1].text, "Raw123");
  QCOMPARE(tokens[1].number, 0.0);

  QCOMPARE(tokens[2].type, FormulaTokenType::Identifier);
  QCOMPARE(tokens[2].text, "signal_1");
  QCOMPARE(tokens[2].number, 0.0);

  QCOMPARE(tokens[3].type, FormulaTokenType::Identifier);
  QCOMPARE(tokens[3].text, "_raw");
  QCOMPARE(tokens[3].number, 0.0);
}

void tst_formulas::test_formulas_lexer_invalid()
{
  using namespace qds;

  FormulaLexer lexer("2 +- 2 = 4");

  std::vector<FormulaToken> tokens;

  for (;;)
  {
    auto token = lexer.next();

    if (token.type == FormulaTokenType::End)
      break;

    tokens.push_back(std::move(token));
  }

  QCOMPARE(tokens.size(), 6);

  QCOMPARE(tokens[0].type, FormulaTokenType::Number);
  QCOMPARE(tokens[0].text, "2");
  QCOMPARE(tokens[0].number, 2.0);

  QCOMPARE(tokens[1].type, FormulaTokenType::Plus);
  QCOMPARE(tokens[1].text, "+");
  QCOMPARE(tokens[1].number, 0.0);

  QCOMPARE(tokens[2].type, FormulaTokenType::Minus);
  QCOMPARE(tokens[2].text, "-");
  QCOMPARE(tokens[2].number, 0.0);

  QCOMPARE(tokens[3].type, FormulaTokenType::Number);
  QCOMPARE(tokens[3].text, "2");
  QCOMPARE(tokens[3].number, 2.0);

  QCOMPARE(tokens[4].type, FormulaTokenType::Invalid);
  QCOMPARE(tokens[4].text, "=");
  QCOMPARE(tokens[4].number, 0.0);

  QCOMPARE(tokens[5].type, FormulaTokenType::Number);
  QCOMPARE(tokens[5].text, "4");
  QCOMPARE(tokens[5].number, 4.0);
}

void tst_formulas::test_formulas_lexer_operators()
{
  using namespace qds;

  FormulaLexer lexer("( ) + - * /");

  std::vector<FormulaToken> tokens;

  for (;;)
  {
    auto token = lexer.next();

    if (token.type == FormulaTokenType::End)
      break;

    tokens.push_back(std::move(token));
  }

  QCOMPARE(tokens.size(), 6);

  QCOMPARE(tokens[0].type, FormulaTokenType::LeftParen);
  QCOMPARE(tokens[0].text, "(");

  QCOMPARE(tokens[1].type, FormulaTokenType::RightParen);
  QCOMPARE(tokens[1].text, ")");

  QCOMPARE(tokens[2].type, FormulaTokenType::Plus);
  QCOMPARE(tokens[2].text, "+");

  QCOMPARE(tokens[3].type, FormulaTokenType::Minus);
  QCOMPARE(tokens[3].text, "-");

  QCOMPARE(tokens[4].type, FormulaTokenType::Multiply);
  QCOMPARE(tokens[4].text, "*");

  QCOMPARE(tokens[5].type, FormulaTokenType::Divide);
  QCOMPARE(tokens[5].text, "/");
}