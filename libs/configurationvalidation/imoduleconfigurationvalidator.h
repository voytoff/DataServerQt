#pragma once

#include "moduleruntimeconfiguration.h"

#include <string>

namespace qds
{

class IModuleConfigurationValidator
{
public:
  virtual ~IModuleConfigurationValidator() = default;

  [[nodiscard]]
  virtual bool validate(
    const ModuleRuntimeConfiguration& configuration,
    std::string& error) const = 0;
};

}