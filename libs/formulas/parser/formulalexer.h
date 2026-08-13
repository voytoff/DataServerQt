#pragma once

#include "formulatoken.h"

namespace qds
{

class FormulaLexer
{
public:

  explicit FormulaLexer(
    std::string_view text);

  FormulaToken next();

private:

  std::string_view m_text;
  std::size_t m_position = 0;
};

}

