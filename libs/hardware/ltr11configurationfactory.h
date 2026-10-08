#pragma once

#include "ltr11configuration.h"
#include "moduleruntimeconfiguration.h"

#include <optional>

namespace qds
{

class Ltr11ConfigurationFactory
{
public:
  [[nodiscard]]
  static std::optional<Ltr11Configuration> create(
    const ModuleRuntimeConfiguration& configuration);
};

}