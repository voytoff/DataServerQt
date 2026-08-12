#pragma once

#include <cstdint>
#include <string>

namespace qds
{

enum class FormulaTokenType
{
  Number,
  Identifier,

  Plus,
  Minus,
  Multiply,
  Divide,

  LeftParen,
  RightParen,

  End,
  Invalid
};


struct FormulaToken
{
  FormulaTokenType type =
    FormulaTokenType::Invalid;

  std::string text;

  double number = 0.0;
};

}