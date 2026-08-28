#pragma once

#include "calculationplan.h"
#include "calibrationrepository.h"
#include "formulaastrepository.h"
#include "formulacalculator.h"
#include "signalmemory.h"
#include "signalmemorylayout.h"

namespace qds
{

class SignalProcessor
{
public:

  SignalProcessor(
    const SignalMemoryLayout& layout,
    const FormulaAstRepository& formulas,
    const CalculationPlan& plan,
    const CalibrationRepository& calibrations);

  [[nodiscard]]
  bool process(
    const RawMemory& raw,
    CalculatedMemory& calculated) const noexcept;

private:

  const SignalMemoryLayout& m_layout;
  const FormulaAstRepository& m_formulas;
  const CalculationPlan& m_plan;
  const CalibrationRepository& m_calibrations;

  FormulaCalculator m_calculator;
};

}