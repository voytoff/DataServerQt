#pragma once

#include "datasourcefactory.h"
#include "runtimesystem.h"

namespace qds
{

class SystemBuilder
{
public:

  bool build(
    SystemConfiguration &configuration,
    const DataSourceFactory& dataSourceFactory,
    IArchiveWriter& archive,
    IFramePublisher& publisher,
    ISchedulerClock& clock,
    RuntimeSystem& runtime);
};

}