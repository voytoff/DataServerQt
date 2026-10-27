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

  static int loadChartInterval(
    const QString& fileName,
    ConfigurationId configurationId);

  static bool saveChartInterval(
    const QString& fileName,
    ConfigurationId configurationId,
    int seconds);

  static std::vector<SignalId> loadHiddenChartSignals(
    const QString& fileName,
    ConfigurationId configurationId);

  static bool saveHiddenChartSignals(
    const QString& fileName,
    ConfigurationId configurationId,
    const std::vector<SignalId>& signalIds);

  static int loadChartYAxisMode(
    const QString& fileName,
    ConfigurationId id);

  static bool saveChartYAxisMode(
    const QString& fileName,
    ConfigurationId id,
    int mode);
};

}