#include "formulacalculator.h"

#include "formulafunctionabs.h"
#include "formulafunctionmax.h"
#include "formulafunctionmin.h"
#include "formulafunctionsqrt.h"

#include <cassert>

namespace qds
{

FormulaCalculator::FormulaCalculator()
  : m_evaluator(m_functions)
{
  assert(m_functions.add(
    "abs",
    std::make_unique<FormulaFunctionAbs>()));

  assert(m_functions.add(
    "max",
    std::make_unique<FormulaFunctionMax>()));

  assert(m_functions.add(
    "min",
    std::make_unique<FormulaFunctionMin>()));

  assert(m_functions.add(
    "sqrt",
    std::make_unique<FormulaFunctionSqrt>()));
}

bool FormulaCalculator::calculate(
  const FormulaNode& formula,
  const RawMemory& raw,
  const CalculatedMemory& calculated,
  double& result) const noexcept
{
  return m_evaluator.evaluate(
    formula,
    raw,
    calculated,
    result);
}

}