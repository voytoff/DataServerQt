#pragma once

#include "formulaastrepository.h"
#include "calculationplan.h"
#include "formulacontextbuilder_NOTUSED.h"

namespace qds
{

class CalculationProcessor// final : public ICalculationProcessor
{
public:

  explicit CalculationProcessor(
    RawMemory& raw,
    CalculatedMemory& calculated,
    const CalculationPlan& plan,
    const FormulaAstRepository& formulas);


  [[nodiscard]]
  bool process(
    const RawMemory& raw,
    CalculatedMemory& calculated);


private:
  RawMemory& m_raw;
  CalculatedMemory m_calculated;
  const CalculationPlan& m_plan;
  const FormulaAstRepository& m_formulas;

  FormulaContextBuilder m_contextBuilder;
};

}