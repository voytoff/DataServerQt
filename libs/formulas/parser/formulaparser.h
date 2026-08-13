#pragma once

#include "formulalexer.h"
#include "formulanode.h"
#include "formulatoken.h"

#include <memory>
#include <string_view>

namespace qds
{

class FormulaParser
{
public:

  explicit FormulaParser(
    std::string_view text);

  std::unique_ptr<FormulaNode> parse();

private:

  bool accept(
    FormulaTokenType type);

  bool expect(
    FormulaTokenType type);

  std::unique_ptr<FormulaNode>
  parseExpression();

  std::unique_ptr<FormulaNode>
  parseTerm();

  std::unique_ptr<FormulaNode>
  parseUnary();

  std::unique_ptr<FormulaNode>
  parsePrimary();

  std::unique_ptr<FormulaNode>
  parseFunctionCall(
    std::string name);

private:

  FormulaLexer m_lexer;
  FormulaToken m_current;
};

}