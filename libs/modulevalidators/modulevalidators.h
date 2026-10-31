#pragma once

#include "moduleconfigurationvalidatorregistry.h"

namespace qds
{

[[nodiscard]]
bool registerModuleValidators(
  ModuleConfigurationValidatorRegistry& registry);

}