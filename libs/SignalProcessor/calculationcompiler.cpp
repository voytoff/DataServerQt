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
  m_lastError.clear();
  plan.clear();

  for (const auto& definition :
       m_cfg.signalDefinitions())
  {
    if (definition.kind != SignalKind::Calculated)
      continue;

    if (m_formulas.find(
          definition.formulaId) == nullptr)
    {
      m_lastError =
        "Не найдена формула вычисляемого сигнала \"" +
        definition.name + "\" (SignalId: " +
        std::to_string(definition.id.value) + ")";

      return false;
    }
  }

  CalculationOrder order;

  if (!order.build(m_cfg))
  {
    m_lastError = order.lastError();
    return false;
  }

  CalculationPlan tmp;

  for (SignalId id : order.order())
  {
    const auto* definition =
      m_cfg.findSignalDefinition(id);

    if (definition == nullptr)
    {
      m_lastError =
        "Не найден вычисляемый сигнал (SignalId: " +
        std::to_string(id.value) + ")";

      return false;
    }

    tmp.add({
      .signal = id,
      .formula = definition->formulaId,
      .sensorId = definition->sensorId
    });
  }

  plan = std::move(tmp);

  return true;
}

const std::string&
CalculationCompiler::lastError() const noexcept
{
  return m_lastError;
}

}