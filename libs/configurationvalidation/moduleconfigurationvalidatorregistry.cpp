#include "moduleconfigurationvalidatorregistry.h"

#include <utility>

namespace qds
{

bool ModuleConfigurationValidatorRegistry::registerType(
  ModuleType type,
  Validator validator)
{
  if (type == ModuleType::Unknown || !validator)
    return false;

  return m_validators.emplace(
                       type,
                       std::move(validator)).second;
}

bool ModuleConfigurationValidatorRegistry::validate(
  const ModuleRuntimeConfiguration& configuration,
  std::string& error) const
{
  error.clear();

  const auto it =
    m_validators.find(configuration.module.type);

  if (it == m_validators.end())
  {
    error =
      "Не зарегистрирована проверка конфигурации "
      "модуля (ModuleId: " +
      std::to_string(configuration.module.id.value) +
      ", тип: " +
      std::to_string(static_cast<int>(
        configuration.module.type)) + ")";

    return false;
  }

  if (!it->second(configuration, error))
  {
    if (error.empty())
      error = "Проверка конфигурации модуля завершилась ошибкой";

    return false;
  }

  return true;
}

}