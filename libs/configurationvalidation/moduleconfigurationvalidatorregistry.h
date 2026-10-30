#pragma once

#include "moduleruntimeconfiguration.h"

#include <functional>
#include <string>
#include <unordered_map>

namespace qds
{

class ModuleConfigurationValidatorRegistry
{
public:

  using Validator = std::function<bool(
    const ModuleRuntimeConfiguration&,
    std::string&)>;

  [[nodiscard]]
  bool registerType(
    ModuleType type,
    Validator validator);

  [[nodiscard]]
  bool validate(
    const ModuleRuntimeConfiguration& configuration,
    std::string& error) const;

private:

  std::unordered_map<ModuleType, Validator> m_validators;
};

}