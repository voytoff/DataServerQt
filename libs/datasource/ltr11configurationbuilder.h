#pragma once

#include "moduleruntimeconfiguration.h"
#include "ltr11configuration.h"

#include <string>

namespace qds
{

class Ltr11ConfigurationBuilder
{
public:

  [[nodiscard]]
  bool build(
    const ModuleRuntimeConfiguration& moduleConfiguration,
    Ltr11Configuration& configuration);

  [[nodiscard]]
  const std::string& lastError() const noexcept;

private:

  std::string m_lastError;
};

}