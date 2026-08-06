#include "calculationruntime.h"

namespace qds
{

CalculationRuntime::CalculationRuntime(
  const CalculationPlan &plan,
  const FormulaRepository &repository)
  : m_plan(plan)
  , m_repository(repository) { }

bool CalculationRuntime::execute(Frame &frame)
{
  for (const CalculationStep& step : m_plan.steps())
  {
    FormulaContext ctx;
    //ctx.inputs = step.inputs.data();
    //ctx.inputCount = step.inputs.size();
    //ctx.output = step.output;

    auto _ = step.formula->execute(ctx);
  }
  return false;

}

}