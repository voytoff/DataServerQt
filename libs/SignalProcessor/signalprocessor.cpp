#include "signalprocessor.h"

namespace qds
{

SignalProcessor::SignalProcessor(
  const SignalMemoryLayout& layout,
  const FormulaAstRepository& formulas,
  const CalculationPlan& plan,
  const SensorRepository& sensors,
  const CalibrationRepository& calibrations)
  : m_layout(layout)
  , m_formulas(formulas)
  , m_plan(plan)
  , m_sensors(sensors)
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

    // 1. Вычисляем формулу.
    if (!m_calculator.calculate(
          *formula,
          raw,
          calculated,
          result))
      return false;

    // 2. Применяем тарировку, если назначен датчик.
    if (step.sensorId)
    {
      const Sensor* sensor =
        m_sensors.find(step.sensorId);

      if (sensor == nullptr)
        return false;

      double calibrated = 0.0;

      switch (sensor->calibrationMode)
      {
      case SensorCalibrationMode::Own:
        if (!m_calibrations.calibrateBySensor(
              sensor->id,
              sensor->calibrationKind,
              result,
              calibrated))
          return false;
        break;

      case SensorCalibrationMode::ByType:
      {
        const SensorType* type =
          m_sensors.find(sensor->sensorTypeId);

        if (type == nullptr)
          return false;

        if (!m_calibrations.calibrateBySensorType(
              type->id,
              type->calibrationKind,
              result,
              calibrated))
          return false;

        break;
      }

      default:
        return false;
      }

      result = calibrated;
    }

    // 3. Записываем результат.
    const SignalReference reference =
      m_layout.reference(step.signal);

    if (!reference.isValid() ||
        reference.area != SignalMemoryArea::Calculated ||
        reference.index >= calculated.size())
      return false;

    calculated.setValue(
      reference.index,
      result);
  }

  return true;
}

}