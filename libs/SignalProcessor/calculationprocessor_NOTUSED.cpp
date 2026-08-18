#include "calculationprocessor_NOTUSED.h"

namespace qds
{

CalculationProcessor::CalculationProcessor(
  RawMemory &raw,
  CalculatedMemory &calculated,
  const CalculationPlan& plan,
  const FormulaAstRepository &formulas)
  : m_raw(raw)
  , m_plan(plan)
  , m_calculated(calculated)
  , m_formulas(formulas) { }

/*
bool CalculationProcessor::process(
  Frame& frame)
{
  for (const auto& step : m_plan.steps())
  {
    FormulaContext ctx =
      m_contextBuilder.create(
        frame,
        step);

    if (!step.formula->execute(ctx))
      return false;
  }

  return true;
}
*/
}
