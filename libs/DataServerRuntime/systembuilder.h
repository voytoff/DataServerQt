#pragma once

#include "db/calibrationrepository.h"
#include "datastreamsourcefactory.h"
#include "iarchivewriter.h"
#include "iclock.h"
#include "ilogger.h"
#include "runtimesystem.h"
#include "systemconfiguration.h"

#include <memory>

namespace qds
{

class SystemBuilder
{
public:

  [[nodiscard]]
  std::unique_ptr<RuntimeSystem> build(
    SystemConfiguration& configuration,
    const DataStreamSourceFactory& dataSourceFactory,
    const CalibrationRepository& calibrations,
    IClock& clock,
    IArchiveWriter& archive,
    ILogger& logger);
};

}