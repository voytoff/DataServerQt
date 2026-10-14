#pragma once

#include <memory>
#include <string>
#include <vector>
#include "signaldefinition.h"

namespace qds
{

enum class FormulaNodeType
{
  Number,
  Identifier,
  Signal,

  Add,
  Subtract,
  Multiply,
  Divide,

  Negate,

  FunctionCall
};

struct FormulaNode
{
  FormulaNodeType type =
    FormulaNodeType::Number;

  double number = 0.0;

  std::string identifier;

  SignalReference signal;

  std::unique_ptr<FormulaNode> left;
  std::unique_ptr<FormulaNode> right;

  std::vector<std::unique_ptr<FormulaNode>> arguments;
};

}
