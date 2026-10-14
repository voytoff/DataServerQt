#pragma once

#include "calculationplan.h"
#include "db/calibrationrepository.h"
#include "formulaastrepository.h"
#include "formulacalculator.h"
#include "db/sensorrepository.h"
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
    const SensorRepository& sensors,
    const CalibrationRepository& calibrations);

  [[nodiscard]]
  bool process(
    const RawMemory& raw,
    CalculatedMemory& calculated) const noexcept;

private:

  const SignalMemoryLayout& m_layout;
  const FormulaAstRepository& m_formulas;
  const CalculationPlan& m_plan;

  const SensorRepository& m_sensors;
  const CalibrationRepository& m_calibrations;

  FormulaCalculator m_calculator;
};

}