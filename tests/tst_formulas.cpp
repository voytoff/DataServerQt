#include "tst_formulas.h"
#include "parser/formulalexer.h"
#include "parser/formulaparser.h"
#include "parser/identifierresolver.h"
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

  QCOMPARE(
    root->left->type,
    FormulaNodeType::Multiply);

  QCOMPARE(
    root->left->left->type,
    FormulaNodeType::Signal);

  QCOMPARE(
    root->left->left->signal.area,
    SignalMemoryArea::Raw);

  QCOMPARE(
    root->left->left->signal.index,
    0u);

  QCOMPARE(
    root->left->right->type,
    FormulaNodeType::Signal);

  QCOMPARE(
    root->left->right->signal.area,
    SignalMemoryArea::Calculated);

  QCOMPARE(
    root->left->right->signal.index,
    1u);

  QCOMPARE(
    root->right->type,
    FormulaNodeType::Add);

}