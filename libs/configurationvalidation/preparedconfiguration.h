#pragma once

#include "calculationplan.h"
#include "formulaastrepository.h"
#include "signalmemorylayout.h"

namespace qds
{

struct PreparedConfiguration
{
  SignalMemoryLayout layout;
  FormulaAstRepository formulas;
  CalculationPlan calculationPlan;
};

}