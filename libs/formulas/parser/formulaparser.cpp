#include "formulaparser.h"

namespace qds
{

FormulaParser::FormulaParser(
  std::string_view text)
  : m_lexer(text)
{
  m_current = m_lexer.next();
}

bool FormulaParser::accept(
  FormulaTokenType type)
{
  if (m_current.type != type)
    return false;

  m_current = m_lexer.next();
  return true;
}

bool FormulaParser::expect(
  FormulaTokenType type)
{
  if (m_current.type != type)
    return false;

  m_current = m_lexer.next();
  return true;
}

std::unique_ptr<FormulaNode>
FormulaParser::parse()
{
  auto node = parseExpression();

  if (!node)
    return nullptr;

  if (m_current.type != FormulaTokenType::End)
    return nullptr;

  return node;
}

std::unique_ptr<FormulaNode>
FormulaParser::parseExpression()
{
  auto left = parseTerm();

  if (!left)
    return nullptr;

  while (m_current.type == FormulaTokenType::Plus ||
         m_current.type == FormulaTokenType::Minus)
  {
    const auto operation = m_current.type;

    m_current = m_lexer.next();

    auto right = parseTerm();

    if (!right)
      return nullptr;

    auto node = std::make_unique<FormulaNode>();

    node->type =
      operation == FormulaTokenType::Plus
        ? FormulaNodeType::Add
        : FormulaNodeType::Subtract;

    node->left = std::move(left);
    node->right = std::move(right);

    left = std::move(node);
  }

  return left;
}

std::unique_ptr<FormulaNode>
FormulaParser::parseTerm()
{
  auto left = parseUnary();

  if (!left)
    return nullptr;

  while (m_current.type == FormulaTokenType::Multiply ||
         m_current.type == FormulaTokenType::Divide)
  {
    const auto operation = m_current.type;

    m_current = m_lexer.next();

    auto right = parseUnary();

    if (!right)
      return nullptr;

    auto node = std::make_unique<FormulaNode>();

    node->type =
      operation == FormulaTokenType::Multiply
        ? FormulaNodeType::Multiply
        : FormulaNodeType::Divide;

    node->left = std::move(left);
    node->right = std::move(right);

    left = std::move(node);
  }

  return left;
}

std::unique_ptr<FormulaNode>
FormulaParser::parseUnary()
{
  if (accept(FormulaTokenType::Plus))
    return parseUnary();

  if (accept(FormulaTokenType::Minus))
  {
    auto node = std::make_unique<FormulaNode>();

    node->type = FormulaNodeType::Negate;
    node->left = parseUnary();

    if (!node->left)
      return nullptr;

    return node;
  }

  return parsePrimary();
}

std::unique_ptr<FormulaNode>
FormulaParser::parsePrimary()
{
  if (m_current.type == FormulaTokenType::Number)
  {
    auto node = std::make_unique<FormulaNode>();

    node->type = FormulaNodeType::Number;
    node->number = m_current.number;

    m_current = m_lexer.next();

    return node;
  }

  if (m_current.type == FormulaTokenType::Identifier)
  {
    std::string name = m_current.text;

    m_current = m_lexer.next();

    if (m_current.type == FormulaTokenType::LeftParen)
      return parseFunctionCall(std::move(name));

    auto node = std::make_unique<FormulaNode>();

    node->type = FormulaNodeType::Identifier;
    node->identifier = std::move(name);

    return node;
  }

  if (accept(FormulaTokenType::LeftParen))
  {
    auto node = parseExpression();

    if (!node)
      return nullptr;

    if (!expect(FormulaTokenType::RightParen))
      return nullptr;

    return node;
  }

  return nullptr;
}

std::unique_ptr<FormulaNode>
FormulaParser::parseFunctionCall(
  std::string name)
{
  if (!expect(FormulaTokenType::LeftParen))
    return nullptr;

  auto node = std::make_unique<FormulaNode>();

  node->type = FormulaNodeType::FunctionCall;
  node->identifier = std::move(name);

  // f()
  if (accept(FormulaTokenType::RightParen))
    return node;

  // f(expr, expr, ...)
  for (;;)
  {
    auto argument = parseExpression();

    if (!argument)
      return nullptr;

    node->arguments.push_back(
      std::move(argument));

    if (accept(FormulaTokenType::RightParen))
      break;

    if (!expect(FormulaTokenType::Comma))
      return nullptr;
  }

  return node;
}

}