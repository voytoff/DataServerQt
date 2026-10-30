#pragma once

#include "calibrationrepository.h"
#include "preparedconfiguration.h"
#include "sensorrepository.h"
#include "systemconfiguration.h"
#include "moduleconfigurationvalidatorregistry.h"

#include <string>

namespace qds
{

class ConfigurationValidator
{
public:

  [[nodiscard]]
  bool build(
    SystemConfiguration& configuration,
    const SensorRepository& sensors,
    const CalibrationRepository& calibrations,
    const ModuleConfigurationValidatorRegistry& moduleValidators,
    PreparedConfiguration& prepared);

  [[nodiscard]]
  const std::string& lastError() const noexcept;

private:

  bool validateCalibrations(
    const CalculationPlan& plan,
    const SensorRepository& sensors,
    const CalibrationRepository& calibrations);

private:
  bool validateModules(
    const SystemConfiguration& configuration,
    const ModuleConfigurationValidatorRegistry& validators);

  std::string m_lastError;
};

}