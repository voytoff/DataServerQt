#include "calculationcompiler.h"
#include "calculationorder.h"

namespace qds
{

CalculationCompiler::CalculationCompiler(
  const SystemConfiguration& cfg,
  const SignalMemoryLayout &layout,
  FormulaAstRepository& formulas)
  : m_cfg(cfg)
  , m_layout(layout)
  , m_formulas(formulas)
{
}

bool CalculationCompiler::build(
  CalculationPlan& plan)
{
  plan.clear();

  for (const auto& definition :
       m_cfg.signalDefinitions())
  {
    if (definition.kind != SignalKind::Calculated)
      continue;

    if (m_formulas.find(
          definition.formulaId) == nullptr)
      return false;
  }

  CalculationOrder order;

  if (!order.build(m_cfg))
    return false;

  CalculationPlan tmp;

  for (SignalId id : order.order())
  {
    const auto* definition =
      m_cfg.findSignalDefinition(id);

    if (definition == nullptr)
      return false;

    tmp.add({
      .signal = id,
      .formula = definition->formulaId,
      .calibrationMode = definition->calibrationMode,
      .signalType = definition->signalType
    });
  }

  plan = std::move(tmp);

  return true;
}

}