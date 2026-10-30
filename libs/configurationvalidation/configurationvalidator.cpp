#include "configurationvalidator.h"

#include "calculationcompiler.h"
#include "formulabuilder.h"

#include <utility>

namespace qds
{

bool ConfigurationValidator::build(
  SystemConfiguration& configuration,
  const SensorRepository& sensors,
  const CalibrationRepository& calibrations,
  const ModuleConfigurationValidatorRegistry& moduleValidators,
  PreparedConfiguration& prepared)
{
  m_lastError.clear();

  // Работаем с копией конфигурации.
  SystemConfiguration tmpConfiguration =
    configuration;

  PreparedConfiguration tmp;

  if (!tmp.layout.build(tmpConfiguration))
  {
    m_lastError =
      "Ошибка построения памяти сигналов: " +
      tmp.layout.lastError();

    return false;
  }

  if (!validateModules(
        tmpConfiguration,
        moduleValidators))
  {
    return false;
  }

  FormulaBuilder formulaBuilder;

  if (!formulaBuilder.build(
        tmpConfiguration,
        tmp.layout,
        tmp.formulas))
  {
    m_lastError =
      "Ошибка построения формул: " +
      formulaBuilder.lastError();

    return false;
  }

  CalculationCompiler compiler(
    tmpConfiguration,
    tmp.layout,
    tmp.formulas);

  if (!compiler.build(
        tmp.calculationPlan))
  {
    m_lastError =
      "Ошибка построения плана вычислений: " +
      compiler.lastError();

    return false;
  }

  if (!validateCalibrations(
        tmp.calculationPlan,
        sensors,
        calibrations))
  {
    m_lastError =
      "Ошибка проверки тарировок: " +
      m_lastError;

    return false;
  }

  // Все проверки прошли успешно.
  // Публикуем результаты.

  configuration =
    std::move(tmpConfiguration);

  prepared =
    std::move(tmp);

  return true;
}

const std::string&
ConfigurationValidator::lastError() const noexcept
{
  return m_lastError;
}

bool ConfigurationValidator::validateCalibrations(
  const CalculationPlan &plan,
  const SensorRepository &sensors,
  const CalibrationRepository &calibrations)
{
  for (const CalculationStep& step : plan.steps())
  {
    if (!step.sensorId)
      continue;

    const Sensor* sensor =
      sensors.find(step.sensorId);

    if (sensor == nullptr)
    {
      m_lastError =
        "Не найден датчик (SensorId: " +
        std::to_string(step.sensorId.value) +
        ") для сигнала (SignalId: " +
        std::to_string(step.signal.value) + ")";

      return false;
    }

    double result = 0.0;

    switch (sensor->calibrationMode)
    {
    case SensorCalibrationMode::Own:
    {
      if (!calibrations.calibrateBySensor(
            sensor->id,
            sensor->calibrationKind,
            0.0,
            result))
      {
        m_lastError =
          "Ошибка индивидуальной тарировки датчика "
          "(SensorId: " +
          std::to_string(sensor->id.value) +
          ", SignalId: " +
          std::to_string(step.signal.value) + ")";

        return false;
      }

      break;
    }

    case SensorCalibrationMode::ByType:
    {
      const SensorType* type =
        sensors.find(sensor->sensorTypeId);

      if (type == nullptr)
      {
        m_lastError =
          "Не найден тип датчика (SensorTypeId: " +
          std::to_string(sensor->sensorTypeId.value) +
          ") для датчика (SensorId: " +
          std::to_string(sensor->id.value) + ")";

        return false;
      }

      if (!calibrations.calibrateBySensorType(
            type->id,
            type->calibrationKind,
            0.0,
            result))
      {
        m_lastError =
          "Ошибка тарировки по типу датчика "
          "(SensorTypeId: " +
          std::to_string(type->id.value) +
          ", SensorId: " +
          std::to_string(sensor->id.value) +
          ", SignalId: " +
          std::to_string(step.signal.value) + ")";

        return false;
      }

      break;
    }

    default:
      m_lastError =
        "Неизвестный режим тарировки датчика "
        "(SensorId: " +
        std::to_string(sensor->id.value) + ")";

      return false;
    }
  }

  return true;
}

bool ConfigurationValidator::validateModules(
  const SystemConfiguration& configuration,
  const ModuleConfigurationValidatorRegistry& validators)
{
  for (const auto& module : configuration.modules())
  {
    if (module.type == ModuleType::Unknown)
    {
      m_lastError =
        "Неизвестный тип модуля (ModuleId: " +
        std::to_string(module.id.value) + ")";

      return false;
    }

    const auto cfg =
      configuration.moduleRuntimeConfiguration(
        module.id);

    if (!cfg)
    {
      m_lastError =
        "Не удалось сформировать конфигурацию модуля "
        "(ModuleId: " +
        std::to_string(module.id.value) + ")";

      return false;
    }

    std::string error;

    if (!validators.validate(*cfg, error))
    {
      m_lastError =
        "Ошибка конфигурации модуля "
        "(ModuleId: " +
        std::to_string(module.id.value) +
        "): " + error;

      return false;
    }
  }

  return true;
}

}