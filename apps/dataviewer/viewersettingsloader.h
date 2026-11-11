#pragma once

#include "datatypes.h"

#include <QString>

#include <cstdint>
#include <vector>

namespace qds
{

struct ViewerTableSettings
{
  uint32_t frequency = 100;
  std::vector<SignalId> channels;
};

class ViewerSettingsLoader
{
public:
  static std::vector<ViewerTableSettings> loadTables(
    const QString& fileName);

  static bool saveTables(
    const QString& fileName,
    const std::vector<ViewerTableSettings>& tables);
};

}