#include "calculationprocessor.h"

namespace qds
{

CalculationProcessor::CalculationProcessor(
  const CalculationPlan& plan)
  : m_plan(plan) { }

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

}

/*
 * Следующий логичный шаг после переименования — добавить первый интеграционный тест уже не для CalculationCompiler, а для связки:

Frame
 +
CalculationPlan
 +
CalculationProcessor
 +
FormulaCopy / FormulaAdd

То есть проверить полный цикл:

RAW memory
   |
   v
Formula execution
   |
   v
Calculated memory

Это будет первая проверка, что вся архитектура действительно замкнулась.
 */