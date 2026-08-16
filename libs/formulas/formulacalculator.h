#pragma once

#include "calculationorder.h"
#include "formulaastrepository.h"
#include "formulafunctionrepository.h"
#include "parser/formulaevaluator.h"
#include "signalmemory.h"
#include "signalmemorylayout.h"
#include "systemconfiguration.h"

namespace qds
{

class FormulaCalculator
{
public:
  explicit FormulaCalculator();

  bool initialize(
    const SystemConfiguration& configuration,
    const SignalMemoryLayout& layout,
    FormulaAstRepository& formulas);

  bool calculate(
    const RawMemory& raw,
    CalculatedMemory& calculated) const noexcept;

private:

  const SystemConfiguration* m_configuration = nullptr;
  const SignalMemoryLayout* m_layout = nullptr;
  FormulaAstRepository* m_formulas = nullptr;

  CalculationOrder m_order;

  FormulaFunctionRepository m_functions;
  FormulaEvaluator m_evaluator;
};

}