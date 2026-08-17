#pragma once

#include "formulafunctionrepository.h"
#include "parser/formulaevaluator.h"
#include "signalmemory.h"

namespace qds
{

class FormulaCalculator
{
public:

  FormulaCalculator();

  bool calculate(
    const FormulaNode& formula,
    const RawMemory& raw,
    const CalculatedMemory& calculated,
    double& result) const noexcept;

private:

  FormulaFunctionRepository m_functions;
  FormulaEvaluator m_evaluator;
};

}