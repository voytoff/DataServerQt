#pragma once

#include "calculationplan.h"
#include "systemconfiguration.h"

namespace qds
{

class CalculationPlanBuilder
{
  struct Node
  {
    SignalId id;

    std::vector<SignalId> dependencies;

    std::vector<Node*> children;

    uint32_t indegree = 0;
  };

public:

  [[nodiscard]]
  bool build(
    const SystemConfiguration& cfg,
    CalculationPlan& plan);

};

}