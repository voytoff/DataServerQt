#pragma once

#include "calculationplan.h"
#include "formularepository.h"

namespace qds
{

class CalculationRuntime
{
public:

  CalculationRuntime(
    const CalculationPlan& plan,
    const FormulaRepository& repository);

  bool execute(Frame& frame);

private:

  const CalculationPlan&      m_plan;
  const FormulaRepository&    m_repository;
};

}