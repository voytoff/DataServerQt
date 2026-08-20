#pragma once

#include "datasourcefactory.h"
#include "runtimesystem.h"

namespace qds
{

class SystemBuilder
{
public:

  bool build(
    SystemConfiguration& configuration,
    const DataSourceFactory& dataSourceFactory,
    RuntimeSystem& runtime);
};

}