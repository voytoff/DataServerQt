#pragma once

#include "datatypes.h"

#include <QString>
#include <vector>

namespace qds
{

class MonitorSettingsLoader
{
public:
  static std::vector<SignalId> loadSignals(
    const QString& fileName,
    ConfigurationId configurationId);

  static bool saveSignals(
    const QString& fileName,
    ConfigurationId configurationId,
    const std::vector<SignalId>& signalIds);
};

}