#pragma once

#include "ilogger.h"
#include "moduleruntimeconfiguration.h"
#include "ltr11configuration.h"

namespace qds
{

class Ltr11ConfigurationBuilder
{
public:
  bool build(
    const ModuleRuntimeConfiguration& moduleConfiguration,
    Ltr11Configuration& configuration,
    ILogger &logger);
};

}