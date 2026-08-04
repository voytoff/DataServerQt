#pragma once

#include "calculationplan.h"
#include "systemconfiguration.h"

namespace qds
{

class CalculationPlanBuilder
{
public:

  [[nodiscard]]
  bool build(
    const SystemConfiguration& cfg,
    CalculationPlan& plan);

};

}