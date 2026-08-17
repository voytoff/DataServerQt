#pragma once

#include <strongidhash.h>
#include "calculationplan.h"
#include "formulaastrepository.h"
#include "signalmemorylayout.h"
#include "systemconfiguration.h"

namespace qds
{

class CalculationCompiler
{
public:

  explicit CalculationCompiler(
    const SystemConfiguration& cfg,
    const SignalMemoryLayout& layout,
    FormulaAstRepository& formulas);

  bool build(
    CalculationPlan& plan);

private:

  const SystemConfiguration& m_cfg;
  const SignalMemoryLayout &m_layout;
  FormulaAstRepository& m_formulas;
};

}