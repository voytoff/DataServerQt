#include "calculationplanbuilder.h"
#include "signalmemorylayout.h"

namespace qds
{

bool CalculationPlanBuilder::build(
  const SystemConfiguration& cfg,
  CalculationPlan& plan)
{
  std::vector<SignalId> calculatedSignals;

  for (const auto& definition : cfg.signalDefinitions())
  {
    if (definition.kind == SignalKind::Calculated)
      calculatedSignals.push_back(definition.id);
  }

  plan.setOrder(calculatedSignals);

  return true;
}

}
