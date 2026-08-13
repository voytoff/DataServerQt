#pragma once

#include "parser/formulanode.h"
#include "signalmemory.h"

namespace qds
{

class FormulaEvaluator
{
public:

  bool evaluate(
    const FormulaNode& root,
    const RawMemory& raw,
    CalculatedMemory& calculated,
    double& result) const noexcept;

private:

  bool evaluateNode(
    const FormulaNode& node,
    const RawMemory& raw,
    const CalculatedMemory& calculated,
    double& result) const noexcept;
};

}

