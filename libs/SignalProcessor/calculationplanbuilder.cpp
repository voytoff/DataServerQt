#include "calculationplanbuilder.h"
#include "signalmemorylayout.h"

namespace qds
{

bool CalculationPlanBuilder::build(const SystemConfiguration &cfg, CalculationPlan &plan)
{
  std::vector<SignalId> order;
  const auto& definitions = cfg.signalDefinitions();

  for (const auto& definition : definitions) {
    if (definition.kind == SignalKind::Calculated) {
      order.push_back(definition.id);
    }
  }

  plan.setOrder(order);

  return true;
}

}
