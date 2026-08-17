#include "signalprocessor.h"

namespace qds
{

SignalProcessor::SignalProcessor(
  const SignalMemoryLayout& layout,
  const FormulaAstRepository& formulas,
  const CalculationPlan& plan)
  : m_layout(layout)
  , m_formulas(formulas)
  , m_plan(plan)
{
}

bool SignalProcessor::process(
  const RawMemory& raw,
  CalculatedMemory& calculated) const noexcept
{
  double result = 0.0;

  for (const CalculationStep& step : m_plan.steps())
  {
    const FormulaNode* formula =
      m_formulas.find(step.formula);

    if (formula == nullptr)
      return false;

    if (!m_calculator.calculate(
          *formula,
          raw,
          calculated,
          result))
      return false;

    const SignalReference reference =
      m_layout.reference(step.signal);

    if (!reference.isValid())
      return false;

    calculated.setValue(
      reference.index,
      result);
  }

  return true;
}

}