#include "signalprocessor.h"

namespace qds
{

SignalProcessor::SignalProcessor(
  const SignalMemoryLayout& layout,
  const FormulaAstRepository& formulas,
  const CalculationPlan& plan,
  const CalibrationRepository &calibrations)
  : m_layout(layout)
  , m_formulas(formulas)
  , m_plan(plan)
  , m_calibrations(calibrations)
{
}

bool SignalProcessor::process(
  const RawMemory& raw,
  CalculatedMemory& calculated) const noexcept
{
  double result = 0.0;

  for (const CalculationStep& step : m_plan.steps())
  {
    const FormulaNode* formula =
      m_formulas.find(step.formula);

    if (formula == nullptr)
      return false;

    if (!m_calculator.calculate(
          *formula,
          raw,
          calculated,
          result))
      return false;

    // калибровка
    switch (step.calibrationMode)
    {
    case CalibrationMode::None:
      break;

    case CalibrationMode::BySignal:
      if (!m_calibrations.calibrateBySignal(
            step.signal,
            result,
            result))
        return false;
      break;

    case CalibrationMode::BySignalType:
      if (!m_calibrations.calibrateBySignalType(
            step.signalType,
            result,
            result))
        return false;
      break;

    default:
      return false;
    }

    const SignalReference reference =
      m_layout.reference(step.signal);

    if (!reference.isValid())
      return false;

    calculated.setValue(
      reference.index,
      result);
  }

  return true;
}

}