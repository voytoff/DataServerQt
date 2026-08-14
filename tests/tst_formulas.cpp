#include "tst_formulas.h"
#include "calculationorder.h"
#include "parser/formulaevaluator.h"
#include "formulafunctionsqrt.h"
#include "parser/formulalexer.h"
#include "parser/formulaparser.h"
#include "parser/identifierresolver.h"
#include "signalmemory.h"
#include "testsrv.h"
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

void tst_formulas::test_formulas_parser_number()
{
  using namespace qds;

  FormulaParser parser("123.5");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::Number);

  QCOMPARE(root->number, 123.5);
}

void tst_formulas::test_formulas_parser_identifier()
{
  using namespace qds;

  FormulaParser parser("Raw123");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::Identifier);

  QCOMPARE(root->identifier, "Raw123");
}

void tst_formulas::test_formulas_parser_add()
{
  using namespace qds;

  FormulaParser parser("Raw0 + Raw1");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::Add);

  QVERIFY(root->left != nullptr);
  QVERIFY(root->right != nullptr);

  QCOMPARE(
    root->left->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    root->left->identifier,
    "Raw0");

  QCOMPARE(
    root->right->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    root->right->identifier,
    "Raw1");
}

void tst_formulas::test_formulas_parser_tree_base()
{
  using namespace qds;

  FormulaParser parser("Raw0 + Raw1 * 2");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::Add);

  QVERIFY(root->left != nullptr);
  QVERIFY(root->right != nullptr);

  QCOMPARE(
    root->left->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    root->left->identifier,
    "Raw0");

  const auto &right0 = root->right;

  QCOMPARE(
    right0->type,
    FormulaNodeType::Multiply);

  QVERIFY(right0->left != nullptr);
  QVERIFY(right0->right != nullptr);

  QCOMPARE(
    right0->left->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    right0->left->identifier,
    "Raw1");

  QCOMPARE(
    right0->right->type,
    FormulaNodeType::Number);

  QCOMPARE(right0->right->number, 2.0);
}

void tst_formulas::test_formulas_parser_parentheses()
{
  using namespace qds;

  FormulaParser parser("(Raw0 + Raw1) * 2");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::Multiply);

  QVERIFY(root->left != nullptr);
  QVERIFY(root->right != nullptr);

  const auto& left = root->left;

  QCOMPARE(
    left->type,
    FormulaNodeType::Add);

  QVERIFY(left->left != nullptr);
  QVERIFY(left->right != nullptr);

  QCOMPARE(
    left->left->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    left->left->identifier,
    "Raw0");

  QCOMPARE(
    left->right->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    left->right->identifier,
    "Raw1");

  QCOMPARE(
    root->right->type,
    FormulaNodeType::Number);

  QCOMPARE(
    root->right->number,
    2.0);
}

void tst_formulas::test_formulas_parser_unaryMinus()
{
  using namespace qds;

  FormulaParser parser("-Raw0");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::Negate);

  QVERIFY(root->left != nullptr);

  QCOMPARE(
    root->left->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    root->left->identifier,
    "Raw0");

  QVERIFY(root->right == nullptr);
}
//------
void tst_formulas::test_formulas_parser_unaryMinusWithIdentifer()
{
  using namespace qds;

  FormulaParser parser("-Raw0 + Raw1");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QVERIFY(root->left != nullptr);
  QVERIFY(root->right != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::Add);

  const auto &left = root->left;

  QCOMPARE(
    left->type,
    FormulaNodeType::Negate);

  QVERIFY(left->left != nullptr);

  QCOMPARE(
    left->left->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    left->left->identifier,
    "Raw0");

  QCOMPARE(
    root->right->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    root->right->identifier,
    "Raw1");
}

void tst_formulas::test_formulas_parser_unaryMinusWithOperator()
{
  using namespace qds;

  FormulaParser parser("-(Raw0 + Raw1)");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QVERIFY(root->left != nullptr);
  QVERIFY(root->right == nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::Negate);

  const auto &left = root->left;

  QCOMPARE(
    left->type,
    FormulaNodeType::Add);

  QVERIFY(left->left != nullptr);
  QVERIFY(left->right != nullptr);

  QCOMPARE(
    left->left->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    left->left->identifier,
    "Raw0");

  QCOMPARE(
    left->right->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    left->right->identifier,
    "Raw1");
}

void tst_formulas::test_formulas_parser_errorPluseEnds()
{
  using namespace qds;

  FormulaParser parser("Raw0 +");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_errorMultiplyEnds()
{
  using namespace qds;

  FormulaParser parser("Raw0 *");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_errorOperatorEnds()
{
  using namespace qds;

  FormulaParser parser("(Raw0 + Raw1");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_errorBktEnds()
{
  using namespace qds;

  FormulaParser parser("Raw0 + )");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_errorEqualsEnds()
{
  using namespace qds;

  FormulaParser parser("Raw0 + =");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_errorOperatorEqualsEnds()
{
  using namespace qds;

  FormulaParser parser("Raw0 + Raw1 =");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_operatorPrecedence()
{
  using namespace qds;

  FormulaParser parser("1 + 2 * 3 - 4 / 2");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(root->type, FormulaNodeType::Subtract);

  QVERIFY(root->left != nullptr);
  QVERIFY(root->right != nullptr);

  // (1 + (2 * 3)) - (4 / 2)

  const auto& add = root->left;

  QCOMPARE(add->type, FormulaNodeType::Add);

  QVERIFY(add->left != nullptr);
  QVERIFY(add->right != nullptr);

  QCOMPARE(add->left->type, FormulaNodeType::Number);
  QCOMPARE(add->left->number, 1.0);

  const auto& multiply = add->right;

  QCOMPARE(multiply->type, FormulaNodeType::Multiply);

  QCOMPARE(multiply->left->type, FormulaNodeType::Number);
  QCOMPARE(multiply->left->number, 2.0);

  QCOMPARE(multiply->right->type, FormulaNodeType::Number);
  QCOMPARE(multiply->right->number, 3.0);

  const auto& divide = root->right;

  QCOMPARE(divide->type, FormulaNodeType::Divide);

  QCOMPARE(divide->left->type, FormulaNodeType::Number);
  QCOMPARE(divide->left->number, 4.0);

  QCOMPARE(divide->right->type, FormulaNodeType::Number);
  QCOMPARE(divide->right->number, 2.0);
}

void tst_formulas::test_formulas_parser_leftAssociative()
{
  using namespace qds;

  FormulaParser parser("10 - 3 - 2");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  // (10 - 3) - 2

  QCOMPARE(root->type, FormulaNodeType::Subtract);

  QVERIFY(root->left != nullptr);
  QVERIFY(root->right != nullptr);

  QCOMPARE(root->right->type, FormulaNodeType::Number);
  QCOMPARE(root->right->number, 2.0);

  const auto& left = root->left;

  QCOMPARE(left->type, FormulaNodeType::Subtract);

  QCOMPARE(left->left->type, FormulaNodeType::Number);
  QCOMPARE(left->left->number, 10.0);

  QCOMPARE(left->right->type, FormulaNodeType::Number);
  QCOMPARE(left->right->number, 3.0);
}

void tst_formulas::test_formulas_parser_functionCall()
{
  using namespace qds;

  FormulaParser parser("sqrt(Raw0)");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::FunctionCall);

  QCOMPARE(
    root->identifier,
    "sqrt");

  QCOMPARE(
    root->arguments.size(),
    std::size_t(1));

  QVERIFY(root->arguments[0] != nullptr);

  QCOMPARE(
    root->arguments[0]->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    root->arguments[0]->identifier,
    "Raw0");
}

void tst_formulas::test_formulas_parser_nestedFunctionCall()
{
  using namespace qds;

  FormulaParser parser(
    "sqrt(Raw0 + Raw1)");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::FunctionCall);

  QCOMPARE(
    root->identifier,
    "sqrt");

  QCOMPARE(
    root->arguments.size(),
    std::size_t(1));

  const auto& argument =
    root->arguments[0];

  QVERIFY(argument != nullptr);

  QCOMPARE(
    argument->type,
    FormulaNodeType::Add);

  QCOMPARE(
    argument->left->identifier,
    "Raw0");

  QCOMPARE(
    argument->right->identifier,
    "Raw1");
}

void tst_formulas::test_formulas_parser_functionCall_multipleArguments()
{
  using namespace qds;

  FormulaParser parser(
    "add(Raw0, Raw1)");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::FunctionCall);

  QCOMPARE(
    root->identifier,
    "add");

  QCOMPARE(
    root->arguments.size(),
    std::size_t(2));

  QVERIFY(root->arguments[0] != nullptr);
  QVERIFY(root->arguments[1] != nullptr);

  QCOMPARE(
    root->arguments[0]->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    root->arguments[0]->identifier,
    "Raw0");

  QCOMPARE(
    root->arguments[1]->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    root->arguments[1]->identifier,
    "Raw1");
}

void tst_formulas::test_formulas_parser_functionCall_errorBktEnds()
{
  using namespace qds;

  FormulaParser parser("sqrt(");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_functionCall_errorArgumentEnds()
{
  using namespace qds;

  FormulaParser parser("sqrt(Raw0");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_functionCall_errorComaArgumentsEnds()
{
  using namespace qds;

  FormulaParser parser("sqrt(Raw0, )");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_functionCall_errorComaArgumentsStart()
{
  using namespace qds;

  FormulaParser parser("sqrt(, Raw0)");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_functionCall_errorMissingComaArguments()
{
  using namespace qds;

  FormulaParser parser("sqrt(Raw0 Raw1)");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_functionCall_errorDuplicateBkts()
{
  using namespace qds;

  FormulaParser parser("sqrt(Raw0))");

  auto root = parser.parse();

  QVERIFY(root == nullptr);
}

void tst_formulas::test_formulas_parser_functionCall_SomeArguments()
{
  using namespace qds;

  FormulaParser parser(
    "add(Raw0, 2, Raw1, 44.4)");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::FunctionCall);

  QCOMPARE(
    root->identifier,
    "add");

  QCOMPARE(
    root->arguments.size(),
    std::size_t(4));

  QVERIFY(root->arguments[0] != nullptr);
  QVERIFY(root->arguments[1] != nullptr);
  QVERIFY(root->arguments[2] != nullptr);
  QVERIFY(root->arguments[3] != nullptr);

  QCOMPARE(
    root->arguments[0]->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    root->arguments[0]->identifier,
    "Raw0");

  QCOMPARE(
    root->arguments[1]->type,
    FormulaNodeType::Number);

  QCOMPARE(
    root->arguments[1]->number,
    2.0);

  QCOMPARE(
    root->arguments[2]->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    root->arguments[2]->identifier,
    "Raw1");

  QCOMPARE(
    root->arguments[3]->type,
    FormulaNodeType::Number);

  QCOMPARE(
    root->arguments[3]->number,
    44.4);
}

void tst_formulas::test_formulas_parser_nestedFunctionCallFuncArguments()
{
  using namespace qds;

  FormulaParser parser(
    "sqrt(Raw0 * 2 - 3 / Raw1 + 7.2)");

  auto root = parser.parse();

  QVERIFY(root != nullptr);
  QVERIFY(root->left == nullptr);
  QVERIFY(root->right == nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::FunctionCall);

  QCOMPARE(
    root->identifier,
    "sqrt");

  QCOMPARE(
    root->arguments.size(),
    std::size_t(1));

  const auto &add = root->arguments[0];

  QCOMPARE(
    add->type,
    FormulaNodeType::Add); // ((Raw0 * 2) - (3 / Raw1)) [+] 7.2

  QVERIFY(add->left != nullptr);
  QVERIFY(add->right != nullptr);

  const auto &left0 = add->left;
  const auto &right0 = add->right;

  QCOMPARE(
    left0->type,
    FormulaNodeType::Subtract); // (Raw0 * 2) [-] (3 / Raw1)

  QVERIFY(left0->left != nullptr);
  QVERIFY(left0->right != nullptr);

  QCOMPARE(
    right0->type,
    FormulaNodeType::Number); // 7.2

  QCOMPARE(
    right0->number,
    7.2);

  QVERIFY(right0->left == nullptr);
  QVERIFY(right0->right == nullptr);

  const auto &left1 = left0->left;
  const auto &right1 = left0->right;

  QVERIFY(left1->left != nullptr);
  QVERIFY(left1->right != nullptr);

  QCOMPARE(
    left1->type,
    FormulaNodeType::Multiply); // Raw0 [*] 2

  QCOMPARE(
    left1->left->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    left1->left->identifier,
    "Raw0");

  QCOMPARE(
    left1->right->type,
    FormulaNodeType::Number);

  QCOMPARE(
    left1->right->number,
    2.0);

  QVERIFY(right1->left != nullptr);
  QVERIFY(right1->right != nullptr);

  QCOMPARE(
    right1->type,
    FormulaNodeType::Divide); // 3 [/] Raw1

  QCOMPARE(
    right1->left->type,
    FormulaNodeType::Number);

  QCOMPARE(
    right1->left->number,
    3.0);

  QCOMPARE(
    right1->right->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    right1->right->identifier,
    "Raw1");
}

void tst_formulas::test_formulas_parser_functionCallAndArguments()
{
  using namespace qds;

  FormulaParser parser(
    "sqrt(Raw0 + Raw1) * 2");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  QCOMPARE(
    root->type,
    FormulaNodeType::Multiply); // sqrt(Raw0 + Raw1) [*] 2

  QVERIFY(root->left != nullptr);
  QVERIFY(root->right != nullptr);

  QCOMPARE(
    root->left->type,
    FormulaNodeType::FunctionCall);

  QCOMPARE(
    root->left->identifier,
    "sqrt");

  QCOMPARE(
    root->left->arguments.size(),
    std::size_t(1));

  const auto &left = root->left->arguments[0];

  QCOMPARE(
    left->type,
    FormulaNodeType::Add); // Raw0 + Raw1

  QVERIFY(left->left != nullptr);
  QVERIFY(left->right != nullptr);

  QCOMPARE(
    left->left->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    left->left->identifier,
    "Raw0");

  QCOMPARE(
    left->right->type,
    FormulaNodeType::Identifier);

  QCOMPARE(
    left->right->identifier,
    "Raw1");

  QCOMPARE(
    root->right->type,
    FormulaNodeType::Number); // 2

  QCOMPARE(
    root->right->number,
    2.0);
}

void tst_formulas::test_formulas_identifierResolver()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser(
    "Raw0 + Raw1 * 2");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  IdentifierResolver resolver(
    cfg,
    layout);

  QVERIFY(resolver.resolve(*root));

  QCOMPARE(
    root->type,
    FormulaNodeType::Add);

  QCOMPARE(
    root->left->type,
    FormulaNodeType::Signal);

  QCOMPARE(
    root->left->signal.area,
    SignalMemoryArea::Raw);

  QCOMPARE(
    root->left->signal.index,
    0u);

  QCOMPARE(
    root->right->type,
    FormulaNodeType::Multiply);

  QCOMPARE(
    root->right->left->type,
    FormulaNodeType::Signal);

  QCOMPARE(
    root->right->left->signal.area,
    SignalMemoryArea::Raw);

  QCOMPARE(
    root->right->left->signal.index,
    1u);

  QCOMPARE(
    root->right->right->type,
    FormulaNodeType::Number);

  QCOMPARE(
    root->right->right->number,
    2.0);
}

void tst_formulas::test_formulas_identifierResolver_unknown()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser(
    "Raw0 + Unknown");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  IdentifierResolver resolver(
    cfg,
    layout);

  QVERIFY(!resolver.resolve(*root));
}

void tst_formulas::test_formulas_identifierResolverCalculated()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser(
    "Raw0 * B + (A + C)");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  IdentifierResolver resolver(
    cfg,
    layout);

  QVERIFY(resolver.resolve(*root));

  QCOMPARE(
    root->type,
    FormulaNodeType::Add);

  QVERIFY(root->left != nullptr);

  QCOMPARE(
    root->left->type,
    FormulaNodeType::Multiply);

  QVERIFY(root->left->left != nullptr);

  QCOMPARE(
    root->left->left->type,
    FormulaNodeType::Signal);

  QCOMPARE(
    root->left->left->signal.area,
    SignalMemoryArea::Raw);

  QCOMPARE(
    root->left->left->signal.index, // Raw0
    0u);

  QVERIFY(root->left->right != nullptr);

  QCOMPARE(
    root->left->right->type,
    FormulaNodeType::Signal);

  QCOMPARE(
    root->left->right->signal.area,
    SignalMemoryArea::Calculated);

  QCOMPARE(
    root->left->right->signal.index, // B
    1u);

  QVERIFY(root->right != nullptr);

  QCOMPARE(
    root->right->type,
    FormulaNodeType::Add);

  QVERIFY(root->right->left != nullptr);

  QCOMPARE(
    root->right->left->type,
    FormulaNodeType::Signal);

  QCOMPARE(
    root->right->left->signal.area,
    SignalMemoryArea::Calculated);

  QCOMPARE(
    root->right->left->signal.index, // A
    0u);

  QVERIFY(root->right->right != nullptr);

  QCOMPARE(
    root->right->right->type,
    FormulaNodeType::Signal);

  QCOMPARE(
    root->right->right->signal.area,
    SignalMemoryArea::Calculated);

  QCOMPARE(
    root->right->right->signal.index, // C
    2u);
}

void tst_formulas::test_formulas_evaluator_number()
{
  using namespace qds;

  FormulaParser parser("42.5");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(0);
  calculated.initialize(0);

  FormulaFunctionRepository functions;
  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 42.5);
}

void tst_formulas::test_formulas_evaluator_raw0()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("Raw0");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  IdentifierResolver resolver(
    cfg,
    layout);

  QVERIFY(resolver.resolve(*root));

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(layout.rawSignalCount());

  raw.setValue(0, 12.34);       // Raw0

  FormulaFunctionRepository functions;
  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 12.34);
}

void tst_formulas::test_formulas_evaluator_raw0AndRaw1()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("Raw0 + Raw1");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  IdentifierResolver resolver(
    cfg,
    layout);

  QVERIFY(resolver.resolve(*root));

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(layout.rawSignalCount());

  raw.setValue(0, 15.0);       // Raw0
  raw.setValue(1, 30.0);       // Raw1

  FormulaFunctionRepository functions;
  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 45.0);
}

void tst_formulas::test_formulaEvaluator_base()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser(
    "Raw0 * B + (A + C)");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  IdentifierResolver resolver(
    cfg,
    layout);

  QVERIFY(resolver.resolve(*root));

  RawMemory raw;
  raw.initialize(layout.rawSignalCount());

  CalculatedMemory calculated;
  calculated.initialize(layout.calculatedSignalCount());

  raw.setValue(0, 2.0);       // Raw0

  calculated.setValue(0, 10.0); // A
  calculated.setValue(1, 20.0); // B
  calculated.setValue(2, 30.0); // C

  FormulaFunctionRepository functions;
  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(
    evaluator.evaluate(
      *root,
      raw,
      calculated,
      result));

  QCOMPARE(result, 80.0);
}

void tst_formulas::test_formulas_evaluator_divideByZero()
{
  using namespace qds;

  FormulaParser parser("Raw0 / Raw1");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  RawMemory raw;
  raw.initialize(layout.rawSignalCount());

  raw.setValue(0, 10.0);
  raw.setValue(1, 0.0);

  CalculatedMemory calculated;
  calculated.initialize(layout.calculatedSignalCount());

  FormulaFunctionRepository functions;
  FormulaEvaluator evaluator(functions);

  double result = 123.0;

  QVERIFY(!evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 123.0);
}

void tst_formulas::test_formulas_evaluator_failSignalReference()
{
  using namespace qds;

  FormulaParser parser("Raw0 + A");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  RawMemory raw;
  raw.initialize(layout.rawSignalCount());

  raw.setValue(0, 10.0);

  CalculatedMemory calculated;
  calculated.initialize(layout.calculatedSignalCount());

  calculated.setValue(0, 20.0);

  QCOMPARE(root->right->type, FormulaNodeType::Signal);
  root->right->signal.index = 1000;

  FormulaFunctionRepository functions;
  FormulaEvaluator evaluator(functions);

  double result = 123.0;

  QVERIFY(!evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 123.0);
}

void tst_formulas::test_formulas_evaluator_subtract()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("Raw1 - Raw0 - 5.0");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  IdentifierResolver resolver(
    cfg,
    layout);

  QVERIFY(resolver.resolve(*root));

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(layout.rawSignalCount());

  raw.setValue(0, 15.0);       // Raw0
  raw.setValue(1, 30.0);       // Raw1

  FormulaFunctionRepository functions;
  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 10.0);
}

void tst_formulas::test_formulas_evaluator_multiply()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("Raw1 * B * 2.0");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  IdentifierResolver resolver(
    cfg,
    layout);

  QVERIFY(resolver.resolve(*root));

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(layout.rawSignalCount());
  calculated.initialize(layout.rawSignalCount());

  raw.setValue(1, 3.0);         // Raw1
  calculated.setValue(1, 7.0);  // B

  FormulaFunctionRepository functions;
  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 42.0);
}

void tst_formulas::test_formulas_evaluator_negate()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("-C");

  auto root = parser.parse();

  QVERIFY(root != nullptr);

  IdentifierResolver resolver(
    cfg,
    layout);

  QVERIFY(resolver.resolve(*root));

  RawMemory raw;
  CalculatedMemory calculated;

  calculated.initialize(layout.calculatedSignalCount());

  calculated.setValue(2, -112.0);  // C

  FormulaFunctionRepository functions;
  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 112.0);
}

void tst_formulas::test_formulas_evaluator_formulaFunctionRepository()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("sqrt(Raw0 * 4)");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "sqrt",
    std::make_unique<FormulaFunctionSqrt>()));

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(layout.rawSignalCount());
  raw.setValue(0, 4.0);       // Raw0

  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 4.0);
}

void tst_formulas::test_formulas_formulaFunctionRepository_uniqueNames()
{
  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "sqrt",
    std::make_unique<FormulaFunctionSqrt>()));

  QVERIFY(!functions.add(
    "sqrt",
    std::make_unique<FormulaFunctionSqrt>()));
}

void tst_formulas::test_formulas_formulaFunctionRepository_find()
{
  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "sqrt",
    std::make_unique<FormulaFunctionSqrt>()));

  QVERIFY(functions.find("sqrt") != nullptr);

  QVERIFY(functions.find("unknown") == nullptr);
}

void tst_formulas::test_formulas_formulaFunctionRepository_clear()
{
  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "sqrt",
    std::make_unique<FormulaFunctionSqrt>()));

  QCOMPARE(functions.size(), std::size_t(1));
  QVERIFY(functions.find("sqrt") != nullptr);

  functions.clear();

  QCOMPARE(functions.size(), std::size_t(0));
  QVERIFY(functions.find("sqrt") == nullptr);
}

void tst_formulas::test_formulas_functionCall_unknown()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("unknown(Raw0 * 4)");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "sqrt",
    std::make_unique<FormulaFunctionSqrt>()));

  RawMemory raw;
  CalculatedMemory calculated;

  FormulaEvaluator evaluator(functions);

  double result = 123.0;

  QVERIFY(!evaluator.evaluate( // if (function == nullptr) return false;
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 123.0);
}

void tst_formulas::test_formulas_functionCall_missingParams()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("sqrt()");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "sqrt",
    std::make_unique<FormulaFunctionSqrt>()));

  RawMemory raw;
  CalculatedMemory calculated;

  FormulaEvaluator evaluator(functions);

  double result = 123.0;

  QVERIFY(!evaluator.evaluate( // if (arguments.size() != 1) return false;
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 123.0);
}

void tst_formulas::test_formulas_functionCall_failCountParams()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("sqrt(1, 2)");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "sqrt",
    std::make_unique<FormulaFunctionSqrt>()));

  RawMemory raw;
  CalculatedMemory calculated;

  FormulaEvaluator evaluator(functions);

  double result = 123.0;

  QVERIFY(!evaluator.evaluate( // if (arguments.size() != 1) return false;
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 123.0);
}

void tst_formulas::test_formulas_functionCall_failValueParams()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("sqrt(-1)");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "sqrt",
    std::make_unique<FormulaFunctionSqrt>()));

  RawMemory raw;
  CalculatedMemory calculated;

  FormulaEvaluator evaluator(functions);

  double result = 123.0;

  QVERIFY(!evaluator.evaluate( // для sqrt - if (arguments[0] < 0.0) return false;
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 123.0);
}

void tst_formulas::test_formulas_functionRepository_invalidAdd()
{
  using namespace qds;

  FormulaFunctionRepository functions;

  QVERIFY(!functions.add(
    "",
    std::make_unique<FormulaFunctionSqrt>()));

  QVERIFY(!functions.add(
    "sqrt",
    nullptr));

  QCOMPARE(
    functions.size(),
    std::size_t(0));
}

void tst_formulas::test_formulas_functionCall_sqrtSignal()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("sqrt(Raw0)");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "sqrt",
    std::make_unique<FormulaFunctionSqrt>()));

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(layout.rawSignalCount());
  calculated.initialize(layout.calculatedSignalCount());

  raw.setValue(0, 16.0);

  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 4.0);
}

void tst_formulas::test_formulas_functionRepository_abs_negate()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("abs(-Raw0)");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "abs",
    std::make_unique<FormulaFunctionAbs>()));

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(layout.rawSignalCount());
  calculated.initialize(layout.calculatedSignalCount());

  raw.setValue(0, -25.0);

  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 25.0);
}

void tst_formulas::test_formulas_functionRepository_max()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("max(Raw0, Raw1, 7)");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "max",
    std::make_unique<FormulaFunctionMax>()));

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(layout.rawSignalCount());
  calculated.initialize(layout.calculatedSignalCount());

  raw.setValue(0, 10.0);
  raw.setValue(1, 20.0);

  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 20.0);
}

void tst_formulas::test_formulas_functionRepository_min()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("min(Raw0, 7, Raw1)");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "min",
    std::make_unique<FormulaFunctionMin>()));

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(layout.rawSignalCount());
  calculated.initialize(layout.calculatedSignalCount());

  raw.setValue(0, 10.0);
  raw.setValue(1, 20.0);

  FormulaEvaluator evaluator(functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 7.0);
}

void tst_formulas::test_formulas_functionRepository_max_missingParams()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("max()");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "max",
    std::make_unique<FormulaFunctionMax>()));

  RawMemory raw;
  CalculatedMemory calculated;
  raw.initialize(layout.rawSignalCount());
  calculated.initialize(layout.calculatedSignalCount());

  FormulaEvaluator evaluator(functions);

  double result = 123.0;

  QVERIFY(!evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 123.0);
}

void tst_formulas::test_formulas_functionRepository_max_failParams()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("max(1)");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "max",
    std::make_unique<FormulaFunctionMax>()));

  RawMemory raw;
  CalculatedMemory calculated;
  raw.initialize(layout.rawSignalCount());
  calculated.initialize(layout.calculatedSignalCount());

  FormulaEvaluator evaluator(functions);

  double result = 123.0;

  QVERIFY(!evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 123.0);
}

void tst_formulas::test_formulas_functionRepository_min_missingParams()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("min()");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "min",
    std::make_unique<FormulaFunctionMin>()));

  RawMemory raw;
  CalculatedMemory calculated;
  raw.initialize(layout.rawSignalCount());
  calculated.initialize(layout.calculatedSignalCount());

  FormulaEvaluator evaluator(functions);

  double result = 123.0;

  QVERIFY(!evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 123.0);
}

void tst_formulas::test_formulas_functionRepository_min_failParams()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser("min(1)");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  FormulaFunctionRepository functions;

  QVERIFY(functions.add(
    "min",
    std::make_unique<FormulaFunctionMin>()));

  RawMemory raw;
  CalculatedMemory calculated;
  raw.initialize(layout.rawSignalCount());
  calculated.initialize(layout.calculatedSignalCount());

  FormulaEvaluator evaluator(functions);

  double result = 123.0;

  QVERIFY(!evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 123.0);
}

void tst_formulas::test_formulas_functionRepository_nesteFunctions()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  SignalMemoryLayout layout;
  layout.build(cfg);

  FormulaParser parser(
    "max(abs(Raw0), min(Raw1, 10), sqrt(C) * 7)");

  auto root = parser.parse();
  QVERIFY(root != nullptr);

  IdentifierResolver resolver(cfg, layout);
  QVERIFY(resolver.resolve(*root));

  auto functions = createFormulaFunctionRepository();
  QVERIFY(functions != nullptr);

  RawMemory raw;
  CalculatedMemory calculated;

  raw.initialize(layout.rawSignalCount());
  calculated.initialize(layout.calculatedSignalCount());

  raw.setValue(0, -30.0);         // Raw0
  raw.setValue(1, 16.0);         // Raw1
  calculated.setValue(2, 25.0);  // C


  FormulaEvaluator evaluator(*functions);

  double result = 0.0;

  QVERIFY(evaluator.evaluate(
    *root,
    raw,
    calculated,
    result));

  QCOMPARE(result, 35.0);
}

void tst_formulas::test_calculationOrder_base()
{
  using namespace qds;

  SystemConfiguration cfg =
    createTestConfig_Copy_Add();

  CalculationOrder order;

  QVERIFY(order.build(cfg));

  QCOMPARE(order.order().size(), std::size_t(3));

  QCOMPARE(order.order()[0], SignalId{2}); // A
  QCOMPARE(order.order()[1], SignalId{3}); // B
  QCOMPARE(order.order()[2], SignalId{4}); // C
}