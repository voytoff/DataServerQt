#pragma once

#include "parser/formulanode.h"

namespace qds
{

class PreprocessingResolver
{
public:
  bool resolve(FormulaNode& root);

private:
  bool resolveNode(FormulaNode& node);
};

}